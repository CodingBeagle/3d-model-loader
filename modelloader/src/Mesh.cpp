#include "Mesh.hpp"

Mesh::Mesh(std::string filepath)
{
	vertices = std::vector<float>{};
	indices = std::vector<unsigned>{};

	LoadMesh(filepath);
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