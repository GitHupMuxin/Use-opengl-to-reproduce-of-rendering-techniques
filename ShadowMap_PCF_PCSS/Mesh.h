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
#include "Vertex.h"

class Mesh
{
protected:
	GLuint VAO, VBO, EBO, textureCount;
	virtual void setUpMesh();
public:
	glm::vec3 ka, kd, ks;
	std::vector<Vertex> verteices;
	std::vector<GLuint> indices;
	std::vector<ModelTexture> textures;

	inline const GLuint getVAO() { return this->VAO; }
	explicit Mesh();
	Mesh(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka = glm::vec3(0.0), glm::vec3 Kd = glm::vec3(0.0),  glm::vec3 Ks = glm::vec3(0.0));
	void initData(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka = glm::vec3(0.0), glm::vec3 Kd = glm::vec3(0.0), glm::vec3 Ks = glm::vec3(0.0));
	virtual void bindingSH(const std::vector<GLfloat>& sh, GLuint SHOrder, GLuint offset);
	void Draw(const Shader& shader);
	void DrawInstance(const Shader& shader, const GLuint& amount);
};

class MeshWithSphericalHarmonics : public Mesh
{
protected:
	virtual void setUpMesh();
public:
	GLint SHOrder;
	std::vector<GLfloat> SH;
	virtual void bindingSH(const std::vector<GLfloat>& sh, GLuint SHSize, GLuint offset);
	MeshWithSphericalHarmonics(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka = glm::vec3(0.0), glm::vec3 Kd = glm::vec3(0.0), glm::vec3 Ks = glm::vec3(0.0));
};