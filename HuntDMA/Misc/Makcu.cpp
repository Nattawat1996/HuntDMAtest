#include "Pch.h"
#include "Makcu.h"
#include <thread>
#include <atomic>
#include <mutex>

namespace Makcu
{
	// Constants for Makcu device identity
	const char* MAKCU_VID = "1A86";
	const char* MAKCU_PID = "55D3";
	const char* MAKCU_EXPECT_SIGNATURE = "km.MAKCU";
	const BYTE CHANGE_CMD[] = { 0xDE, 0xAD, 0x05, 0x00, 0xA5, 0x00, 0x09, 0x3D, 0x00 };

	// State variables
	bool connected = false;
	std::string version = "";
	std::string portName = "";  // Track connected port name

	// Serial port handle
	static HANDLE hSerial = INVALID_HANDLE_VALUE;
	
	// Button state (1-5: Left, Right, Middle, Mouse4, Mouse5)
	static bool buttonState[6] = { false };
	static std::mutex buttonMutex;
	
	// Button listener thread
	static std::atomic<bool> runListener{ false };
	static std::thread listenerThread;

	// Helper to find COM port by VID/PID
	std::string FindComPortByVidPid(const std::string& vid, const std::string& pid)
	{
		HKEY hKey;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
			return "";

		char valueName[256];
		BYTE data[256];
		DWORD valueNameSize, dataSize, type;
		
		for (DWORD i = 0; ; i++)
		{
			valueNameSize = sizeof(valueName);
			dataSize = sizeof(data);
			
			LONG result = RegEnumValueA(hKey, i, valueName, &valueNameSize, nullptr, &type, data, &dataSize);
			if (result == ERROR_NO_MORE_ITEMS)
				break;
			if (result != ERROR_SUCCESS)
				continue;

			// Check if the device path contains our VID/PID
			std::string devicePath = valueName;
			if (devicePath.find(vid) != std::string::npos && devicePath.find(pid) != std::string::npos)
			{
				RegCloseKey(hKey);
				return std::string((char*)data);
			}
		}

		RegCloseKey(hKey);
		return "";
	}

