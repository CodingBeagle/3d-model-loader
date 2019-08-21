#include "Window.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void APIENTRY OpenGlMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

Window::Window(std::wstring windowTitle, int windowWidth, int windowHeight)
{
	const auto dummyWindow = CreateBaseWindow(L"", 0, 0, DefWindowProc, L"dummywindowclass");
	const auto dummyOpenGlContext = CreateBasicOpenGlContext(GetDC(dummyWindow));

	windowHandle = CreateBaseWindow(windowTitle, windowWidth, windowHeight, WndProc, CLASS_NAME);
	contextHandle = CreateAdvanceOpenGlContext(GetDC(windowHandle));

	ShowWindow(windowHandle, true);

	const auto didDeleteDummyContext = wglDeleteContext(dummyOpenGlContext);
	if (!didDeleteDummyContext)
	{
		OutputDebugStringA("Failed to delete dummy OpenGL context! \n");
		assert(false);
	}
	
	const auto didDestroyDummyWindow = DestroyWindow(dummyWindow);
	if (!didDestroyDummyWindow)
	{
		OutputDebugStringA("Failed to destroy dummy window! \n");
		assert(false);
	}
}

void Window::Show() const
{
	ShowWindow(windowHandle, true);
}

HDC Window::GetDeviceContext() const
{
	return GetDC(windowHandle);
}

void Window::SwapFrontAndBackBuffers() const
{
	// TODO: Maybe try and research this SwapBuffers vs wglSwapLayerBuffers dilemma a bit more?
	// When swapping buffers, it's recommended to use the SwapBuffers function instead of wglSwapLayerBuffers: https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows
	// SwapBuffers exchanges the front and back buffer if the current pixel format for the window
	// referenced by the device context has a back buffer.
	SwapBuffers(GetDeviceContext());
}

HWND Window::CreateBaseWindow(std::wstring windowTitle, int windowWidth, int windowHeight, WNDPROC windowProcedure, const std::wstring windowClassName)
{
	const auto didGetModuleHandle = GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"",
		&applicationHandle);

	if (!didGetModuleHandle)
	{
		OutputDebugStringA("Failed to get the application handle! \n");
		assert(false);
	}

	WNDCLASSEX dummy;
	const auto didFindExistingClass = GetClassInfoEx(applicationHandle, windowClassName.c_str(), &dummy);

	if (!didFindExistingClass)
	{
		// Register window class
		// A Window Class describes a template for instances of windows.
		// They define a set of behaviour several windows might have in common.
		// Every window must be associated with a Window Class, even if you only ever create a single
		// window in your application.
		WNDCLASSEX windowClass = {};

		// Should always be set to sizeof(WNDCLASSEX)
		// Setting this member is required.
		windowClass.cbSize = sizeof(WNDCLASSEX);

		// The pointer to the application defined "Window Procedure".
		windowClass.lpfnWndProc = windowProcedure;

		// The handle to the application instance.
		windowClass.hInstance = applicationHandle;

		// A string that uniquely identifies the Window Class.
		windowClass.lpszClassName = windowClassName.c_str();

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
			errorMessage.append("\n");
			
			OutputDebugStringA("Failed to register window class! \n");
		}
	}

	const auto newWindowHandle = CreateWindowEx(
		0, // Optional window styles
		windowClassName.c_str(), // Name of the Window Class to use
		windowTitle.c_str(), // The text at the title bar of the window
		// The Window Style
		// WS_OVERLAPPEDWINDOW gives the window a title bar, a border, and minimize / maximize buttons
		WS_OVERLAPPEDWINDOW, 
		// Size and Position
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
		nullptr, // Parent Window
		nullptr, // Menu
		applicationHandle, // Instance Handle
		nullptr // Additional application data
	);

	if (newWindowHandle == nullptr)
	{
		OutputDebugStringA("Failed to create main application window! \n");
		assert(false);
	}

	return newWindowHandle;
}

