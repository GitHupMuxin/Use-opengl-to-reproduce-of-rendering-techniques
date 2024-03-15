#include "Texture.h"
#include "stb_image.h"

Texture2D::Texture2D() 
{
	this->id = 0;
}

Texture2D::Texture2D(const GLchar* path)
{
	this->init(path);
}

Texture2D::Texture2D(const GLint& Width, const GLint& Height, const GLenum& format) : width(Width), height(Height)
{
	this->init(format);
}

void Texture2D::init(const GLenum& format)
{

	glGenTextures(1, &this->id);
	glBindTexture(GL_TEXTURE_2D, this->id);

	glTexImage2D(GL_TEXTURE_2D, 0, format, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::init(const GLchar* path)
{
	glGenTextures(1, &this->id);
	GLint width, height, nrComponents;
	GLubyte* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, this->id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Parameteri(const GLenum& target, const GLenum& value)
{
	this->use();
	glTexParameteri(GL_TEXTURE_2D, target, value);
	this->unUse();
}

void Texture2D::Parameteri(const std::vector<GLenum>& enumVec)
{
	this->use();
	for (std::vector<GLenum>::const_iterator it = enumVec.begin(); it != enumVec.end(); it += 2)
		glTexParameteri(GL_TEXTURE_2D, *it, *(it + 1));
	this->unUse();
}

void Texture2D::use() const
{
	glBindTexture(GL_TEXTURE_2D, this->id);
}

void Texture2D::unUse() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
