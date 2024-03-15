#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"


void Shader::init(const GLchar* vertexShaderPath, const GLchar* fragmentShaderPath, const GLchar* geometryShaderPath)
{
	std::string vertexCode = "", fragmentCode = "", geometryCode = "";
	std::ifstream vShaderFile, fShaderFile, gShaderFile;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		std::stringstream vShaderStream, fShaderStream, gShaderStream;

		vShaderFile.open(vertexShaderPath);
		fShaderFile.open(fragmentShaderPath);

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		if (geometryShaderPath != "")
		{
			gShaderFile.open(geometryShaderPath);
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}
	this->id = glCreateProgram();
	int success = 0;
	char infoLog[512];
	const char* vShaderCode = vertexCode.c_str();
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX_SHADER::COMPLILE_FAIL: \n" << infoLog << std::endl;
		infoLog[0] = '\0';
		success = false;
	}
	glAttachShader(this->id, vertex);

	const char* fShaderCode = fragmentCode.c_str();
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::COMPLILE_FAIL: \n" << infoLog << std::endl;
		infoLog[0] = '\n';
		success = false;
	}
	glAttachShader(this->id, fragment);

	if (!geometryCode.empty())
	{
		const char* gShaderCode = geometryCode.c_str();
		GLuint geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::COMPLILE_FAIL: \n" << infoLog << std::endl;
			infoLog[0] = '\n';
			success = false;
		}
		glAttachShader(this->id, geometry);
		glDeleteShader(geometry);
	}

	glLinkProgram(this->id);
	glGetProgramiv(this->id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM_LINK_FAIL: \n" << infoLog << std::endl;
		infoLog[0] = '\n';
		success = false;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader()
{
	this->id = 0;
}

Shader::Shader(const GLchar* vertexShaderPath, const GLchar* fragmentShaderPath, const GLchar* geometryShaderPath)
{
	this->id = 0;
	init(vertexShaderPath, fragmentShaderPath, geometryShaderPath);
}

void Shader::use()
{
	glUseProgram(this->id);
}

void Shader::setBool(const std::string& name, const bool& value) const
{
	glUniform1i(glGetUniformLocation(this->id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int& value) const
{
	glUniform1i(glGetUniformLocation(this->id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float& value) const
{
	glUniform1f(glGetUniformLocation(this->id, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(this->id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const float& x, const float& y, const float& z) const
{
	glUniform3f(glGetUniformLocation(this->id, name.c_str()), x, y, z);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(this->id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, const float& x, const float& y)
{
	glUniform2f(glGetUniformLocation(this->id, name.c_str()), x, y);
}

