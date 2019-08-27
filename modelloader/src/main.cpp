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

#include "Mesh.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

std::vector<unsigned int> indices{};

unsigned int textureObject = 0;

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
	
	auto modelVertices = std::vector<float>{};
	
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

	// Generate EBO
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
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
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture UV attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (void*)(sizeof(float) * 3));
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
	glBindTexture(GL_TEXTURE_2D, textureObject);
	
	float rotation = 0;
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

		// Our model matrix
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 1.0f));

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

		myShader.setMatrix("model", trans);
		myShader.setMatrix("view", view);
		myShader.setMatrix("projection", projection);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
		
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

unsigned int globalIndiceCount = 0;

std::vector<float> GetVertices(const aiNode* node, const aiScene* scene)
{
	std::vector<float> nodeVertices{};

	const auto numberOfMeshes = node->mNumMeshes;

	for (unsigned int i = 0; i < numberOfMeshes; i++)
	{
		const auto currentMeshIndex = node->mMeshes[i];
		const auto currentMesh = scene->mMeshes[currentMeshIndex];

		auto rofl = currentMesh->mNumVertices;

		// We are only interested in rendering triangle primitives.
		// Everything else we ignore for now
		if (currentMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			auto currentGlobalCount = globalIndiceCount;
			
			// Materials
			auto theMaterialIndex = currentMesh->mMaterialIndex;
			auto theMaterial = scene->mMaterials[theMaterialIndex];

			aiString path;
			if (aiGetMaterialTexture(theMaterial, aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
			{
				auto pathToImage = std::string{"shaders/"} + std::string{ path.C_Str() };

				// First step in loading a texture is to create a Texture Object.
				// By this point it won't have any dimensionality or type.
				glGenTextures(1, &textureObject);

				// The dimensionality or type is determined the first time you bind the texture
				// To a texture target using glBindTexture. Here, we bind it to the GL_TEXTURE_2D,
				// Making it a 2D texture.
				glBindTexture(GL_TEXTURE_2D, textureObject);

				// Texture coordinates are given in the space of 0.0 to 1.0 on each axis.
				// If the texture coordinates provided to OpenGL's built-in functions are somehow
				// Outside of this range, they have to be brought back into the range. How this is done
				// Can be controlled by the parameters GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T.
				// When the mode is GL_CLAMP_TO_BORDER, an attempt to read outside the 0.0 to 1.0 range
				// Will result in the constant border color for the texture to be used as a final value.
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

				// GL_TEXTURE_MIN_FILTER controls how texels are constructed when the mipmap
				// level is greater than zero. There are a total of six setting available for
				// this parameter.
				// Choosing GL_NEAREST or GL_LINEAR will disable mipmapping and will cause OpenGL
				// to only use the base level (level 0).
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				int width, height, nrChannels;
				const auto data = stbi_load(pathToImage.c_str(), &width, &height, &nrChannels, 0);
				if (data)
				{
					// After having created a texture object and specified its dimensionality,
					// We need to specify storage and data for the texture.
					// glTexImage2D is a MUTABLE texture image specification command.
					// It is, however, best practice to declare texture storage as immutable (meaning
					// they can't be resized or have their format changed, etc...).
					// glTexImage2D can also (optionally) provide the initial data.
					// InternalFormat = Specifies the format with which OpenGL should store the texels
					// In the texture.
					// The format of the initial texel data is given by the combination of FORMAT and TYPE.
					// OpenGL will convert the specified data from this format into the internal format
					// Specified by InternalFormat.
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

					// OpenGL provides a function to automatically generate all of the mipmaps for a texture.
					// It's up to the OpenGL implementation to provide a mechanism to downsample the high
					// resolution images to produce the lower resolution mipmaps.
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				else
				{
					OutputDebugStringA("Failed to load mesh texture!");
					assert(false);
				}

				// Clean Up
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			auto uvChannel = currentMesh->mTextureCoords[0];
			
			for (unsigned int j = 0; j < currentMesh->mNumVertices; j++)
			{
				auto currentVertex = currentMesh->mVertices[j];
				
				// Position
				nodeVertices.push_back(currentVertex.x);
				nodeVertices.push_back(currentVertex.y);
				nodeVertices.push_back(currentVertex.z);

				globalIndiceCount += 1;

				// Texture Coordinate
				auto textureCoordinate = uvChannel[j];
				nodeVertices.push_back(textureCoordinate.x);
				nodeVertices.push_back(textureCoordinate.y);
			}

			for (unsigned int q = 0; q < currentMesh->mNumFaces; q++)
			{
				const auto currentFace = currentMesh->mFaces[q];

				for (unsigned int rofl = 0; rofl < currentFace.mNumIndices; rofl++)
				{
					const auto currentIndex = currentFace.mIndices[rofl] + (currentGlobalCount);
					indices.push_back(currentIndex);
				}
			}
		} else
		{
			OutputDebugStringA("MET NON TRIANGLE!");
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

