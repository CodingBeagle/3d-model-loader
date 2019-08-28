#include "Mesh.hpp"

Mesh::Mesh(std::string filepath)
{
	vertices = std::vector<float>{};
	indices = std::vector<unsigned>{};

	LoadMesh(filepath);
	GenerateTexture();
	UploadVertexData();
}

std::string Mesh::GetTexturePath() const
{
	return texturePath;
}

std::vector<float> Mesh::GetVertices() const
{
	return vertices;
}

std::vector<unsigned> Mesh::GetIndices() const
{
	return indices;
}

unsigned Mesh::GetTextureObject() const
{
	return textureObject;
}

float rotation = 0;

void Mesh::Draw(Shader shader)
{
	// Prepare texture
	glBindTexture(GL_TEXTURE_2D, textureObject);
	
	// Prepare vertex data
	glBindVertexArray(vao);

	// Transform
	modelMatrix = glm::mat4{ 1.0f };
	modelMatrix = glm::translate(modelMatrix, glm::vec3(pos_x, pos_y, pos_z));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 1.0f));
	shader.setMatrix("model", modelMatrix);
	
	// Render
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

	// Cleanup
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Mesh::SetPosition(float x, float y, float z)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
}

void Mesh::LoadMesh(const std::string filepath)
{
	// ifstreams are streams used for reading from a file
	// When you provide a file path in the constructor, the stream will be attach to that file
	// The second parameter, the mode, is in this case specified as ios::in (the mode for reading a file)
	std::ifstream assetFile{filepath, std::ios::in};

	if (!assetFile.good())
		assert(false);

	std::string currentLine{};
	while (!assetFile.eof())
	{
		std::getline(assetFile, currentLine);

		if (currentLine.length() > 0)
		{
			const auto startSymbol = currentLine.front();

			if (std::tolower(startSymbol) == 'v')
			{
				currentLine.erase(0, 2);

				std::string currentCharacter{};
				std::stringstream lol{ currentLine };
				std::vector<std::string> values{};
				while (std::getline(lol, currentCharacter, ','))
				{
					values.push_back(currentCharacter);
				}

				float vertex_pos_x = std::stof(values[0]);
				float vertex_pos_y = std::stof(values[1]);
				float vertex_pos_z = std::stof(values[2]);

				vertices.push_back(vertex_pos_x);
				vertices.push_back(vertex_pos_y);
				vertices.push_back(vertex_pos_z);

				float uv_map_x = std::stof(values[3]);
				float uv_map_y = std::stof(values[4]);

				vertices.push_back(uv_map_x);
				vertices.push_back(uv_map_y);
			} else if (std::tolower(startSymbol) == 'f')
			{
				currentLine.erase(0, 2);

				std::string currentCharacter{};
				std::stringstream currentLineStream{ currentLine };
				std::vector<std::string> values{};
				while (std::getline(currentLineStream, currentCharacter, ','))
				{
					values.push_back(currentCharacter);
				}

				int indice1 = std::stoi(values[0]);
				int indice2 = std::stoi(values[1]);
				int indice3 = std::stoi(values[2]);

				indices.push_back(indice1);
				indices.push_back(indice2);
				indices.push_back(indice3);
			} else if (std::tolower(startSymbol) == 't')
			{
				currentLine.erase(0, 2);
				texturePath = std::string{ "shaders/" } + std::string{currentLine};
			}
		}
	}
}

void Mesh::GenerateTexture()
{
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
	const auto data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
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

void Mesh::UploadVertexData()
{
	// OpenGl Core REQUIRES us to use Vertex Array Objects (VAOs)
	// VAOs are OpenGL objects which will save state related to these calls:
	// -- Calls to glEnableVertexAttribArray or glDisableVertexAttribArray
	// -- Vertex attribute configurations via glVertexAttribPointer
	// -- Vertex buffer objects associated with vertex attributes by calls to glVertexAttribPointer
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Generate EBO
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// We generate an OpenGL buffer object
	// OpenGL buffers can be used for many things. They are simply allocated memory which can be used
	// to store whatever you want
	glGenBuffers(1, &vbo);

	// Now we bind our generated buffer to the GL_ARRAY_BUFFER target. This essentially means that we will
	// be using it is a vertex buffer object.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Now that we have bound our buffer object to a target, we can start to make OpenGL calls to functions
	// That affect the state relevant for that object
	// Here we copy our vertice data to the GPU, to our newly created buffer object.
	// We also hint to OpenGL that the date most likely won't change. This means that OpenGL can make some assumptions
	// about the data which can be used to optimize it.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

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
}