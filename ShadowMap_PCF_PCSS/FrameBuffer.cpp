#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
	glGenFramebuffers(1, &this->fbo);
	glGenRenderbuffers(1, &this->rbo);
}

void FrameBuffer::binding()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}

void FrameBuffer::unBinding()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::use()
{
	this->binding();
	glViewport(0, 0, this->width, this->height);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	this->unBinding();
}

void FrameBuffer::bufferStorage(const GLint& width, const GLint& height, const GLenum& e) 
{
	this->binding();
	this->width = width;
	this->height = height;
	glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, e, width, height);
	GLenum temp = GL_DEPTH_STENCIL_ATTACHMENT;
	switch (e)
	{
	case GL_DEPTH24_STENCIL8:
		temp = GL_DEPTH_STENCIL_ATTACHMENT;
	default:
		break;
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, temp, GL_RENDERBUFFER, this->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	this->check();
	this->unBinding();
}

void FrameBuffer::BindTexture(const Texture2D& texture, const GLenum& e)
{
	this->binding();
	texture.use();
	glFramebufferTexture2D(GL_FRAMEBUFFER, e, GL_TEXTURE_2D, texture.id, 0);
	texture.unUse();
	this->unBinding();
}

void FrameBuffer::check()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &this->fbo);
}
