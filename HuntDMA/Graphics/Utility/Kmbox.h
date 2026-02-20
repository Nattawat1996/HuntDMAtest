#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <string>
#include <vector>
#include <thread>
#include <setupapi.h>
#include <devguid.h>
#include <atomic>
#pragma comment(lib, "setupapi.lib")
namespace kmbox
{
	// Device type enum for different KMBox/Makcu hardware
	enum class DeviceType : int {
		AutoDetect = 0,    // Try Makcu first, fall back to standard
		Makcu = 1,         // Makcu-specific handshake + 4MHz
		StandardKmbox = 2, // Standard KMBox B/B+ at configured baud rate
		KmboxNet = 3       // Kmbox Net (UDP)
	};

	// Info about an available COM port
	struct PortInfo {
		std::string portName;    // e.g. "COM3"
		std::string description; // e.g. "USB-SERIAL CH340 (COM3)"
	};

	extern bool connected;
	extern DeviceType detectedDevice;
	extern std::string connectedPort; // Which port we are connected to
	extern std::string find_port(const std::string& targetDescription);
	extern std::vector<PortInfo> enumerate_ports(); // List all available COM ports
	extern void KmboxInitialize(std::string port, int baudRate = 115200, DeviceType type = DeviceType::AutoDetect);
	extern void NetInitialize(std::string ip, std::string port, std::string uuid);
	extern bool IsDown(int key);

	extern void move(int x, int y);
	extern void test_move();
	extern void left_click();
	extern void lock_all();

	// Makcu Specific
	extern void RxThread();
	extern std::atomic<bool> monitor;
	extern void left_click_release();
}