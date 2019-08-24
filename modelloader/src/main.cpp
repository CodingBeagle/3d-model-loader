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
#include <vector>
#include <random>

#include "Window.h"
#include "Shader.h"

#include <assimp/Importer.hpp> // C++ Importer Interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

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

// Function Prototypes
std::vector<float> GetVertices(const aiNode* node, const aiScene* scene);

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

	Assimp::Importer importer;

	// By default all 3D data is provided in right-handed coordinate system (OpenGL also uses a right-hand coordinate system).
	// The nodes in the returned hierarchy do not directly store meshes. The meshes are found in the "aiMesh" property of the scene.
	// Each node simply refers to an index of this array.
	// A mesh lives inside the referred node's local coordinate system.
	// If you want the mesh's orientation in global space, you'd have to concatenate the transformations from the referring node and all
	// of its parents.
	// Each mesh use a single material only. Parts of the model using different materials will be separate meshes of the same node.
	// We use aiProcess_Triangulate pre-processing option to split up faces with more than 3 indices into triangles (so other faces of 3 indicies)
	// We use aiProcess_SortByPType to split up meshes with more than one primitive type into homogeneous sub-meshes.
	// We use these two post-processing steps because, for real-time 3d rendering, we are only (usually) interested in rendering a set of triangles
	// This way it will be easy for us to sort / ignore any other primitive type
	const aiScene* scene = importer.ReadFile("shaders/chair.blend",
		aiProcess_Triangulate | aiProcess_SortByPType);

	std::vector<float> modelVertices = GetVertices(scene->mRootNode, scene);

	int lol = 0;
	
	// The Z-buffer of OpenGL allows OpenGL to decide when to draw over a pixel
	// and when not to, based on depth testing.
	// OpenGL stores all depth information in a z-buffer, known as the "depth buffer".
	// GLFW automatically creates such a buffer for you.
	// However, we have to explicitly enable depth testing.
	// We do that here.
	glEnable(GL_DEPTH_TEST);

	// OpenGl Core REQUIRES us to use Vertex Array Objects (VAOs)
	// VAOs are OpenGL objects which will save state related to these calls:
	// -- Calls to glEnableVertexAttribArray or glDisableVertexAttribArray
	// -- Vertex attribute configurations via glVertexAttribPointer
	// -- Vertex buffer objects associated with vertex attributes by calls to glVertexAttribPointer
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// We generate an OpenGL buffer object
	// OpenGL buffers can be used for many things. They are simply allocated memory which can be used
	// to store whatever you want
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// Now we bind our generated buffer to the GL_ARRAY_BUFFER target. This essentially means that we will
	// be using it is a vertex buffer object.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	// Now that we have bound our buffer object to a target, we can start to make OpenGL calls to functions
	// That affect the state relevant for that object
	// Here we copy our vertice data to the GPU, to our newly created buffer object.
	// We also hint to OpenGL that the date most likely won't change. This means that OpenGL can make some assumptions
	// about the data which can be used to optimize it.
	glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(float), &modelVertices[0], GL_STATIC_DRAW);

	// In the vertex shader we specified that location 0 accepted a 3D vector as input
	// OpenGL is very flexible when it comes to how to feed input into that location
	// But that also means we have to describe how the buffer is structured
	// So that OpenGL knows how to take the x, y and z number of each vertex described
	// in our array
	// Parameter 1: The index of the location we want to input to
	// Parameter 2: The number of components per generic vertex attribute
	// -- We have 3 components, since our input is a Vec3 in the vertex shader
	// Parameter 3: The data type of each component.
	// -- They are 32-bit floats
	// Parameter 4: Should data be normalized. Should be FALSE for floats.
	// Parameter 5: The byte offset between each consecutive generic vertex attribute.
	// -- Our array is tightly packed, so 0 byte offset between them
	// Parameter 6: The byte offset of the first component of the first generic vertex
	// Attribute.
	// -- This is 0 for us. It begins at the start of the array.
	// NOTICE: glVertexAttribPointer reads the currently bound buffer in GL_ARRAY_BUFFER
	// and stores it in the VAO, so unbinding the buffer in GL_ARRAY_BUFFER will not affect
	// The currently bound VAO

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 6, (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	// Cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);

	// Vertex programming
	Shader myShader("./shaders/transvertex.glsl", "./shaders/fragment.glsl");
	myShader.activate();

	glBindVertexArray(VAO);

	float rotation = 0;
	float radius = 5.0f;
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	float aliveCounter = 0.0f;
	
	// Game Loop
	MSG msg = {};
	while (true)
	{	
		// Besides clearing the color buffer, we also want to clear the
		// depth buffer, otherwise depth information from the previous frame stays in the buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Our model matrix
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 1.0f));

		// Our view matrix
		float camX = sin(aliveCounter) * radius;
		float camZ = cos(aliveCounter) * radius;
		
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(
			glm::vec3(camX, 2.0f, camZ),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		// Our projection matrix
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		myShader.setMatrix("model", trans);
		myShader.setMatrix("view", view);
		myShader.setMatrix("projection", projection);

		glDrawArrays(GL_TRIANGLES, 0, modelVertices.size());
		
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

		mainWindow.SwapFrontAndBackBuffers();

		aliveCounter += 0.011f;
	}
	
	return 0;
}

std::vector<float> GetVertices(const aiNode* node, const aiScene* scene)
{
	std::vector<float> nodeVertices{};

	const auto numberOfMeshes = node->mNumMeshes;

	for (unsigned int i = 0; i < numberOfMeshes; i++)
	{
		const auto currentMeshIndex = node->mMeshes[i];
		const auto currentMesh = scene->mMeshes[currentMeshIndex];

		// We are only interested in rendering triangle primitives.
		// Everything else we ignore for now
		if (currentMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			for (unsigned int j = 0; j < currentMesh->mNumVertices; j++)
			{
				auto currentVertex = currentMesh->mVertices[j];

				// Position
				nodeVertices.push_back(currentVertex.x);
				nodeVertices.push_back(currentVertex.y);
				nodeVertices.push_back(currentVertex.z);

				// Color
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<double> dist(0.0, 1.0);

				nodeVertices.push_back(dist(mt));
				nodeVertices.push_back(dist(mt));
				nodeVertices.push_back(dist(mt));
			}
		}
	}

	// Base Case
	if (node->mNumChildren == 0)
		return nodeVertices;

	// Alternative Case
	const auto numberOfChildren = node->mNumChildren;
	for (unsigned int i = 0; i < numberOfChildren; i++)
	{
		const auto currentChild = node->mChildren[i];
		auto returnedVertices = GetVertices(currentChild, scene);
		nodeVertices.insert(nodeVertices.end(), returnedVertices.begin(), returnedVertices.end());
	}

	return nodeVertices;
}