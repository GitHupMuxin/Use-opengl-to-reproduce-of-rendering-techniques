#pragma once
#include <GLAD/glad/glad.h>
#include "Texture.h"

class FrameBuffer
{
protected:
	GLuint width, height;
public:
	GLuint fbo;
	GLuint rbo;

	FrameBuffer();	
	void use();
	void binding();
	void unBinding();
	virtual void bufferStorage(const GLint& width, const GLint& height, const GLenum& e = GL_DEPTH32F_STENCIL8);
	virtual void BindTexture2D(const Texture2D& texture, const GLuint& level = 0, const GLenum& e = GL_COLOR_ATTACHMENT0);
	virtual void BindTexture(const GLuint& textureID, const GLenum& textureType = GL_TEXTURE_2D, const GLuint& level = 0, const GLenum& e = GL_COLOR_ATTACHMENT0);
	virtual void check();

	virtual ~FrameBuffer();
};

class FrameBufferMSAA : public FrameBuffer
{
protected:
	GLint samples;
public:
	FrameBufferMSAA(GLint T);
	void bufferStorage(const GLint& width, const GLint& height, const GLenum& e = GL_DEPTH32F_STENCIL8);
	void BindTexture2D(const Texture2DMultisample& texture, const GLuint& level = 0, const GLenum& e = GL_COLOR_ATTACHMENT0);
};