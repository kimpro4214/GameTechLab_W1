#include "Platform/Win32Window.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam);

namespace
{
	constexpr wchar_t WindowClassName[] = L"JungleWindowClass";
	constexpr wchar_t WindowTitle[] = L"Game Tech Lab";
}

Win32Window::~Win32Window()
{
	if (Handle != nullptr && IsWindow(Handle))
	{
		DestroyWindow(Handle);
	}
	if (ApplicationInstance != nullptr)
	{
		UnregisterClassW(WindowClassName, ApplicationInstance);
	}
}

bool Win32Window::Create(HINSTANCE Instance, int ShowCommand)
{
	ApplicationInstance = Instance;

	WNDCLASSW WindowClass{};
	WindowClass.lpfnWndProc = WindowProcedure;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = WindowClassName;
	if (!RegisterClassW(&WindowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
	{
		return false;
	}

	Handle = CreateWindowExW(
		0,
		WindowClassName,
		WindowTitle,
		WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1024,
		1024,
		nullptr,
		nullptr,
		Instance,
		nullptr);
	if (Handle == nullptr)
	{
		return false;
	}

	ShowWindow(Handle, ShowCommand);
	return true;
}

LRESULT CALLBACK Win32Window::WindowProcedure(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam)
{
	if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
	{
		return true;
	}

	if (Message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(Window, Message, WParam, LParam);
}
