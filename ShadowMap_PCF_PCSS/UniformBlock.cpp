#include "UniformBlock.h"

GLuint UniformBlock::blockIndex = 0;

UniformBlock::UniformBlock(GLint Size) : size(Size), offset(0), index(UniformBlock::blockIndex++)
{
	glGenBuffers(1, &this->uniformBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferData(GL_UNIFORM_BUFFER, this->size, NULL, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, this->index, this->uniformBlock, 0, size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const GLfloat data)
{
	if (sizeof(GLfloat) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(GLfloat);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, &data);
	this->offset += step;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const GLfloat data, const GLuint& offset)
{
	if (sizeof(GLfloat) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(GLfloat);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, step, &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::vec2& data)
{
	if (sizeof(glm::vec2) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec2);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data));
	this->offset += step;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::vec2& data, const GLuint& offset)
{
	if (sizeof(glm::vec2) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec2);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, step, glm::value_ptr(data));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::vec3& data)
{
	if (sizeof(glm::vec4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data));
	this->offset += step;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::vec3& data, const GLuint& offset)
{
	if (sizeof(glm::vec4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, step, glm::value_ptr(data));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::mat3x3& data)
{
	if (sizeof(glm::mat4x4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data));
	this->offset += step;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::mat3x3& data, const GLuint& offset)
{
	if (sizeof(glm::mat4x4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, step, glm::value_ptr(data));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::mat4x4& data)
{
	if (sizeof(glm::mat4x4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data));
	this->offset += step;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const glm::mat4x4& data, const GLuint& offset)
{
	if (sizeof(glm::mat4x4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, step, glm::value_ptr(data));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::vec2>& data)
{
	if (data.size() * sizeof(glm::vec2) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec2);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data[i]));
		this->offset += step;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::vec2>& data, const GLuint& offset)
{
	if (data.size() * sizeof(glm::vec2) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec2);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
		glBufferSubData(GL_UNIFORM_BUFFER, offset + i * step, step, glm::value_ptr(data[i]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::vec3>& data)
{
	if (data.size() * sizeof(glm::vec4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data[i]));
		this->offset += step;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::vec3>& data, const GLuint& offset)
{
	if (data.size() * sizeof(glm::vec4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::vec4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
		glBufferSubData(GL_UNIFORM_BUFFER, offset + i * step, step, glm::value_ptr(data[i]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::mat3x3>& data)
{
	if (data.size() * sizeof(glm::mat4x4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data[i]));
		this->offset += step;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::mat3x3>& data, const GLuint& offset)
{
	if (data.size() * sizeof(glm::mat4x4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
		glBufferSubData(GL_UNIFORM_BUFFER, offset + i * step, step, glm::value_ptr(data[i]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void UniformBlock::insert(const std::vector<glm::mat4x4>& data)
{
	if (data.size() * sizeof(glm::mat4x4) + this->offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, this->offset, step, glm::value_ptr(data[i]));
		this->offset += step;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::insert(const std::vector<glm::mat4x4>& data, const GLuint& offset)
{
	if (data.size() * sizeof(glm::mat4x4) + offset > this->size)
	{
		std::cout << "ERROR::UNIFORM_BLOCK : data to big." << std::endl;
		return;
	}

	GLuint step = sizeof(glm::mat4x4);

	glBindBuffer(GL_UNIFORM_BUFFER, this->uniformBlock);
	for (GLint i = 0; i < data.size(); i++)
		glBufferSubData(GL_UNIFORM_BUFFER, offset + i * step, step, glm::value_ptr(data[i]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::bindingUniformBlock(const Shader& shader, std::string str)
{
	GLuint BindingBlockIndex = glGetUniformBlockIndex(shader.id, str.c_str());
	glUniformBlockBinding(shader.id, BindingBlockIndex, this->index);
}
