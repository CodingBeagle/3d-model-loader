#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cassert> // For assert
#include <sstream> // For stringstream

#include <assimp/Importer.hpp> // C++ Importer Interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <glad/glad.h>

class Mesh
{
public:
	explicit Mesh(std::string filepath);
	std::string GetTexturePath() const;
	std::vector<float> GetVertices() const;
	std::vector<unsigned> GetIndices() const;
	void Draw();
private:
	void LoadMesh(std::string filepath);
	std::vector<float> vertices;
	std::vector<unsigned> indices;
	std::string texturePath;
};
