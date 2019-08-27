#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cassert> // For assert
#include <sstream> // For stringstream

class Mesh
{
public:
	explicit Mesh(std::string filepath);
	void Draw();
private:
	void LoadMesh(std::string filepath);
	std::vector<float> vertices;
	std::vector<unsigned> indices;
};