	// Button listener thread function
	void ButtonListenerThread()
	{
		while (runListener)
		{
			if (!connected || hSerial == INVALID_HANDLE_VALUE)
			{
				Sleep(100);
				continue;
			}

			BYTE buffer;
			DWORD bytesRead;
			
			if (ReadFile(hSerial, &buffer, 1, &bytesRead, nullptr) && bytesRead > 0)
			{
				// Update button states based on bitmask
				std::lock_guard<std::mutex> lock(buttonMutex);
				for (int i = 1; i <= 5; i++)
				{
					buttonState[i] = (buffer & (1 << (i - 1))) != 0;
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}

	namespace Internal
	{
		bool ValidateMakcuSignature()
		{
			if (hSerial == INVALID_HANDLE_VALUE)
				return false;

			// Clear buffer and request version
			PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
			
			const char* cmd = "km.version()\r";
			DWORD written;
			WriteFile(hSerial, cmd, strlen(cmd), &written, nullptr);
			
			Sleep(200);

			char response[256] = { 0 };
			DWORD bytesRead;
			ReadFile(hSerial, response, sizeof(response) - 1, &bytesRead, nullptr);

			if (bytesRead > 0)
			{
				version = std::string(response, bytesRead);
				// Remove any trailing newlines/carriage returns
				while (!version.empty() && (version.back() == '\n' || version.back() == '\r'))
					version.pop_back();
				
				return version.find(MAKCU_EXPECT_SIGNATURE) != std::string::npos;
			}

			return false;
		}

		void StartButtonListener()
		{
			if (runListener || !connected)
				return;

			// Enable button stream
			const char* cmd1 = "km.buttons(1)\r\n";
			const char* cmd2 = "km.echo(0)\r\n";
			DWORD written;
			WriteFile(hSerial, cmd1, strlen(cmd1), &written, nullptr);
			WriteFile(hSerial, cmd2, strlen(cmd2), &written, nullptr);
			
			PurgeComm(hSerial, PURGE_RXCLEAR);

			runListener = true;
			listenerThread = std::thread(ButtonListenerThread);
		}

		void StopButtonListener()
		{
			if (!runListener)
				return;

			runListener = false;
			if (listenerThread.joinable())
				listenerThread.join();

			if (connected && hSerial != INVALID_HANDLE_VALUE)
			{
				const char* cmd = "km.buttons(0)\r\n";
				DWORD written;
				WriteFile(hSerial, cmd, strlen(cmd), &written, nullptr);
			}
		}
	}

	bool Connect(const std::string& port)
	{
		try
		{
			// Construct device path
			std::string devicePath = "\\\\.\\" + port;

			// Open serial port
			hSerial = CreateFileA(
				devicePath.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,
				nullptr,
				OPEN_EXISTING,
				0,
				nullptr
			);

			if (hSerial == INVALID_HANDLE_VALUE)
				return false;

			// Configure serial port (115200 baud initially)
			DCB dcbSerialParams = { 0 };
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
			
			if (!GetCommState(hSerial, &dcbSerialParams))
			{
				CloseHandle(hSerial);
				hSerial = INVALID_HANDLE_VALUE;
				return false;
			}

			dcbSerialParams.BaudRate = 115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			if (!SetCommState(hSerial, &dcbSerialParams))
			{
				CloseHandle(hSerial);
				hSerial = INVALID_HANDLE_VALUE;
				return false;
			}

			// Set timeouts
			COMMTIMEOUTS timeouts = { 0 };
			timeouts.ReadIntervalTimeout = 50;
			timeouts.ReadTotalTimeoutConstant = 100;
			timeouts.ReadTotalTimeoutMultiplier = 10;
			timeouts.WriteTotalTimeoutConstant = 100;
			timeouts.WriteTotalTimeoutMultiplier = 10;
			SetCommTimeouts(hSerial, &timeouts);

			Sleep(150);

			// Send mode change command
			DWORD written;
			WriteFile(hSerial, CHANGE_CMD, sizeof(CHANGE_CMD), &written, nullptr);
			FlushFileBuffers(hSerial);

			// Switch to high speed (4000000 baud)
			dcbSerialParams.BaudRate = 4000000;
			SetCommState(hSerial, &dcbSerialParams);

			Sleep(150);

			// Validate device signature
			if (!Internal::ValidateMakcuSignature())
			{
				CloseHandle(hSerial);
				hSerial = INVALID_HANDLE_VALUE;
				return false;
			}

			connected = true;
			portName = port;  // Store the port name
			LOG_INFO("Makcu connected on port: %s", port.c_str());
			Internal::StartButtonListener();
			
			return true;
		}
		catch (...)
		{
			if (hSerial != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hSerial);
				hSerial = INVALID_HANDLE_VALUE;
			}
			connected = false;
			return false;
		}
	}

	bool AutoConnectMakcu()
	{
		// Try to find Makcu device by VID/PID
		std::string comPort = FindComPortByVidPid(MAKCU_VID, MAKCU_PID);
		
		if (comPort.empty())
		{
			// Try common COM ports as fallback
			for (int i = 1; i <= 20; i++)
			{
				std::string port = "COM" + std::to_string(i);
				if (Connect(port))
				{
					LOG_INFO("Makcu auto-connected on port: %s", port.c_str());
					return true;
				}
			}
			return false;
		}

		if (Connect(comPort))
		{
			LOG_INFO("Makcu connected via VID/PID on port: %s", comPort.c_str());
			return true;
		}
		return false;
	}

	void Disconnect()
	{
		if (!connected)
			return;

		Internal::StopButtonListener();

		if (hSerial != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hSerial);
			hSerial = INVALID_HANDLE_VALUE;
		}

		connected = false;
		version = "";
		portName = "";
	}

	void move(int x, int y)
	{
		if (!connected || hSerial == INVALID_HANDLE_VALUE)
			return;

		char cmd[64];
		snprintf(cmd, sizeof(cmd), "km.move(%d, %d)\r", x, y);
		
		DWORD written;
		WriteFile(hSerial, cmd, strlen(cmd), &written, nullptr);
	}

	void move_smooth(int x, int y, int segments)
	{
		if (!connected || hSerial == INVALID_HANDLE_VALUE)
			return;

		char cmd[64];
		snprintf(cmd, sizeof(cmd), "km.move(%d, %d, %d)\r", x, y, segments);
		
		DWORD written;
		WriteFile(hSerial, cmd, strlen(cmd), &written, nullptr);
	}

	bool button_pressed(int button)
	{
		if (!connected || button < 1 || button > 5)
			return false;

		std::lock_guard<std::mutex> lock(buttonMutex);
		return buttonState[button];
	}
}
