#include <iostream>
#include <string>
#include <fstream>

#include <assimp/Importer.hpp> // C++ Importer Interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

void ExportModel(const aiNode* node, const aiScene* scene);

int main(int argc, char* argv[])
{
	std::cout << "Executing the epic beagle asset importer!" << std::endl;
	
	if (argc == 2)
	{
		const std::string providedFile{ argv[1] };
		std::cout << "Provided file: " << providedFile << std::endl;

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
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(providedFile.c_str(), aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FlipUVs);

		if (scene == nullptr)
		{
			std::cout << "Failed to load the file." << std::endl;
			std::cout << "Error: " << importer.GetErrorString() << std::endl;
			getchar();
			return -1;
		}

		ExportModel(scene->mRootNode, scene);
	}
	else
	{
		std::cout << "No file provided." << std::endl;
		getchar();
	}
		
 }

unsigned int globalIndiceCount = 0;

void ExportModel(const aiNode* node, const aiScene* scene)
{
	std::ofstream exportedFile;
	exportedFile.open("export.beagleasset");
	
	const auto numberOfMeshes = node->mNumMeshes;

	for (unsigned int i = 0; i < numberOfMeshes; i++)
	{
		const auto currentMeshIndex = node->mMeshes[i];
		const auto currentMesh = scene->mMeshes[currentMeshIndex];

		// We are only interested in rendering triangle primitives.
		// Everything else we ignore for now
		if (currentMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			const auto currentGlobalCount = globalIndiceCount;

			// Materials
			const auto theMaterialIndex = currentMesh->mMaterialIndex;
			const auto theMaterial = scene->mMaterials[theMaterialIndex];

			const auto uvChannel = currentMesh->mTextureCoords[0];

			for (unsigned int j = 0; j < currentMesh->mNumVertices; j++)
			{
				globalIndiceCount += 1;
				auto currentVertex = currentMesh->mVertices[j];
				auto textureCoordinate = uvChannel[j];
				
				exportedFile << "v:" << currentVertex.x << "," << currentVertex.y << "," << currentVertex.z << "," << textureCoordinate.x << "," << textureCoordinate.y << "\n";
			}

			for (unsigned int q = 0; q < currentMesh->mNumFaces; q++)
			{
				exportedFile << "f:";
				
				const auto currentFace = currentMesh->mFaces[q];
				
				for (unsigned int rofl = 0; rofl < currentFace.mNumIndices; rofl++)
				{
					const auto currentIndex = currentFace.mIndices[rofl] + (currentGlobalCount);
					
					exportedFile << currentIndex;

					if (rofl != 2)
						exportedFile << ",";
				}

				exportedFile << "\n";
			}

			aiString path;
			if (aiGetMaterialTexture(theMaterial, aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
			{
				auto textureName = std::string{ path.C_Str() };

				exportedFile << "t:" << textureName << "\n";
			}
		}
	}

	// Base Case
	if (node->mNumChildren == 0)
		return;

	// Alternative Case
	const auto numberOfChildren = node->mNumChildren;
	for (unsigned int i = 0; i < numberOfChildren; i++)
	{
		const auto currentChild = node->mChildren[i];
		ExportModel(currentChild, scene);
	}

	exportedFile.close();
}