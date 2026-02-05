#pragma once
#include "pch.h"
#include "Registry.h"

namespace Keyboard
{
	bool InitKeyboard();

	struct KeyStateInfo
	{
		bool dmaAvailable = false;
		bool dmaDown = false;
		bool makcuDown = false;
		bool combinedDown = false;
	};

	void UpdateKeys();
	bool IsKeyDown(uint32_t virtual_key_code);
	KeyStateInfo GetKeyStateInfo(uint32_t virtual_key_code);
};
