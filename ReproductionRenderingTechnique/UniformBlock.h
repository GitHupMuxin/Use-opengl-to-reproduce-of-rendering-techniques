#pragma once
#include <GLAD/glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "Shader.h"


class UniformBlock
{
private:
	static GLuint blockIndex;
	GLuint index;
	GLuint uniformBlock;
	GLuint size;
	GLuint offset;
public:
	UniformBlock(GLint Size);
	void insert(const GLfloat data);
	void insert(const GLfloat data, const GLuint& offset);

	void insert(const glm::vec2& data);
	void insert(const glm::vec2& data, const GLuint& offset);

	void insert(const glm::vec3& data);
	void insert(const glm::vec3& data, const GLuint& offset);

	void insert(const glm::mat3x3& data);
	void insert(const glm::mat3x3& data, const GLuint& offset);

	void insert(const glm::mat4x4& data);
	void insert(const glm::mat4x4& data, const GLuint& offset);

	void insert(const std::vector<glm::vec2>& data);
	void insert(const std::vector<glm::vec2>& data, const GLuint& offset);

	void insert(const std::vector<glm::vec3>& data);
	void insert(const std::vector<glm::vec3>& data, const GLuint& offset);

	void insert(const std::vector<glm::mat3x3>& data);
	void insert(const std::vector<glm::mat3x3>& data, const GLuint& offset);

	void insert(const std::vector<glm::mat4x4>& data);
	void insert(const std::vector<glm::mat4x4>& data, const GLuint& offset);

	void bindingUniformBlock(const Shader& shader, std::string str);
};



