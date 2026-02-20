#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "kmbox.h"
#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace kmbox
{
	HANDLE serial_handle = NULL;
	bool connected = false;
	DeviceType detectedDevice = DeviceType::AutoDetect;
	std::string connectedPort = "";
	std::atomic<bool> monitor = false;
	bool key_state[256] = { false };
	std::thread rx_thread;
	std::thread tx_thread;
	std::queue<std::string> tx_queue;
	std::mutex tx_mutex;
	std::condition_variable tx_cv;

	// Kmbox Net Globals
	SOCKET client_socket = INVALID_SOCKET;
	sockaddr_in server_addr;
	bool net_connected = false;
	std::string stored_uuid = "";

	// Placeholder comment removed as per instruction.

	int clamp(int i)
	{
		if (i > 127)
			i = 127;
		if (i < -128)
			i = -128;

		return i;
	}

	bool IsDown(int key) {
		if (key < 0 || key >= 256) return false;
		return key_state[key];
	}

	bool IsValidByte(uint8_t data) {
		// List of valid Makcu/Mambo button state bytes
		static const uint8_t valid[] = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
			0x16, 0x17, 0x19, 0x1F
		};
		for (uint8_t v : valid) {
			if (data == v) return true;
		}
		return false;
	}

	void RxThread()
	{
		monitor = true;
		LOG_INFO("[Makcu] Starting RxThread...");
		
		while (monitor && connected && serial_handle != INVALID_HANDLE_VALUE)
		{
			DWORD bytesRead;
			uint8_t buffer[1];
			
			if (ReadFile(serial_handle, buffer, 1, &bytesRead, NULL) && bytesRead > 0)
			{
				uint8_t data = buffer[0];
				
				// Debug Log: Print every byte from device (Comment out to reduce spam after fix)
				// LOG_INFO("[Makcu] RX: %02X", data);

				if (!IsValidByte(data)) continue;

				// Map bits to buttons 
				// Bit 0: Left, Bit 1: Right, Bit 2: Middle, Bit 3: X1, Bit 4: X2
				int btnParams[] = { 1, 2, 4, 5, 6 }; // VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2
				for (int i = 0; i < 5; i++) {
					bool down = (data & (1 << i)) != 0;
					key_state[btnParams[i]] = down;
				}
				
				// Logic to dump buffer like EFT-DMA-Radar?
				// PurgeComm(serial_handle, PURGE_RXCLEAR);
			}
		}
		LOG_INFO("[Makcu] RxThread Stopped.");
	}

	void TxThread()
	{
		LOG_INFO("[KMBox] Starting TxThread...");
		while (monitor)
		{
			std::string cmd;
			{
				std::unique_lock<std::mutex> lock(tx_mutex);
				tx_cv.wait(lock, [] { return !tx_queue.empty() || !monitor; });
				if (!monitor && tx_queue.empty()) break;
				if (!tx_queue.empty()) {
					cmd = tx_queue.front();
					tx_queue.pop();
				} else {
					continue;
				}
			}

			if (detectedDevice == DeviceType::KmboxNet) {
				if (net_connected && client_socket != INVALID_SOCKET) {
					sendto(client_socket, cmd.c_str(), (int)cmd.length(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
				}
			} else if (serial_handle && serial_handle != INVALID_HANDLE_VALUE) {
				DWORD bytesWritten;
				WriteFile(serial_handle, cmd.c_str(), (DWORD)cmd.length(), &bytesWritten, NULL);
			}
		}
		LOG_INFO("[KMBox] TxThread Stopped.");
	}

	std::string find_port(const std::string& targetDescription)
	{
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return "";

		SP_DEVINFO_DATA deviceInfoData;
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		std::string foundCom = "";

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); ++i)
		{
			char buf[512];
			DWORD nSize = 0;

			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)buf, sizeof(buf), &nSize) && nSize > 0)
			{
				buf[nSize] = '\0';
				std::string deviceDescription = buf;
				LOG_INFO("[KMBox] Found Serial Device: %s", deviceDescription.c_str());

				size_t comPos = deviceDescription.find("COM");
				size_t endPos = deviceDescription.find(")", comPos);

				if (comPos != std::string::npos && endPos != std::string::npos)
				{
					if (deviceDescription.find(targetDescription) != std::string::npos)
					{
						foundCom = deviceDescription.substr(comPos, endPos - comPos);
						LOG_INFO("MATCHED Port: %s (%s)", foundCom.c_str(), deviceDescription.c_str());
						break; 
					}
				}
			}
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return foundCom;
	}

	// Find any available serial COM port
	std::string find_any_serial_port()
	{
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return "";

		SP_DEVINFO_DATA deviceInfoData;
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		std::string foundCom = "";

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); ++i)
		{
			char buf[512];
			DWORD nSize = 0;

			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)buf, sizeof(buf), &nSize) && nSize > 0)
			{
				buf[nSize] = '\0';
				std::string deviceDescription = buf;

				size_t comPos = deviceDescription.find("COM");
				size_t endPos = deviceDescription.find(")", comPos);

				if (comPos != std::string::npos && endPos != std::string::npos)
				{
					foundCom = deviceDescription.substr(comPos, endPos - comPos);
					LOG_INFO("[KMBox] Using first available port: %s (%s)", foundCom.c_str(), deviceDescription.c_str());
					break;
				}
			}
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return foundCom;
	}

	// Enumerate all available COM ports on the system
	std::vector<PortInfo> enumerate_ports()
	{
		std::vector<PortInfo> ports;
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return ports;

		SP_DEVINFO_DATA deviceInfoData;
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); ++i)
		{
			char buf[512];
			DWORD nSize = 0;

			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)buf, sizeof(buf), &nSize) && nSize > 0)
			{
				buf[nSize] = '\0';
				std::string deviceDescription = buf;

				size_t comPos = deviceDescription.find("COM");
				size_t endPos = deviceDescription.find(")", comPos);

				if (comPos != std::string::npos && endPos != std::string::npos)
				{
					PortInfo info;
					info.portName = deviceDescription.substr(comPos, endPos - comPos);
					info.description = deviceDescription;
					ports.push_back(info);
				}
			}
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return ports;
	}

	// Helper: Open a serial port with given baud rate
	static bool OpenSerialPort(const std::string& port, int baudRate)
	{
		serial_handle = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (serial_handle == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR("[KMBox] Failed to open serial port: %s", port.c_str());
			return false;
		}

		if (!SetupComm(serial_handle, 8192, 8192)) { CloseHandle(serial_handle); serial_handle = NULL; return false; }

		COMMTIMEOUTS timeouts = { 0 };
		if (GetCommTimeouts(serial_handle, &timeouts))
		{
			timeouts.ReadIntervalTimeout = 1;
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.ReadTotalTimeoutConstant = 1;
			timeouts.WriteTotalTimeoutMultiplier = 0;
			timeouts.WriteTotalTimeoutConstant = 50;  // 50ms max — prevents long block on slow device
			SetCommTimeouts(serial_handle, &timeouts);
		}

		PurgeComm(serial_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(serial_handle, &dcbSerialParams)) { CloseHandle(serial_handle); serial_handle = NULL; return false; }

		dcbSerialParams.BaudRate = baudRate;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		
		if (!SetCommState(serial_handle, &dcbSerialParams)) {
			LOG_ERROR("[KMBox] Failed to set baud rate %d!", baudRate);
			CloseHandle(serial_handle);
			serial_handle = NULL;
			return false;
		}

		return true;
	}

	// Helper: Send setup commands and start RX thread
	static void FinalizeConnection(const std::string& portName)
	{
		connected = true;
		DWORD bytesWritten;

		// Disable echo first, then enable buttons
		std::string disable_echo = "km.echo(0)\r\n";
		WriteFile(serial_handle, disable_echo.c_str(), disable_echo.length(), &bytesWritten, NULL);
		Sleep(10);
		
		std::string enable_buttons = "km.buttons(1)\r\n";
		WriteFile(serial_handle, enable_buttons.c_str(), enable_buttons.length(), &bytesWritten, NULL);
		
		FlushFileBuffers(serial_handle);

		// Start RX and TX listener threads
		rx_thread = std::thread(RxThread);
		tx_thread = std::thread(TxThread);

		connectedPort = portName;
		LOG_INFO("[KMBox] Successfully connected on %s", portName.c_str());
	}

	// Try connecting as Makcu device (handshake + 4MHz)
	static bool TryConnectMakcu(const std::string& port)
	{
		LOG_INFO("[KMBox] Trying Makcu connection on %s...", port.c_str());
		
		// Open at default baud first
		if (!OpenSerialPort(port, 115200))
			return false;

		// Send Magic Handshake to switch to 4MHz
		byte change_cmd[] = { 0xDE, 0xAD, 0x05, 0x00, 0xA5, 0x00, 0x09, 0x3D, 0x00 }; // 0x3D0900 = 4000000
		LOG_INFO("[KMBox] Sending Makcu Handshake...");
		DWORD bytesWritten;
		WriteFile(serial_handle, change_cmd, sizeof(change_cmd), &bytesWritten, NULL);
		FlushFileBuffers(serial_handle);
		Sleep(150);

		// Switch to High Speed (4MHz)
		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(serial_handle, &dcbSerialParams)) {
			CloseHandle(serial_handle);
			serial_handle = NULL;
			return false;
		}

		dcbSerialParams.BaudRate = 4000000;
		
		if (!SetCommState(serial_handle, &dcbSerialParams)) 
		{ 
			LOG_WARNING("[KMBox] Makcu 4MHz baud rate failed - device may not be a Makcu.");
			CloseHandle(serial_handle);
			serial_handle = NULL;
			return false;
		}
		
		Sleep(50);
		LOG_INFO("[KMBox] Makcu connected at 4MHz on %s", port.c_str());
		detectedDevice = DeviceType::Makcu;
		return true;
	}

	// Try connecting as standard KMBox (direct baud rate, no handshake)
	static bool TryConnectStandard(const std::string& port, int baudRate)
	{
		LOG_INFO("[KMBox] Trying Standard KMBox connection on %s at %d baud...", port.c_str(), baudRate);
		
		if (!OpenSerialPort(port, baudRate))
			return false;

		Sleep(50);
		LOG_INFO("[KMBox] Standard KMBox connected at %d baud on %s", baudRate, port.c_str());
		detectedDevice = DeviceType::StandardKmbox;
		return true;
	}

	void KmboxInitialize(std::string port, int baudRate, DeviceType type)
	{
		// Cleanup previous connection
		monitor = false;
		tx_cv.notify_all();
		if (rx_thread.joinable())
			rx_thread.join();
		if (tx_thread.joinable())
			tx_thread.join();

		// Clear the queue
		{
			std::lock_guard<std::mutex> lock(tx_mutex);
			std::queue<std::string> empty;
			std::swap(tx_queue, empty);
		}

		connected = false;
		net_connected = false;
		connectedPort = "";
		if (serial_handle)
		{
			CloseHandle(serial_handle);
			serial_handle = NULL;
		}

		if (client_socket != INVALID_SOCKET) {
			closesocket(client_socket);
			WSACleanup();
			client_socket = INVALID_SOCKET;
		}

		if (type == DeviceType::KmboxNet) {
			// Kmbox Net is handled by NetInitialize usually, but if called here with type KmboxNet, warn user
			LOG_ERROR("KmboxInitialize called with DeviceType::KmboxNet. Use NetInitialize instead.");
			return;
		}

		// Use user-specified port if provided, otherwise auto-detect
		std::string foundPort = "";
		
		if (!port.empty())
		{
			// User specified a port directly (e.g. "COM3")
			foundPort = port;
			LOG_INFO("[KMBox] Using user-specified port: %s", foundPort.c_str());
		}
		else
		{
			// Auto-detect: try common KMBox/Makcu USB-serial chip descriptors
			const char* searchTerms[] = { "CH34", "CH9102", "CP210", "FTDI", "USB-SERIAL", "USB Serial" };
			for (const char* term : searchTerms)
			{
				foundPort = find_port(term);
				if (!foundPort.empty())
					break;
			}
			
			// If not found with known descriptors, try any available serial port
			if (foundPort.empty())
			{
				LOG_WARNING("[KMBox] No known USB-serial device found, trying first available COM port...");
				foundPort = find_any_serial_port();
			}
		}

		if (foundPort.empty())
		{
			LOG_ERROR("[KMBox] No serial port found! Make sure your KMBox/Makcu is plugged in.");
			return;
		}

		std::string fullPort = "\\\\.\\" + foundPort;
		LOG_INFO("[KMBox] Using port: %s (DeviceType: %d)", fullPort.c_str(), (int)type);

		bool success = false;

		switch (type)
		{
		case DeviceType::Makcu:
			// Force Makcu mode
			success = TryConnectMakcu(fullPort);
			if (!success)
				LOG_ERROR("[KMBox] Makcu connection failed!");
			break;

		case DeviceType::StandardKmbox:
			// Force standard mode
			success = TryConnectStandard(fullPort, baudRate);
			if (!success)
				LOG_ERROR("[KMBox] Standard KMBox connection failed!");
			break;

		case DeviceType::AutoDetect:
		default:
			// Try Makcu first, then fall back to standard
			LOG_INFO("[KMBox] Auto-detecting device type...");
			success = TryConnectMakcu(fullPort);
			if (!success)
			{
				LOG_INFO("[KMBox] Makcu handshake failed, trying standard KMBox...");
				success = TryConnectStandard(fullPort, baudRate);
			}
			if (!success)
				LOG_ERROR("[KMBox] Auto-detect failed! Could not connect as Makcu or Standard KMBox.");
			break;
		}

		if (success)
		{
			FinalizeConnection(fullPort);
		}
	}

	void NetInitialize(std::string ip, std::string port, std::string uuid) {
		// Cleanup previous
		monitor = false;
		tx_cv.notify_all();
		if (rx_thread.joinable()) rx_thread.join();
		if (tx_thread.joinable()) tx_thread.join();

		// Clear the queue
		{
			std::lock_guard<std::mutex> lock(tx_mutex);
			std::queue<std::string> empty;
			std::swap(tx_queue, empty);
		}

		connected = false;
		if (serial_handle) { CloseHandle(serial_handle); serial_handle = NULL; }
		if (client_socket != INVALID_SOCKET) { closesocket(client_socket); WSACleanup(); client_socket = INVALID_SOCKET; }

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			LOG_ERROR("[KmboxNet] WSAStartup failed");
			return;
		}

		client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (client_socket == INVALID_SOCKET) {
			LOG_ERROR("[KmboxNet] Socket creation failed");
			WSACleanup();
			return;
		}

		// Set timeout
		DWORD timeout = 100;
		setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(std::stoi(port));
		inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

		stored_uuid = uuid;
		net_connected = true;
		connected = true; // Global connected flag
		detectedDevice = DeviceType::KmboxNet;
		connectedPort = ip + ":" + port;

		monitor = true;
		tx_thread = std::thread(TxThread);

		LOG_INFO("[KmboxNet] Initialized UDP socket to %s:%s", ip.c_str(), port.c_str());
	}

	void SendCommand(const char* command, size_t length)
	{
		std::string cmd(command, length);
		{
			std::lock_guard<std::mutex> lock(tx_mutex);
			if (tx_queue.size() > 50) return; // Drop if overloaded
			tx_queue.push(cmd);
		}
		tx_cv.notify_one();
	}

	void SendCommand(const std::string& command)
	{
		SendCommand(command.c_str(), command.length());
	}


	void move(int x, int y)
	{
		if (!connected) return;
		x = clamp(x);
		y = clamp(y);
		char buffer[64];
		int len = sprintf_s(buffer, "km.move(%d,%d)\r\n", x, y);
		SendCommand(buffer, len);
	}

	void test_move()
	{
		if (!connected)
			return;
			
		// Move in a small square to demonstrate control
		move(50, 0);   // Right
		Sleep(100);
		move(0, 50);   // Down
		Sleep(100);
		move(-50, 0);  // Left
		Sleep(100);
		move(0, -50);  // Up
	}

	void left_click()
	{
		if (!connected)
		{
			//LOG_ERROR("not connected?");
			return;
		}
		std::string command = "km.left(1)\r\n";
		SendCommand(command.c_str());
	}

	void left_click_release()
	{
		if (!connected)
		{
			//LOG_ERROR("not connected?");
			return;
		}
		std::string command = "km.left(0)\r\n";
		SendCommand(command.c_str());
	}
}
