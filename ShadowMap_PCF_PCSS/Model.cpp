#include <fstream>
#include <sstream>
#include <regex>
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Model::Model() { }

Model::Model(const char* path)
{
	loadModel(path);
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);;
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode* node, const aiScene* scene)
{
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	for (GLuint i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

Mesh* Model::processMesh(const aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> verteices;
	std::vector<GLuint> indices;
	std::vector<ModelTexture> textures;
	glm::vec3 ka = glm::vec3(0.0), kd = glm::vec3(0.0), ks = glm::vec3(0.0);
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vec;
		vec.x = mesh->mVertices[i].x;
		vec.y = mesh->mVertices[i].y;
		vec.z = mesh->mVertices[i].z;
		vertex.Position = vec;
		if (mesh->HasNormals())
		{
			vec.x = mesh->mNormals[i].x;
			vec.y = mesh->mNormals[i].y;
			vec.z = mesh->mNormals[i].z;
			vertex.Normal = vec;
		}
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoord = vec;
		}
		else
			vertex.TexCoord = glm::vec2(0.0f);
		verteices.push_back(vertex);
	}

	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color;

		std::vector<ModelTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<ModelTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<ModelTexture> reflectionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
		textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());

		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		ka = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		kd = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		ks = glm::vec3(color.r, color.g, color.b);
	}

	return new Mesh(verteices, indices, textures, ka, kd, ks);
}

std::vector<ModelTexture> Model::loadMaterialTextures(const aiMaterial* mat, const aiTextureType& type, const std::string& typeName)
{
	std::vector<ModelTexture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(this->textures_loaded[j].path.data(), str.C_Str()))
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			ModelTexture texture;
			if (typeName == "texture_diffuse")
				texture.id = TextureFromFile(str.C_Str(), directory, GL_SRGB);
			else
				texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

void Model::init(const char* path)
{
	loadModel(path);
}

void Model::Draw(const Shader& shader)
{
	for (GLuint i = 0; i < meshes.size(); i++)
		meshes[i]->Draw(shader);
}

void Model::DrawInstance(const Shader& shader, const GLuint& amount)
{
	for (GLuint i = 0; i < meshes.size(); i++)
		meshes[i]->DrawInstance(shader, amount);
}

Model::~Model()
{
	for (int i = 0; i < meshes.size(); i++)
		delete meshes[i];
}


GLuint TextureFromFile(const char* path, const std::string& directory, const GLenum& e)
{
	std::string fileName = std::string(path);
	fileName = directory + '/' + fileName;
	return Texture2D(fileName.c_str(), e).id;
}

Mesh* ModelWithSphericalHarmonics::processMesh(const aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> verteices;
	std::vector<GLuint> indices;
	std::vector<ModelTexture> textures;
	glm::vec3 ka = glm::vec3(0.0), kd = glm::vec3(0.0), ks = glm::vec3(0.0);

	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vec;
		vec.x = mesh->mVertices[i].x;
		vec.y = mesh->mVertices[i].y;
		vec.z = mesh->mVertices[i].z;
		vertex.Position = vec;
		if (mesh->HasNormals())
		{
			vec.x = mesh->mNormals[i].x;
			vec.y = mesh->mNormals[i].y;
			vec.z = mesh->mNormals[i].z;
			vertex.Normal = vec;
		}
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoord = vec;
		}
		else
			vertex.TexCoord = glm::vec2(0.0f);
		verteices.push_back(vertex);
	}

	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color;

		std::vector<ModelTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<ModelTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<ModelTexture> reflectionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
		textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());

		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		ka = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		kd = glm::vec3(color.r, color.g, color.b);
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		ks = glm::vec3(color.r, color.g, color.b);
	}

	return new MeshWithSphericalHarmonics(verteices, indices, textures, ka, kd, ks);
}

void ModelWithSphericalHarmonics::init(const char* path, const std::string& fileName)
{
	std::ifstream coeffsFile;
	coeffsFile.open(fileName.c_str());
	float value;
	coeffsFile >> value;
	while (coeffsFile >> value)
		this->SH.push_back(value);
	this->loadModel(path);
	GLuint vertexSize = 0;
	for (auto& it : this->meshes)
		vertexSize += it->verteices.size();
	this->SHOrder = sqrt(this->SH.size() / vertexSize) - 1;

	GLuint offset = 0;
	GLuint SHSize = (this->SHOrder + 1) * (this->SHOrder + 1);
	for (GLint i = this->meshes.size() - 1; i >= 0; i--)
	{
		this->meshes[i]->bindingSH(this->SH, SHSize, offset);
		offset += this->meshes[i]->verteices.size() * SHSize;
	}
}

ModelWithSphericalHarmonics::ModelWithSphericalHarmonics() { }


ModelWithSphericalHarmonics::ModelWithSphericalHarmonics(const char* path, const std::string& fileName)
{
	this->init(path, fileName);
}