HGLRC Window::CreateBasicOpenGlContext(HDC deviceContext)
{
	// WGLCREATECONTEXT SEEMS TO FAIL IF A PROPER PIXEL FORMAT IS NOT FORMED, THEREFORE WE ALSO DO THIS FOR THE DUMMY WINDOW
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
		OutputDebugStringA("Failed to match a pixel format. \n");
		assert(false);
	}

	// Now that we have a pixel format number, we can use it to set the pixel format of the
	// window's device context.
	const auto didSetPixelFormat = SetPixelFormat(deviceContext, pixelFormatNumber, &pixelFormatDescriptor);
	if (!didSetPixelFormat)
	{
		OutputDebugStringA("Failed to set pixel format of windows device context. \n");
		assert(false);
	}

	// Now that we've set the pixel format and thus described our framebuffer, we can create the OpenGL context.
	// wglCreateContext creates a new OpenGL context and returns a handle to it
	const auto openGlContextHandle = wglCreateContext(deviceContext);
	if (openGlContextHandle == nullptr)
	{
		OutputDebugStringA("Failed to create OpenGl dummy context! \n");
		assert(false);
	}

	// Before an OpenGL context can be used, it must be set to current.
	const auto didMakeOpenGlContextCurrent = wglMakeCurrent(deviceContext, openGlContextHandle);
	if (!didMakeOpenGlContextCurrent)
	{
		OutputDebugStringA("Failed to make OpenGL dummy context current! \n");
		assert(false);
	}

	return openGlContextHandle;
}

HGLRC Window::CreateAdvanceOpenGlContext(HDC deviceContext)
{
	// Load OpenGL functions using GLAD
	if (!gladLoadGL())
	{
		OutputDebugStringA("Failed to initialize OpenGL GLAD! \n");
		assert(false);
	}

	// Load WGL extension functions using GLAD
	// We need these for the extended WGL functions with more functionality when creating
	// OpenGL contexts.
	if (!gladLoadWGL(deviceContext))
	{
		OutputDebugStringA("Failed to initialize WGL GLAD! \n");
		assert(false);
	}

	// Create Window
	// If window creation fails, a NULL value is returned.
	const int attribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		// Important. Specification states that the attribute list
		// Has to terminate with a 0 value. Doing otherwise gives you a parameter error
		// When calling the WGL function
		0
	};

	int pixelFormat;
	UINT numFormats;

	const auto didChoosePixelFormatArb = wglChoosePixelFormatARB(deviceContext, attribList, nullptr, 1, &pixelFormat, &numFormats);
	if (!didChoosePixelFormatArb)
	{
		OutputDebugStringA("Failed to choose a pixel format ARB! \n");
		assert(false);
	}

	const auto didSetPixelFormat = SetPixelFormat(deviceContext, pixelFormat, nullptr);
	if (!didSetPixelFormat)
	{
		OutputDebugStringA("Failed to set pixel format! \n");
		assert(false);
	}
	
	const int contextAttributeList[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, // Enable debug context
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // Enable core profile
		// Important. Specification states that the attribute list
		// Has to terminate with a 0 value. Doing otherwise gives you a parameter error
		// When calling the WGL function
		0
	};

	// https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
	// On failure returns NULL
	const auto contextHandle = wglCreateContextAttribsARB(deviceContext, nullptr, contextAttributeList);

	if (!contextHandle)
	{
		OutputDebugStringA("Failed to create context! \n");
		assert(false);
	}

	const auto didMakeContextCurrent = wglMakeCurrent(deviceContext, contextHandle);
	if (!didMakeContextCurrent)
	{
		OutputDebugStringA("Failed to make context current! \n");
		assert(false);
	}

	// Debug messages will not be generated unless debug output is enabled
	glEnable(GL_DEBUG_OUTPUT);
	
	// Set up OpenGL debug messaging using KHR_debug extension
	glDebugMessageCallback(OpenGlMessageCallback, nullptr);

	return contextHandle;
}

void APIENTRY OpenGlMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::string sourceMessage = "Source: ";
	sourceMessage.append(std::to_string(source));
	sourceMessage.append("\n");

	std::string errorMessage = message;
	errorMessage.append("\n");
	
	OutputDebugStringA("**** OpenGL Error **** \n");
	OutputDebugStringA(sourceMessage.c_str());
	OutputDebugStringA(errorMessage.c_str());
	OutputDebugStringA("********************** \n");
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

