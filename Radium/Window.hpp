#pragma once
#include <string>
#include <wtypes.h>
class Window
{
private:
	HWND NewWindow();
public:
	HWND Win{};
	void InitRendering();
	bool Running = true;
	Window();
};

