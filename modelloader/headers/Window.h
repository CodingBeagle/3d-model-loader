#pragma once

// The Windows API runs two parallel APIs. One which uses ANSI strings, and one which
// uses UNICODE strings. This means that a function in the Windows API actually has two versions.
// One which ends in *A and one which ends in W. A = ANSI and W = Wide (Unicode).
// Example: SetWindowTextA and SetWindowTextW.
// By defining the UNICODE macro, we can use the "pure" function name, i.e: SetWindowText, and our calls
// Will be redefined to the W version internally.
// MSDN recommends always using the Unicode version: https://docs.microsoft.com/en-us/windows/win32/learnwin32/working-with-strings
#ifndef UNICODE
#define UNICODE
#endif

#include <glad/glad.h>
#include <glad/glad_wgl.h>

#include <Windows.h>
#include <assert.h>
#include <string>

class Window
{
public:
	Window(std::wstring windowTitle, int windowWidth, int windowHeight);
	void Show() const;
	HDC GetDeviceContext() const;
	void SwapFrontAndBackBuffers() const;
private:
	HWND CreateBaseWindow(std::wstring windowTitle, int windowWidth, int windowHeight, WNDPROC windowProcedure, const std::wstring windowClassName);
	static HGLRC CreateBasicOpenGlContext(HDC deviceContext);
	static HGLRC CreateAdvanceOpenGlContext(HDC deviceContext);
	const std::wstring CLASS_NAME{ L"game-window" };
	HWND windowHandle = nullptr;
	HMODULE applicationHandle = nullptr;
	HGLRC contextHandle = nullptr;
};