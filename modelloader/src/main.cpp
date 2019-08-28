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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// The Windows API is used by including the Windows.h header.
#include <Windows.h>
#include <string>
#include <vector>
#include <random>

#include "Window.h"
#include "Shader.h"

#include <assimp/Importer.hpp> // C++ Importer Interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "Mesh.hpp"

// Cube Vertex Data
float verticesCube[] = {
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f, 0.5f, 0.5f, 0.2f,
	-0.5f,  0.5f, -0.5f, 0.7f, 0.9f, 0.5f,
	-0.5f, -0.5f, -0.5f, 0.2f, 0.3f, 0.1f,

	-0.5f, -0.5f,  0.5f, 0.1f, 0.3f, 0.0f,
	 0.5f, -0.5f,  0.5f, 0.5f, 0.5f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.4f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.3f, 0.3f, 0.3f,
	-0.5f,  0.5f,  0.5f, 0.1f, 0.1f, 0.1f,
	-0.5f, -0.5f,  0.5f, 0.5f, 0.5f, 1.0f,

	-0.5f,  0.5f,  0.5f, 0.1f, 1.0f, 0.1f,
	-0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 0.2f,
	-0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 0.2f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f, 0.1f, 0.2f, 0.9f,
	 0.5f,  0.5f, -0.5f, 0.9f, 0.8f, 0.7f,
	 0.5f, -0.5f, -0.5f, 0.1f, 1.0f, 0.1f,
	 0.5f, -0.5f, -0.5f, 0.4f, 0.2f, 0.1f,
	 0.5f, -0.5f,  0.5f, 1.0f, 0.9f, 0.8f,
	 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,

	-0.5f, -0.5f, -0.5f, 0.1f, 1.0f, 0.1f,
	 0.5f, -0.5f, -0.5f, 1.0f, 0.2f, 0.8f,
	 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 0.1f, 0.2f, 0.3f,
	-0.5f, -0.5f,  0.5f, 1.0f, 0.1f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.1f, 0.1f, 1.0f,

	-0.5f,  0.5f, -0.5f, 0.1f, 1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f, 1.0f, 0.1f, 0.5f,
	 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.2f,
	-0.5f,  0.5f,  0.5f, 0.5f, 0.2f, 0.1f,
	-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.5f
};

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
	const Window mainWindow{ L"3D Model Loader", 800, 600 };
	
	std::string openGlVersion = "OpenGL version used: ";
	openGlVersion.append((char*)glGetString(GL_VERSION));
	openGlVersion.append("\n");

	OutputDebugStringA(openGlVersion.c_str());

	Mesh myAwesomeMesh{ "shaders/export.beagleasset" };
	myAwesomeMesh.SetPosition(3, 0, 0);

	Mesh myAwesomeMesh2{ "shaders/cylinder.beagleasset" };
	myAwesomeMesh2.SetPosition(0, 0, 0);
	
	// The Z-buffer of OpenGL allows OpenGL to decide when to draw over a pixel
	// and when not to, based on depth testing.
	// OpenGL stores all depth information in a z-buffer, known as the "depth buffer".
	// GLFW automatically creates such a buffer for you.
	// However, we have to explicitly enable depth testing.
	// We do that here.
	glEnable(GL_DEPTH_TEST);

	// Vertex programming
	Shader myShader("./shaders/transvertex.glsl", "./shaders/fragment.glsl");
	myShader.activate();
	
	
	float radius = 7.0f;
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	float aliveCounter = 0.0f;
	
	// Game Loop
	MSG msg = {};
	while (true)
	{	
		// Besides clearing the color buffer, we also want to clear the
		// depth buffer, otherwise depth information from the previous frame stays in the buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Our view matrix
		float camX = sin(aliveCounter) * radius;
		float camZ = cos(aliveCounter) * radius;
		
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(
			glm::vec3(camX, 6.0f, camZ),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		// Our projection matrix
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000000.0f);
		
		myShader.setMatrix("view", view);
		myShader.setMatrix("projection", projection);

		myAwesomeMesh.Draw(myShader);
		myAwesomeMesh2.Draw(myShader);
		
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

		mainWindow.SwapFrontAndBackBuffers();

		aliveCounter += 0.011f;
	}
	
	return 0;
}
