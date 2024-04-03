#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "environmentBox.h"
#include "stb_image.h"

EnvironmentBox::EnvironmentBox()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);

	this->vertices = {
	   -1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f, -1.0f,

	   -1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f,  1.0f,
	   -1.0f, -1.0f,  1.0f,
 
 	    1.0f, -1.0f, -1.0f,
	    1.0f, -1.0f,  1.0f,
	    1.0f,  1.0f,  1.0f,
	    1.0f,  1.0f,  1.0f,
	    1.0f,  1.0f, -1.0f,
	    1.0f, -1.0f, -1.0f,

	   -1.0f, -1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
	    1.0f,  1.0f,  1.0f,
	    1.0f,  1.0f,  1.0f,
	    1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f,  1.0f,

	   -1.0f,  1.0f, -1.0f,
	    1.0f,  1.0f, -1.0f,
	    1.0f,  1.0f,  1.0f,
	    1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,

	   -1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,
	    1.0f, -1.0f, -1.0f,
	    1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,
	    1.0f, -1.0f,  1.0f
	};

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->vertices.size(), &(this->vertices[0]), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)0);

	this->shader.init("environmentBoxVertexShader.glsl", "environmentBoxFragmentShader.glsl");
}

void EnvironmentBox::bindTexture(const std::vector<std::string>& faces)
{
	glGenTextures(1, &this->textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureID);
	int width, height, nrChannels;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		GLubyte* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "cube texture failed to load at path " << faces[i] << std::endl;
			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
}

void EnvironmentBox::bindTexture(const std::string& path)
{
	std::vector<std::string> faces(6);
	faces[0] = path + "/posx";
	faces[1] = path + "/negx";
	faces[2] = path + "/posy";
	faces[3] = path + "/negy";
	faces[4] = path + "/posz";
	faces[5] = path + "/negz";
	this->bindTexture(faces);
}

void EnvironmentBox::Draw()
{
	glDepthFunc(GL_LEQUAL);
	this->shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureID);
	this->shader.setInt("skyBox", 0);
	glBindVertexArray(this->VAO);
	glDrawArrays(GL_TRIANGLES, 0, this->vertices.size() / 3);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void EnvironmentBoxWithSphericalHarmonics::loadLightCoeffs(const std::string& fileName)
{
	std::string coeffs = "";
	std::ifstream coeffsFile;
	coeffsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream coeffsStream;
		coeffsFile.open(fileName.c_str());
		coeffsStream << coeffsFile.rdbuf();
		coeffsFile.close();
		coeffs = coeffsStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "LightCoeffs load fail." << std::endl;
	}
	std::regex re("[\\s]");
	std::vector<std::string> result{ std::sregex_token_iterator(coeffs.begin(), coeffs.end(), re, -1), {} };
	for (auto& it : result)
		this->lightCoeffs.push_back(std::stof(it));
	this->SHOrder = sqrt(this->lightCoeffs.size()) - 1;
}

EnvironmentBoxWithSphericalHarmonics::EnvironmentBoxWithSphericalHarmonics(const std::string& fileName)
{
	this->loadLightCoeffs(fileName);
}

std::vector<GLfloat>& EnvironmentBoxWithSphericalHarmonics::getLightCoeffs()
{
	return this->lightCoeffs;
}