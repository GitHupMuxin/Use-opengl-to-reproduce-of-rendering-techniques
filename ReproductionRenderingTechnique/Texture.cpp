#include "Texture.h"
#include "stb_image.h"

Texture2D::Texture2D() 
{
	glGenTextures(1, &this->id);
}

Texture2D::Texture2D(const GLuint& ID) : id(ID) { }

Texture2D::Texture2D(const GLchar* path, const GLenum& e)
{
	glGenTextures(1, &this->id);

	this->init(path, e);
}

Texture2D::Texture2D(const GLint& Width, const GLint& Height, const GLenum& Fromat, const GLenum& saveFromat, const GLenum& saveType) : width(Width), height(Height), format(Fromat)
{
	glGenTextures(1, &this->id);

	this->init(format, saveFromat, saveType);
}

void Texture2D::init(const GLenum& format, const GLenum& saveFromat, const GLenum& saveType)
{
	glBindTexture(GL_TEXTURE_2D, this->id);

	glTexImage2D(GL_TEXTURE_2D, 0, format, this->width, this->height, 0, saveFromat, saveType, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::init(const GLchar* path, const GLenum& e)
{
	this->use();
	GLint nr = 0;
	GLubyte* data = stbi_load(path, &this->width, &this->height, &nr, 0);
	if (data)
	{
		this->format = GL_RED;
		if (nr == 1)
			this->format = GL_RED;
		else if (nr == 3)
			this->format = GL_RGB;
		else if (nr == 4)
			this->format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, this->id);
		if (e == GL_SRGB)
			glTexImage2D(GL_TEXTURE_2D, 0, e, this->width, this->height, 0, this->format, GL_UNSIGNED_BYTE, data);
		else
		{
			this->format = GL_RED;
			if (nr == 1)
				this->format = GL_RED;
			else if (nr == 3)
				this->format = GL_RGB;
			else if (nr == 4)
				this->format = GL_RGBA;
			glTexImage2D(GL_TEXTURE_2D, 0, this->format, this->width, this->height, 0, this->format, GL_UNSIGNED_BYTE, data);
		}
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

void Texture2D::generateMipmap()
{
	this->use();
	glGenerateMipmap(GL_TEXTURE_2D);
	this->unUse();
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

void CubeTexture::init(const std::vector<std::string>& faces, const GLenum& e)
{
	this->use();
	if (faces.size() != 6)
	{
		std::cout << "Face size less than 6 " << std::endl;
		return;
	}
	for (int i = 0; i < 6; i++)
	{
		GLint nr = 0;
		GLubyte* data = stbi_load(faces[i].c_str(), &this->width, &this->height, &nr, 0);
		if (data)
		{
			this->format = GL_RED;
			if (nr == 1)
				this->format = GL_RED;
			else if (nr == 3)
				this->format = GL_RGB;
			else if (nr == 4)
				this->format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, this->id);
			if (e == GL_SRGB)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, e, this->width, this->height, 0, this->format, GL_FLOAT, data);
			else
			{
				this->format = GL_RED;
				if (nr == 1)
					this->format = GL_RED;
				else if (nr == 3)
					this->format = GL_RGB;
				else if (nr == 4)
					this->format = GL_RGBA;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, this->format, this->width, this->height, 0, this->format, GL_FLOAT, data);
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CubeTexture::generateMipmap()
{
	this->use();
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	this->unUse();
}

void CubeTexture::init(const GLenum& format, const GLenum& saveFromat, const GLenum& saveType)
{
	this->use();
	for (GLuint i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, this->width, this->height, 0, saveFromat, saveType, nullptr);
}

void CubeTexture::Parameteri(const GLenum& target, const GLenum& value)
{
	this->use();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, target, value);
	this->unUse();
}

void CubeTexture::use() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
}

void CubeTexture::unUse() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeTexture::CubeTexture()
{
	glGenTextures(1, &this->id);
}

CubeTexture::CubeTexture(const GLuint& ID)
{
	this->id = ID;
}

CubeTexture::CubeTexture(const std::vector<std::string>& faces, const GLenum& e)
{
	glGenTextures(1, &this->id);
	this->init(faces, e);
}

CubeTexture::CubeTexture(const GLint& Width, const GLint& Height, const GLenum& Fromat, const GLenum& saveFromat, const GLenum& saveType) : width(Width), height(Height)
{
	glGenTextures(1, &this->id);
	this->init(Fromat, saveFromat, saveType);
}


ScreenRenderModel::ScreenRenderModel()
{
	this->verteices.resize(4);
	verteices[0].Position = glm::vec3(-1.0f, 1.0f, 0.0f);
	verteices[0].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	verteices[0].TexCoord = glm::vec2(0.0f, 1.0f);

	verteices[1].Position = glm::vec3(1.0f, 1.0f, 0.0f);
	verteices[1].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	verteices[1].TexCoord = glm::vec2(1.0f, 1.0f);

	verteices[2].Position = glm::vec3(-1.0f, -1.0f, 0.0f);
	verteices[2].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	verteices[2].TexCoord = glm::vec2(0.0f, 0.0f);

	verteices[3].Position = glm::vec3(1.0f, -1.0f, 0.0f);
	verteices[3].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	verteices[3].TexCoord = glm::vec2(1.0f, 0.0f);

	this->indeices = { 0, 1, 2, 1, 2, 3 };

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	
	glBufferData(GL_ARRAY_BUFFER, this->verteices.size() * sizeof(Vertex), &(this->verteices[0]), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indeices.size() * sizeof(GLuint), &(this->indeices[0]), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

	glBindVertexArray(0);
}

void ScreenRenderModel::Draw(const Texture2D& texture, const Shader& shader)
{
	this->shader = shader;
	this->shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	this->shader.setInt("level", 0);
	this->shader.setInt("Texture", 0);
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indeices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void ScreenRenderModel::Draw(const Texture2D& texture, const GLint& level, const Shader& shader)
{
	this->shader = shader;
	this->shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	this->shader.setInt("level", level);
	this->shader.setInt("Texture", 0);
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indeices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void ScreenRenderModel::Draw(const Shader& shader)
{
	this->shader = shader;
	this->shader.use();
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indeices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

Texture2DMultisample::Texture2DMultisample(const GLint& Width, const GLint& Height, const GLint& Samples, const GLenum& Fromat) : Texture2D()
{ 
	this->width = Width;
	this->height = Height;
	this->format = Fromat;
	this->samples = Samples;
	glGenTextures(1, &this->id);
	this->init();
}

void Texture2DMultisample::init()
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, this->samples, this->format, this->width, this->height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture2DMultisample::use() const
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->id);
}

void Texture2DMultisample::unUse() const
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}


