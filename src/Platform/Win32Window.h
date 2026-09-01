#pragma once

#include <Windows.h>

class Win32Window
{
public:
	Win32Window() = default;
	~Win32Window();

	Win32Window(const Win32Window&) = delete;
	Win32Window& operator=(const Win32Window&) = delete;

	bool Create(HINSTANCE Instance, int ShowCommand);
	HWND GetHandle() const { return Handle; }

private:
	static LRESULT CALLBACK WindowProcedure(
		HWND Window,
		UINT Message,
		WPARAM WParam,
		LPARAM LParam);

	HINSTANCE ApplicationInstance = nullptr;
	HWND Handle = nullptr;
};
