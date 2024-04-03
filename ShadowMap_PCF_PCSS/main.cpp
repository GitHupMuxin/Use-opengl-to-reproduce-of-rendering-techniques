#include <GLAD/glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Tool.h"
#include "Scene.h"
#include "UniformBlock.h"
#include "FrameBuffer.h"
#include "Light.h"
#include "environmentBox.h"
#include "material.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048

glm::vec3 cameraPos(6.0f, 3.0f, 8.0f);
glm::vec3 cameraTraget(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
Camera camera(cameraPos, cameraTraget, cameraUp, 40, 2.5);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ShadowMap", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD." << std::endl;
		return -1;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	camera.init(window);

	Scene scene;

	UniformBlock cameraUniformBlock(2.0f * sizeof(glm::mat4x4) + sizeof(glm::vec4));

	FrameBuffer shadowMapBuffer;
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	Texture2D shadowMapTexture(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, GL_RGBA, GL_RGBA);
	shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapTexture.use();
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	shadowMapBuffer.BindTexture2D(shadowMapTexture);
	shadowMapBuffer.bufferStorage(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	Shader shadowMapShader("shadowMapVertexShader.glsl", "shadowMapFragmentShader.glsl");

	const GLint samples = 4;
	FrameBufferMSAA MSAA4X4FrameBuffer(samples);
	Texture2DMultisample MSAA4X4Texture(SCREEN_WIDTH, SCREEN_HEIGHT, samples, GL_RGBA);
	MSAA4X4Texture.Parameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
	MSAA4X4Texture.Parameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
	MSAA4X4FrameBuffer.BindTexture2D(MSAA4X4Texture);
	MSAA4X4FrameBuffer.bufferStorage(SCREEN_WIDTH, SCREEN_HEIGHT);

	FrameBuffer intermediateFBO;
	Texture2D intermediateTexture(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA);
	intermediateFBO.BindTexture2D(intermediateTexture);
	intermediateFBO.bufferStorage(SCREEN_WIDTH, SCREEN_HEIGHT);

	const GLuint attachmentsSize = 4;
	const GLenum attachments[attachmentsSize] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};
	FrameBuffer SSRTFrameBuffer;

	float zeroBorder[4] = { 0.0, 0.0, 0.0, 0.0 };
	Texture2D gNormalTexture(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gNormalTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gNormalTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gNormalTexture.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gNormalTexture.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gNormalTexture, 0, attachments[0]);

	Texture2D gKdBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gKdBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gKdBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gKdBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gKdBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gKdBuffer, 0, attachments[1]);

	Texture2D gPosBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gPosBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gPosBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gPosBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gPosBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gPosBuffer, 0, attachments[2]);

	const GLuint mipMapLevel = 7;
	Texture2D SSRTMipMapTexture(SCREEN_WIDTH, SCREEN_HEIGHT, GL_R16F, GL_RED, GL_FLOAT);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_BASE_LEVEL, 0);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MAX_LEVEL, mipMapLevel);
	SSRTFrameBuffer.BindTexture2D(SSRTMipMapTexture, 0, attachments[3]);
	SSRTFrameBuffer.bufferStorage(SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH32F_STENCIL8);

	Shader SSRTGBufferShder("SSRTMipmapVertexShader.glsl", "SSRTMipmapFragmentShader.glsl");

	FrameBuffer SSRTMipMapBuffer;
	Shader SSRTGenLevelMapShader("SSRTGenLevelMapVertexShader.glsl", "SSRTGenLevelMapFragmentShader.glsl"); 

	Shader SSRTShader("SSRTVertexShader.glsl", "SSRTFragmentShader.glsl");
	Shader enterMaxDepthShader("enterMaxDepthVertexShader.glsl", "enterMaxDepthFragmentShader.glsl");
	Shader textureShader("textureVertexShader.glsl", "textureFragmentShader.glsl");

	ScreenRenderModel screenRenderModel;

	Shader bilingPhongShader("blinnPhongVertexShader.glsl", "blinnPhongFragmentShader.glsl");

	glm::mat4x4 model;

	std::vector<std::string> faces = {
		"Prefab/cubemap/Indoor/posx.jpg",
		"Prefab/cubemap/Indoor/negx.jpg",
		"Prefab/cubemap/Indoor/posy.jpg",
		"Prefab/cubemap/Indoor/negy.jpg",
		"Prefab/cubemap/Indoor/posz.jpg",
		"Prefab/cubemap/Indoor/negz.jpg"
	};

	EnvironmentBox enviromentBox;

	enviromentBox.bindTexture(faces);

	SHEBox indoor("Prefab/cubemap/Indoor/light.txt");

	UniformBlock IndoorUniformSHBlock(3 * sizeof(glm::mat4x4));
	std::vector<GLfloat> coeffs = indoor.getLightCoeffs();
	std::vector<glm::mat4x4> coeffsMat(3);
	for (int k = 0; k < 3; k++)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				coeffsMat[k][i][j] = coeffs[i * 9 + j * 3 + k];
			}
		}
	}

	IndoorUniformSHBlock.insert(coeffsMat);

	MaterialArray mr;
	mr.loadMaterial("Prefab/mary/Marry.mtl");

	SceneSHModel SHMary;
	SHMary.initModel("Prefab/mary/Marry.obj", "Prefab/cubemap/Indoor/transport.txt");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	SHMary.modelMat = model;
	SHMary.modelShader.init("SHMaryVertexShader.glsl", "SHMaryFragmentShader.glsl");
	scene.models.push_back(&SHMary);

	SceneSHModel SHFloor;
	SHFloor.initModel("Prefab/floor/floor.obj", "Prefab/cubemap/IndoorFloor/transport.txt");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	SHFloor.modelMat = model;
	SHFloor.modelShader.init("SHFloorVertexShader.glsl", "SHFloorFragmentShader.glsl");
	scene.models.push_back(&SHFloor);

	SceneModel mary;
	mary.getModel().init("Prefab/mary/Marry.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	mary.modelMat = model;
	mary.modelShader.init("maryVertexShader.glsl", "maryFragmentShader.glsl");
	scene.models.push_back(&mary);

	SceneModel floor;
	floor.getModel().init("Prefab/floor/floor.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	floor.modelMat = model;
	floor.modelShader.init("floorVertexShader.glsl", "floorFragmentShader.glsl");
	scene.models.push_back(&floor);

	Light pointLight;
	pointLight.lightIntansity = glm::vec3(75.0f);
	pointLight.position = glm::vec3(0.2f, 2.0f, 2.0f) * glm::vec3(4.0f);
	pointLight.view = glm::lookAt(pointLight.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	pointLight.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);

	PointLightRenderModel pointLightModel;
	pointLightModel.model = glm::translate(pointLightModel.model, pointLight.position);
	pointLightModel.model = glm::scale(pointLightModel.model, glm::vec3(0.1));


	UniformBlock pointLightBlock(2.0f * sizeof(glm::mat4x4) + 2 * sizeof(glm::vec4));
	pointLightBlock.insert(pointLight.lightIntansity);
	pointLightBlock.insert(pointLight.position);
	pointLightBlock.insert(pointLight.view);
	pointLightBlock.insert(pointLight.projection);

	pointLightBlock.bindingUniformBlock(shadowMapShader, "LightScene");
	pointLightBlock.bindingUniformBlock(mary.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(floor.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(bilingPhongShader, "LightScene");
	pointLightBlock.bindingUniformBlock(SHMary.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(SHFloor.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(SSRTShader, "LightScene");

	cameraUniformBlock.bindingUniformBlock(mary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(floor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(bilingPhongShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(pointLightModel.shader, "Scene");
	cameraUniformBlock.bindingUniformBlock(enviromentBox.shader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SHMary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SHFloor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SSRTGBufferShder, "Scene");
	cameraUniformBlock.bindingUniformBlock(SSRTShader, "Scene");

	IndoorUniformSHBlock.bindingUniformBlock(SHMary.modelShader, "LightSH");
	IndoorUniformSHBlock.bindingUniformBlock(SHFloor.modelShader, "LightSH");

	while (!glfwWindowShouldClose(window))
	{
		glm::mat4x4 view = camera.getViewMat4();
		glm::mat4x4 projection = glm::perspective(glm::radians(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

		cameraUniformBlock.insert(camera.position, 0);
		cameraUniformBlock.insert(view, sizeof(glm::vec4));
		cameraUniformBlock.insert(projection, sizeof(glm::vec4) + sizeof(glm::mat4x4));

		//shadow map render
		shadowMapBuffer.use();
		shadowMapShader.use();
		shadowMapShader.setMat4("model", SHFloor.modelMat);
		SHFloor.getModel().Draw(shadowMapShader);

		shadowMapShader.use();
		shadowMapShader.setMat4("model", mary.modelMat);
		mary.getModel().Draw(shadowMapShader);

		shadowMapShader.use();
		shadowMapShader.setMat4("model", SHMary.modelMat);
		SHMary.getModel().Draw(shadowMapShader);

		//ssrt gbuffer render
		SSRTFrameBuffer.use();
		glDrawBuffers(attachmentsSize, attachments);
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", SHFloor.modelMat);
		SHFloor.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", mary.modelMat);
		mary.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", SHMary.modelMat);
		SHMary.getModel().Draw(SSRTGBufferShder);
		
		//glStencilMask(0x00); 会导致glClear(GL_STENCIL_BUFFER_BIT)清除不了
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		enterMaxDepthShader.use();
		screenRenderModel.Draw(enterMaxDepthShader);
		glDisable(GL_STENCIL_TEST);

		//ssrtGenLevelOfMipmap
		SSRTMipMapBuffer.use();
		SSRTGenLevelMapShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SSRTMipMapTexture.id);
		SSRTGenLevelMapShader.setInt("MipMap", 0);
		SSRTMipMapTexture.generateMipmap();
		for (GLint i = 1; i < mipMapLevel; i++)
		{
			GLuint currentWidth = SCREEN_WIDTH >> i, currentHeight = SCREEN_HEIGHT >> i;
			currentWidth = currentWidth > 0 ? currentWidth : 1;
			currentHeight = currentHeight > 0 ? currentHeight : 1;
			SSRTGenLevelMapShader.setVec2("viewPort", glm::vec2(currentWidth, currentHeight));
			SSRTGenLevelMapShader.setInt("lastLevel", i - 1);
			glViewport(0, 0, currentWidth, currentHeight);
			screenRenderModel.Draw(SSRTGenLevelMapShader);
		}

		//main render
		MSAA4X4FrameBuffer.use();
		mary.modelShader.use();
		glActiveTexture(GL_TEXTURE1);
		mary.modelShader.setInt("shadowMap", 1);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		mary.modelShader.setMat4("model", mary.modelMat);
		mary.getModel().Draw(mary.modelShader);

		SHMary.modelShader.use();
		glActiveTexture(GL_TEXTURE1);
		mary.modelShader.setInt("shadowMap", 1);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		SHMary.modelShader.setMat4("model", SHMary.modelMat);
		SHMary.getModel().Draw(SHMary.modelShader);

		SHFloor.modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		SHFloor.modelShader.setInt("shadowMap", shadowMapTexture.id);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		SHFloor.modelShader.setMat4("model", SHFloor.modelMat);
		SHFloor.getModel().Draw(SHFloor.modelShader);

		pointLightModel.Draw();
		enviromentBox.Draw();

		//scene.Draw();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAA4X4FrameBuffer.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO.fbo); //intermediateTexture
		glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//screenRenderModel.Draw(SSRTMipMapTexture, 0, textureShader);

		SSRTShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SSRTMipMapTexture.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTexture.id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gKdBuffer.id);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gPosBuffer.id);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, intermediateTexture.id);
		SSRTShader.setInt("MipMap", 0);
		SSRTShader.setInt("gNormalBuffer", 1);
		SSRTShader.setInt("gKdBuffer", 2);
		SSRTShader.setInt("gPosBuffer", 3);
		SSRTShader.setInt("gColorBuffer", 4);
		SSRTShader.setVec2("SCREEN_SIZE", glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT));
		SSRTShader.setInt("maxLevel", mipMapLevel);
		screenRenderModel.Draw(SSRTShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}