#pragma once

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

#include <Windows.h>

#include <string>
#include <vector>
#include <fstream>
#include <cassert> // For assert
#include <sstream> // For stringstream

#include "stb_image.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class Mesh
{
public:
	explicit Mesh(std::string filepath);
	std::string GetTexturePath() const;
	std::vector<float> GetVertices() const;
	std::vector<unsigned> GetIndices() const;
	unsigned GetTextureObject() const;
	void Draw(Shader shader);
	void SetPosition(float x, float y, float z);
private:
	glm::mat4 modelMatrix;
	void LoadMesh(std::string filepath);
	void GenerateTexture();
	void UploadVertexData();
	std::vector<float> vertices;
	std::vector<unsigned> indices;
	std::string texturePath;
	unsigned textureObject;
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
	float pos_x;
	float pos_y;
	float pos_z;
};
