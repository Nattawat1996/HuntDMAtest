#pragma once
#include <string>
#include <windows.h>

namespace Makcu
{
	// Connection state
	extern bool connected;
	extern std::string version;
	extern std::string portName;

	// Connection functions
	bool AutoConnectMakcu();
	bool Connect(const std::string& portName);
	void Disconnect();

	// Mouse movement functions
	void move(int x, int y);
	void move_smooth(int x, int y, int segments);
	
	// Button state tracking (1=Left, 2=Right, 3=Middle, 4=Mouse4, 5=Mouse5)
	bool button_pressed(int button);

	// Internal helpers
	namespace Internal
	{
		void StartButtonListener();
		void StopButtonListener();
		bool ValidateMakcuSignature();
	}
}
