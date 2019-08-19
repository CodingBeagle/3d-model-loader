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
#include <string>

#include <glad/glad.h>
#include <KHR/khrplatform.h>
#include <glad/glad_wgl.h>

// Function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void WriteDebug(std::string message);

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

	// CS_OWNDC = Allocates a unique device context for each window in the class
	// https://docs.microsoft.com/en-us/windows/win32/gdi/device-contexts
	// A device context is a special set of values that applications use for drawing in the client area of
	// their windows.
	// THIS IS NEEDED FOR OpenGL
	windowClass.style = CS_OWNDC;

	// Now we register the Window Class with the operating system
	// This has to be done before we can create a window using it
	if (!RegisterClassEx(&windowClass))
	{
		std::string errorMessage = "Failed to register class with error: ";
		errorMessage += std::to_string(GetLastError());
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
		WriteDebug("Failed to create main application window!");
		return -1;
	}

	ShowWindow(mainWindowHandle, true);

	// Set up OpenGL context
	auto deviceContext = GetDC(mainWindowHandle);

	// The struct PIXELFORMATDESCRIPTOR is used to describe the pixel format of a drawing surface of a device context
	// A Pixel Format is a generic structure that describes the properties of the default framebuffer that the OpenGL context
	// will use.
	// Notice that the pixel format should NOT be set more than once in our case, as the device context references our main window.
	// When we set the pixel format later, that function will also set the pixel format of our window, and setting the pixel format of a window
	// should NOT be done more than once.
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {};

	// Should always be sizeof(PIXELFORMATDESCRIPTOR)
	pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	// The version of the data structure. Should be set to 1.
	pixelFormatDescriptor.nVersion = 1;

	// A set of flags specifying properties of the pixel buffer.
	// PFD_DRAW_TO_WINDOW = The buffer can draw to a window / device surface
	// PFD_SUPPORT_OPENGL = The buffer supports OpenGL
	// PFD_DOUBLEBUFFER = The buffer is double-buffered
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	// Specifying the type of pixel data.
	// PFD_TYPE_RGBA = RGBA pixels. Each pixel has four components: red, green, blue, and alpha.
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;

	// For RGBA pixel types, this describes the size of the color buffer.
	pixelFormatDescriptor.cColorBits = 32;

	// Specifies the number of bits for the depth buffer.
	pixelFormatDescriptor.cDepthBits = 24;

	// Specifies the number of bits for the stencil buffer.
	pixelFormatDescriptor.cStencilBits = 8;

	// Now that we have a Pixel Format Descriptor, it has to be converted to a pixel format number.
	// ChoosePixelFormat takes a device context, and a pixel format descriptor, and attempts to match
	// a pixel format of the description by one supported by the device context.
	// If this function returns 0, the call failed.
	const auto pixelFormatNumber = ChoosePixelFormat(deviceContext, &pixelFormatDescriptor);
	if (pixelFormatNumber == 0)
	{
		WriteDebug("Failed to match a pixel format.");
		return -1;
	}

	// Now that we have a pixel format number, we can use it to set the pixel format of the
	// window's device context.
	const auto didSetPixelFormat = SetPixelFormat(deviceContext, pixelFormatNumber, &pixelFormatDescriptor);
	if (!didSetPixelFormat)
	{
		WriteDebug("Failed to set pixel format of windows device context.");
		return -1;
	}

	// Now that we've set the pixel format and thus described our framebuffer, we can create the OpenGL context.
	// wglCreateContext creates a new OpenGL context and returns a handle to it
	const auto openGlContextHandle = wglCreateContext(deviceContext);
	if (openGlContextHandle == nullptr)
	{
		WriteDebug("Failed to create OpenGl dummy context!");
		return -1;
	}

	// Before an OpenGL context can be used, it must be set to current.
	const auto didMakeOpenGlContextCurrent = wglMakeCurrent(deviceContext, openGlContextHandle);
	if (!didMakeOpenGlContextCurrent)
	{
		WriteDebug("Failed to make OpenGL dummy context current!");
		return -1;
	}

	// Load OpenGL functions using GLAD
	if (!gladLoadGL())
	{
		WriteDebug("Failed to initialize OpenGL GLAD!");
		return -1;
	}

	// Load WGL extension functions using GLAD
	// We need these for the extended WGL functions with more functionality for creating
	// OpenGL contexts.
	if (!gladLoadWGL(deviceContext))
	{
		WriteDebug("Failed to initialize WGL GLAD!");
		return -1;
	}

	const int attribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		0 // End
	};

	int pixelFormat;
	UINT numFormats;

	const auto didChoosePixelFormatArb = wglChoosePixelFormatARB(deviceContext, attribList, nullptr, 1, &pixelFormat, &numFormats);
	if (!didChoosePixelFormatArb)
	{
		WriteDebug("Failed to choose a pixel format ARB!");
		return -1;
	}
	
	std::string openGlVersion = "OpenGL version used: ";
	openGlVersion.append((char*)glGetString(GL_VERSION));
	
	WriteDebug(openGlVersion);
	
	// Game Loop
	MSG msg = {};
	while (true)
	{
		glClearColor(1.0f, 0.f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
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

		// TODO: Maybe try and research this SwapBuffers vs wglSwapLayerBuffers dilemma a bit more?
		// When swapping buffers, it's recommended to use the SwapBuffers function instead of wglSwapLayerBuffers: https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows
		// SwapBuffers exchanges the front and back buffer if the current pixel format for the window
		// referenced by the device context has a back buffer.
		SwapBuffers(deviceContext);
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

void WriteDebug(std::string message)
{
	message += "\n";
	OutputDebugStringA(message.c_str());
}