#include "Mesh.hpp"

Mesh::Mesh(std::string filepath)
{
	vertices = std::vector<float>{};
	indices = std::vector<unsigned>{};

	LoadMesh(filepath);
	GenerateTexture();
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

void Mesh::Draw()
{
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
