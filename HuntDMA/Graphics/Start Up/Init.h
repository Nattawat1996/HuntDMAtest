#pragma once
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "windowscodecs.lib")

const bool enableAimBot = false;

extern std::shared_ptr<CheatFunction> UpdateCam;

void InitialiseClasses();
void InitializeESP();

extern int FrameRate();