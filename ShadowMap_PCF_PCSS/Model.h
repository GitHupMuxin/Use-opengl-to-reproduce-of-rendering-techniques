#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Mesh.h"


GLuint TextureFromFile(const char* path, const std::string& directory, const GLenum& e = GL_RGB);

class Model
{
protected:
	std::vector<Mesh* > meshes;
	std::string directory;
	std::vector<ModelTexture> textures_loaded;

	void loadModel(const std::string& path);
	void processNode(const aiNode* node, const aiScene* scene);
	virtual Mesh* processMesh(const aiMesh* mesh, const aiScene* scene);
	std::vector<ModelTexture> loadMaterialTextures(const aiMaterial* mat, const aiTextureType& type, const std::string& typeName);
public:
	Model();
	Model(const char* path);
	void init(const char* path);
	inline Mesh& getMesh(GLuint i) { return *(this->meshes[i]); }
	inline const Mesh& getMesh(GLuint i) const { return *(this->meshes[i]); }
	inline std::vector<Mesh*>& getMeshArray() { return meshes; }
	void Draw(const Shader& shader);
	void DrawInstance(const Shader& shader, const GLuint& amount);
	virtual ~Model();
};

class ModelWithSphericalHarmonics : public Model
{
protected:
	GLuint SHOrder;
	std::vector<GLfloat> SH;
	Mesh* processMesh(const aiMesh* mesh, const aiScene* scene);
public:
	void init(const char* path, const std::string& fileName);
	explicit ModelWithSphericalHarmonics();
	ModelWithSphericalHarmonics(const char* path, const std::string& fileName);
};

typedef ModelWithSphericalHarmonics SHModel;