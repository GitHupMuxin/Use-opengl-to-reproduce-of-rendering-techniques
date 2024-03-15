#pragma once
#include <GLAD/glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "Shader.h"
#include "Texture.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
};
class ModelTexture : public Texture2D
{
public:
	std::string type;
	std::string path;
};

class Mesh
{
private:
	GLuint VAO, VBO, EBO;
	void setUpMesh();
public:
	std::vector<Vertex> verteices;
	std::vector<GLuint> indices;
	std::vector<ModelTexture> textures;

	inline const GLuint getVAO() { return this->VAO; }
	Mesh(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture);
	void Draw(const Shader& shader);
	void DrawInstance(const Shader& shader, const GLuint& amount);
};

