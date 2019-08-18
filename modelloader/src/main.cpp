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

// The Windows API is used by including the Windows.h header.
#include <Windows.h>
#include <iostream>
#include <string>

// Function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void WriteDebug(std::wstring message);

// The wWinMain entry point is used with the WINDOWS subsystem.
// https://docs.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
// This is the entry point we have to use when we want to create windowed applications
// hInstance = Handle to an instance. This is used by the OS to identify the executable when loaded into memory.
// hPrevInstance = Not used any longer. Has no meaning.
// pCmdLine = Contains command-line arguments as a Unicode string
// nCmdShow = A flag telling whether the main application window will be minimized, maximized or shown normally.
// Return value: Can be used to convey status code to another program. Not used by the operating system, so essentially you can do what you want with it.
// WINAPI = The calling convention. It defines how the function receives parameters from the caller. The order in which they should appear on the stack.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Register window class
	// A Window Class describes a template for instances of windows.
	// They define a set of behaviour several windows might have in common.
	// Every window must be associated with a Window Class, even if you only ever create a single
	// window in your application.
	const wchar_t className[] = L"3D Model Loader";

	WNDCLASSEX windowClass = {};

	// Should always be set to sizeof(WNDCLASSEX)
	// Setting this member is required.
	windowClass.cbSize = sizeof(WNDCLASSEX);
	
	// The pointer to the application defined "Window Procedure".
	windowClass.lpfnWndProc = WndProc;

	// The handle to the application instance.
	windowClass.hInstance = hInstance;

	// A string that uniquely identifies the Window Class.
	windowClass.lpszClassName = className;

	// Now we register the Window Class with the operating system
	// This has to be done before we can create a window using it
	if (!RegisterClassEx(&windowClass))
	{
		std::wstring errorMessage = L"Failed to register class with error: ";
		errorMessage += std::to_wstring(GetLastError());
		WriteDebug(errorMessage);
		
		return -1;
	}

	// Create Window
	// If window creation fails, a NULL value is returned.
	const auto mainWindowWidth = 800;
	const auto mainWindowHeight = 600;
	
	const auto mainWindowHandle = CreateWindowEx(
		0, // Optional window styles
		className, // Name of the Window Class to use
		L"3D Model Loader", // The text at the title bar of the window
		WS_OVERLAPPEDWINDOW, // The Window Style
		// Size and Position
		CW_USEDEFAULT, CW_USEDEFAULT, mainWindowWidth, mainWindowHeight,
		nullptr, // Parent Window
		nullptr, // Menu
		hInstance, // Instance Handle
		nullptr // Additional application data
	);

	if (mainWindowHandle == nullptr)
	{
		WriteDebug(L"Failed to create main application window!");
		return -1;
	}

	ShowWindow(mainWindowHandle, true);

	// Game Loop
	MSG msg = {};
	while (true)
	{
		// When doing realtime applications, it's important to use PeekMessage to look for
		// and remove potential messages, instead of GetMessage, as GetMessage is blocking.
		// If the second parameter (hwnd) is NULL, PeekMessage will retrieve messages
		// for ANY window that belongs to the current thread, and any messages on the current
		// thread's message queue whose hwnd value is NULL.
		// Generating a WM_QUIT message through PostQuitMessage will create a message NOT
		// associated with any window, and since we know that we will never need more than a single window
		// We will set hwnd to NULL so that we catch both messages meant for the window, as well as
		// WM_QUIT when the application is asked to terminate.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// If the message is WM_QUIT, exit the while loop
			// WM_QUIT indicates a request to terminate an application.
			// 
			if (msg.message == WM_QUIT)
				break;
			
			// TranslateMessage function is related to keyboard input.
			// It translates keystrokes into characters.
			// You have to call this one before calling DispatchMessage.
			TranslateMessage(&msg);

			// DispatchMessage function tells Windows to call the Window Procedure of that window
			// That is the target of the message.
			DispatchMessage(&msg);
		} else
		{
			// Game logic here...
		}
	}

	DestroyWindow(mainWindowHandle);
	
	return 0;
}

// Check to see if any messages are waiting in the queue
// https://docs.microsoft.com/en-us/windows/win32/learnwin32/window-messages
// Windows uses a Message-Passing model. The operating system communicates with your application
// by passing messages to it.
// Message = A numeric code that implies a specific event.
// The operating system passes a message to the application through its registered
// Window Procedure function (The WND PROC function)
// For each thread that creates a window, Windows creates a queue for messages.
// You can use pre-defined functions to interact with this queue.
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// VM_DESTROY is sent to a window when it is being destroyed.
	case WM_DESTROY:
		// PostQuitMessage is used when one wants to indicate a request to terminate
		// Usually this function call is a response to WM_DESTROY.
		// The PostQuitMessage will post a WM_QUIT message to the thread's message queue.
		PostQuitMessage(0);
		break;
	default:;
	}
	
	// If you don't handle a particular message in your window procedure, pass the
	// Message parameters directly to DefWindowProc. This function performs the default
	// action for the message.
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/writing-the-window-procedure
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WriteDebug(std::wstring message)
{
	message += L"\n";
	OutputDebugString(message.c_str());
}