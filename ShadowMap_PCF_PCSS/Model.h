#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Mesh.h"


GLuint TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

class Model
{
private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<ModelTexture> textures_loaded;

	void loadModel(const std::string& path);
	void processNode(const aiNode* node, const aiScene* scene);
	Mesh processMesh(const aiMesh* mesh, const aiScene* scene);
	std::vector<ModelTexture> loadMaterialTextures(const aiMaterial* mat, const aiTextureType& type, const std::string& typeName);
public:
	Model();
	Model(const char* path);
	void init(const char* path);
	inline Mesh& getMesh(GLuint i) { return meshes[i]; }
	inline const Mesh& getMesh(GLuint i) const { return meshes[i]; }
	inline std::vector<Mesh>& getMeshArray() { return meshes; }
	void Draw(const Shader& shader);
	void DrawInstance(const Shader& shader, const GLuint& amount);
};

