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
#include "stb_image.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048

#define GBUFFER_WIDTH 2048
#define GBUFFER_HEIGHT 2048

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 512

#define NEAR_PLANE 0.1
#define FAR_PLANE 1000

glm::vec3 cameraPos(8.0f, 4.0f, 12.0f);
glm::vec3 cameraTraget(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
Camera camera(cameraPos, cameraTraget, cameraUp, 40, 2.5);

void pbrPrtTexture(const GLuint& IBL, GLuint* irradianceMap, GLuint* prefilterMap);
void generateHdrTexture(const std::string& path, GLuint* IBL);
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);
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

	UniformBlock cameraUniformBlock(2.0f * sizeof(glm::mat4x4) + sizeof(glm::vec4) + sizeof(float) * 2.0f);

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

	const GLuint attachmentsSize = 6;
	const GLenum attachments[attachmentsSize] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5
	};
	FrameBuffer SSRTFrameBuffer;

	float zeroBorder[4] = { 0.0, 0.0, 0.0, 0.0 };
	float oneBorder[4] = { 1.0, 1.0, 1.0, 1.0 };
	Texture2D gNormalTexture(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gNormalTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gNormalTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gNormalTexture.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gNormalTexture.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gNormalTexture, 0, attachments[0]);

	Texture2D gKdBuffer(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gKdBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gKdBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gKdBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gKdBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gKdBuffer, 0, attachments[1]);

	Texture2D gPosBuffer(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gPosBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gPosBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gPosBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gPosBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gPosBuffer, 0, attachments[2]);

	Texture2D gViewPosBuffer(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	gViewPosBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gViewPosBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	gViewPosBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gViewPosBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gViewPosBuffer, 0, attachments[4]);

	Texture2D gViewDepthBuffer(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT);
	gViewDepthBuffer.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gViewDepthBuffer.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, oneBorder);
	gViewDepthBuffer.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gViewDepthBuffer.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTFrameBuffer.BindTexture2D(gViewDepthBuffer, 0, attachments[5]);

	const GLuint mipMapLevel = 7;
	Texture2D SSRTMipMapTexture(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, oneBorder);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_BASE_LEVEL, 0);
	SSRTMipMapTexture.Parameteri(GL_TEXTURE_MAX_LEVEL, mipMapLevel);
	SSRTFrameBuffer.BindTexture2D(SSRTMipMapTexture, 0, attachments[3]);
	SSRTFrameBuffer.bufferStorage(GBUFFER_WIDTH, GBUFFER_HEIGHT, GL_DEPTH32F_STENCIL8);

	FrameBuffer prtTextureFrameBuffer;
	Texture2D LUTTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RG16F, GL_RG, GL_FLOAT);
	LUTTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	LUTTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	LUTTexture.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	LUTTexture.Parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	prtTextureFrameBuffer.BindTexture2D(LUTTexture, 0, GL_COLOR_ATTACHMENT0);
	prtTextureFrameBuffer.bufferStorage(TEXTURE_WIDTH, TEXTURE_WIDTH, GL_DEPTH24_STENCIL8);

	Shader LUTShader("LUTVertexShader.glsl", "LUTFragmentShader.glsl");


	Shader SSRTGBufferShder("SSRTMipmapVertexShader.glsl", "SSRTMipmapFragmentShader.glsl");

	FrameBuffer SSRTMipMapBuffer;
	Shader SSRTGenLevelMapShader("SSRTGenLevelMapVertexShader.glsl", "SSRTGenLevelMapFragmentShader.glsl"); 

	Shader SSRTShader("SSRTVertexShader.glsl", "SSRTFragmentShader.glsl");
	Shader enterMaxDepthShader("enterMaxDepthVertexShader.glsl", "enterMaxDepthFragmentShader.glsl");
	Shader textureShader("textureVertexShader.glsl", "textureFragmentShader.glsl");
	Shader skyboxShader("environmentBoxVertexShader.glsl", "environmentBoxFragmentShader.glsl");

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
	
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	EnvironmentBox enviromentBox;
	BOXModel boxModel;

	enviromentBox.bindTexture(faces);
	enviromentBox.generateMipmap();

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
	model = glm::translate(model, glm::vec3(3.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	SHMary.modelMat = model;
	SHMary.modelShader.init("SHMaryVertexShader.glsl", "SHMaryFragmentShader.glsl");
	scene.models.push_back(&SHMary);

	SceneSHModel SHFloor;
	SHFloor.initModel("Prefab/floor/floor.obj", "Prefab/cubemap/IndoorFloor/transport.txt");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, -8.0));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	SHFloor.modelMat = model;
	SHFloor.modelShader.init("SHFloorVertexShader.glsl", "SHFloorFragmentShader.glsl");
	scene.models.push_back(&SHFloor);

	SceneModel mary;
	mary.getModel().init("Prefab/mary/Marry.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0f));
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

	SceneModel ball;
	ball.getModel().init("Prefab/ball/TestObj.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	ball.modelMat = model;
	ball.modelShader.init("ballVertexShader.glsl", "ballFragmentShader.glsl");
	scene.models.push_back(&ball);

	const int ballCount = 10;
	const float leftSize = -8;
	const float ballPositionDir = (-leftSize * 2) / (ballCount - 1);
	const float uRoughness = 1.0 / (ballCount - 1);
	SceneModel pbrBall[ballCount];
	for (int i = 0; i < ballCount; i++)
	{
		pbrBall[i].getModel().init("Prefab/ball/TestObj.obj");
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(leftSize + i * ballPositionDir, -1.0f, -8.0f));
		model = glm::scale(model, glm::vec3(0.25f));
		pbrBall[i].modelMat = model;
		pbrBall[i].modelShader.init("pbrVertexShader.glsl", "pbrFragmentShader.glsl");
		pbrBall[i].modelShader.use();
		pbrBall[i].modelShader.setFloat("uRoughness", uRoughness * i);
		scene.models.push_back(pbrBall + i);
	}

	std::vector<std::string> E(3);
	E[0] = "Prefab/ball/GGX_E_LUT.png";
	E[1] = "Prefab/ball/GGX_Eavg_LUT.png";
	E[2] = "Prefab/ball/GGX_E_MC_LUT.png";
	Texture2D EmuIs(E[0].c_str());
	Texture2D Eavg(E[1].c_str());
	Texture2D EmuMc(E[2].c_str());
	SceneModel kullaContyBall[ballCount];
	for (int i = 0; i < ballCount; i++)
	{
		kullaContyBall[i].getModel().init("Prefab/ball/TestObj.obj");
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(leftSize + i * ballPositionDir, -1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(0.25f));
		kullaContyBall[i].modelMat = model;
		kullaContyBall[i].modelShader.init("kullaContyVertexShader.glsl", "kullaContyFragmentShader.glsl");
		kullaContyBall[i].modelShader.use();
		kullaContyBall[i].modelShader.setFloat("uRoughness", uRoughness * i);
		scene.models.push_back(kullaContyBall + i);
	}

	SceneModel cookTorranceBall[ballCount];
	for (int i = 0; i < ballCount; i++)
	{
		cookTorranceBall[i].getModel().init("Prefab/ball/TestObj.obj");
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(leftSize + i * ballPositionDir, -1.0f, -12.0f));
		model = glm::scale(model, glm::vec3(0.25f));
		cookTorranceBall[i].modelMat = model;
		cookTorranceBall[i].modelShader.init("cookTorranceVertexShader.glsl", "cookTorranceFragmentShader.glsl");
		cookTorranceBall[i].modelShader.use();
		cookTorranceBall[i].modelShader.setFloat("uRoughness", uRoughness* i);
		scene.models.push_back(cookTorranceBall + i);
	}

	SceneModel IBLCookTorranceBall[ballCount];
	for (int i = 0; i < ballCount; i++)
	{
		IBLCookTorranceBall[i].getModel().init("Prefab/ball/TestObj.obj");
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(leftSize + i * ballPositionDir, -1.0f, -16.0f));
		model = glm::scale(model, glm::vec3(0.25f));
		IBLCookTorranceBall[i].modelMat = model;
		IBLCookTorranceBall[i].modelShader.init("IBLCookTorranceVertexShader.glsl", "IBLCookTorranceFragmentShader.glsl");
		IBLCookTorranceBall[i].modelShader.use();
		IBLCookTorranceBall[i].modelShader.setFloat("uRoughness", uRoughness * i);
		scene.models.push_back(IBLCookTorranceBall + i);
	}

	SceneModel IBLKullaContyBall[ballCount];
	for (int i = 0; i < ballCount; i++)
	{
		IBLKullaContyBall[i].getModel().init("Prefab/ball/TestObj.obj");
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(leftSize + i * ballPositionDir, -1.0f, -20.0f));
		model = glm::scale(model, glm::vec3(0.25f));
		IBLKullaContyBall[i].modelMat = model;
		IBLKullaContyBall[i].modelShader.init("IBLKullaContyVertexShader.glsl", "IBLKullaContyFragmentShader.glsl");
		IBLKullaContyBall[i].modelShader.use();
		IBLKullaContyBall[i].modelShader.setFloat("uRoughness", uRoughness * i);
		scene.models.push_back(IBLKullaContyBall + i);
	}

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
	pointLightBlock.bindingUniformBlock(ball.modelShader, "LightScene");
	for (auto& it : pbrBall)
		pointLightBlock.bindingUniformBlock(it.modelShader, "LightScene");
	for (auto& it : kullaContyBall)
		pointLightBlock.bindingUniformBlock(it.modelShader, "LightScene");
	for (auto& it : cookTorranceBall)
		pointLightBlock.bindingUniformBlock(it.modelShader, "LightScene");
	for (auto& it : IBLCookTorranceBall)
		pointLightBlock.bindingUniformBlock(it.modelShader, "LightScene");
	for (auto& it : IBLKullaContyBall)
		pointLightBlock.bindingUniformBlock(it.modelShader, "LightScene");

	cameraUniformBlock.bindingUniformBlock(mary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(floor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(bilingPhongShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(pointLightModel.shader, "Scene");
	cameraUniformBlock.bindingUniformBlock(enviromentBox.shader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SHMary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SHFloor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(SSRTGBufferShder, "Scene");
	cameraUniformBlock.bindingUniformBlock(SSRTShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(ball.modelShader, "Scene");
	for (auto& it : pbrBall)
		cameraUniformBlock.bindingUniformBlock(it.modelShader, "Scene");
	for (auto& it : kullaContyBall)
		cameraUniformBlock.bindingUniformBlock(it.modelShader, "Scene");
	for (auto& it : cookTorranceBall)
		cameraUniformBlock.bindingUniformBlock(it.modelShader, "Scene");
	for (auto& it : IBLCookTorranceBall)
		cameraUniformBlock.bindingUniformBlock(it.modelShader, "Scene");
	for (auto& it : IBLKullaContyBall)
		cameraUniformBlock.bindingUniformBlock(it.modelShader, "Scene");

	IndoorUniformSHBlock.bindingUniformBlock(SHMary.modelShader, "LightSH");
	IndoorUniformSHBlock.bindingUniformBlock(SHFloor.modelShader, "LightSH");

	cameraUniformBlock.insert((float)NEAR_PLANE, sizeof(glm::vec4) + 2.0 * sizeof(glm::mat4x4));
	cameraUniformBlock.insert((float)FAR_PLANE, sizeof(glm::vec4) + 2.0 * sizeof(glm::mat4x4) + sizeof(GLfloat));

	GLuint hdrBox, rostockArchesBox;
	generateHdrTexture("Prefab/cubemap/studioGarden/studioGarden4k.hdr", &hdrBox);
	generateHdrTexture("Prefab/cubemap/rostockArches/rostockArches4k.hdr", &rostockArchesBox);

	Texture2D hdrT("Prefab/cubemap/studioGarden/studioGarden4k.hdr");

	GLuint irradianceMap, prefilterMap;
	pbrPrtTexture(enviromentBox.textureID, &irradianceMap, &prefilterMap);

	GLuint hdrIrradianceMap, hdrPrefilterMap;
	pbrPrtTexture(hdrBox, &hdrIrradianceMap, &hdrPrefilterMap);

	GLuint hdrArchesIrradianceMap, hdrArchesPrefilterMap;
	pbrPrtTexture(rostockArchesBox, &hdrArchesIrradianceMap, &hdrArchesPrefilterMap);



	prtTextureFrameBuffer.use();
	screenRenderModel.Draw(LUTShader);

	float level = 7;

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glm::mat4x4 view = camera.getViewMat4();
		glm::mat4x4 projection = glm::perspective(glm::radians(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, (float)NEAR_PLANE, (float)FAR_PLANE);

		cameraUniformBlock.insert(camera.position, 0);
		cameraUniformBlock.insert(view, sizeof(glm::vec4));
		cameraUniformBlock.insert(projection, sizeof(glm::vec4) + sizeof(glm::mat4x4));

		//shadow map pass
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

		shadowMapShader.use();
		shadowMapShader.setMat4("model", ball.modelMat);
		ball.getModel().Draw(shadowMapShader);

		shadowMapShader.use();
		for (auto& it : pbrBall)
		{
			shadowMapShader.setMat4("model", it.modelMat);
			it.getModel().Draw(shadowMapShader);
		}

		shadowMapShader.use();
		for (auto& it : kullaContyBall)
		{
			shadowMapShader.setMat4("model", it.modelMat);
			it.getModel().Draw(shadowMapShader);
		}

		shadowMapShader.use();
		for (auto& it : cookTorranceBall)
		{
			shadowMapShader.setMat4("model", it.modelMat);
			it.getModel().Draw(shadowMapShader);
		}

		shadowMapShader.use();
		for (auto& it : IBLCookTorranceBall)
		{
			shadowMapShader.setMat4("model", it.modelMat);
			it.getModel().Draw(shadowMapShader);
		}

		shadowMapShader.use();
		for (auto& it : IBLKullaContyBall)
		{
			shadowMapShader.setMat4("model", it.modelMat);
			it.getModel().Draw(shadowMapShader);
		}

		//ssrt gbuffer pass
		SSRTFrameBuffer.use();
		glDrawBuffers(attachmentsSize, attachments);
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", SHFloor.modelMat);
		SSRTGBufferShder.setBool("SSRTON", true);
		SHFloor.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", mary.modelMat);
		SSRTGBufferShder.setBool("SSRTON", false);
		mary.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", SHMary.modelMat);
		SSRTGBufferShder.setBool("SSRTON", false);
		SHMary.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		SSRTGBufferShder.setMat4("model", ball.modelMat);
		SSRTGBufferShder.setBool("SSRTON", true);
		ball.getModel().Draw(SSRTGBufferShder);

		SSRTGBufferShder.use();
		for (auto& it : pbrBall)
		{
			SSRTGBufferShder.setMat4("model", it.modelMat);
			SSRTGBufferShder.setBool("SSRTON", false);
			it.getModel().Draw(SSRTGBufferShder);
		}

		SSRTGBufferShder.use();
		for (auto& it : kullaContyBall)
		{
			SSRTGBufferShder.setMat4("model", it.modelMat);
			SSRTGBufferShder.setBool("SSRTON", false);
			it.getModel().Draw(SSRTGBufferShder);
		}

		SSRTGBufferShder.use();
		for (auto& it : cookTorranceBall)
		{
			SSRTGBufferShder.setMat4("model", it.modelMat);
			SSRTGBufferShder.setBool("SSRTON", false);
			it.getModel().Draw(SSRTGBufferShder);
		}

		SSRTGBufferShder.use();
		for (auto& it : IBLCookTorranceBall)
		{
			SSRTGBufferShder.setMat4("model", it.modelMat);
			SSRTGBufferShder.setBool("SSRTON", false);
			it.getModel().Draw(SSRTGBufferShder);
		}

		SSRTGBufferShder.use();
		for (auto& it : IBLKullaContyBall)
		{
			SSRTGBufferShder.setMat4("model", it.modelMat);
			SSRTGBufferShder.setBool("SSRTON", false);
			it.getModel().Draw(SSRTGBufferShder);
		}

		//glStencilMask(0x00); �ᵼ��glClear(GL_STENCIL_BUFFER_BIT)�������
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		enterMaxDepthShader.use();
		screenRenderModel.Draw(enterMaxDepthShader);
		glDisable(GL_STENCIL_TEST);

		//ssrtGenLevelOfMipmap pass
		SSRTMipMapBuffer.use();
		SSRTGenLevelMapShader.use();
		glActiveTexture(GL_TEXTURE0);
		SSRTMipMapTexture.generateMipmap();
		glBindTexture(GL_TEXTURE_2D, SSRTMipMapTexture.id);
		for (GLint i = 1; i <= mipMapLevel; i++)
		{
			GLuint currentWidth = GBUFFER_WIDTH >> i, currentHeight = GBUFFER_HEIGHT >> i;
			currentWidth = currentWidth > 0 ? currentWidth : 1;
			currentHeight = currentHeight > 0 ? currentHeight : 1;
			SSRTGenLevelMapShader.setVec2("viewPort", glm::vec2(currentWidth, currentHeight));
			SSRTGenLevelMapShader.setInt("lastLevel", i - 1);
			SSRTMipMapBuffer.bufferStorage(currentWidth, currentHeight, GL_DEPTH24_STENCIL8);
			SSRTMipMapBuffer.use();
			screenRenderModel.Draw(SSRTGenLevelMapShader);
		}

		//main pass ��ɫ
		MSAA4X4FrameBuffer.use();
		mary.modelShader.use();
		glActiveTexture(GL_TEXTURE1);
		mary.modelShader.setInt("shadowMap", 1);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		mary.modelShader.setMat4("model", mary.modelMat);
		mary.getModel().Draw(mary.modelShader);

		SHMary.modelShader.use();
		glActiveTexture(GL_TEXTURE1);
		SHMary.modelShader.setInt("shadowMap", 1);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		SHMary.modelShader.setMat4("model", SHMary.modelMat);
		SHMary.getModel().Draw(SHMary.modelShader);

		SHFloor.modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		SHFloor.modelShader.setInt("shadowMap", 0);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		SHFloor.modelShader.setMat4("model", SHFloor.modelMat);
		SHFloor.getModel().Draw(SHFloor.modelShader);

		ball.modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		ball.modelShader.setInt("shadowMap", 0);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		ball.modelShader.setMat4("model", ball.modelMat);
		ball.getModel().Draw(ball.modelShader);

		for (int i = 0; i < ballCount; i++)
		{
			pbrBall[i].modelShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentBox.textureID);
			pbrBall[i].modelShader.setInt("shadowMap", 0);
			pbrBall[i].modelShader.setInt("skyBox", 1);
			pbrBall[i].modelShader.setFloat("uTextureLevel", level);
			pbrBall[i].modelShader.setMat4("model", pbrBall[i].modelMat);
			pbrBall[i].getModel().Draw(pbrBall[i].modelShader);

			kullaContyBall[i].modelShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, EmuIs.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, Eavg.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, EmuMc.id);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentBox.textureID);
			kullaContyBall[i].modelShader.setInt("shadowMap", 0);
			kullaContyBall[i].modelShader.setInt("EmuIs", 1);
			kullaContyBall[i].modelShader.setInt("Eavg", 2);
			kullaContyBall[i].modelShader.setInt("EmuMc", 3);
			kullaContyBall[i].modelShader.setInt("skyBox", 4);
			kullaContyBall[i].modelShader.setFloat("uTextureLevel", level);
			kullaContyBall[i].modelShader.setMat4("model", kullaContyBall[i].modelMat);
			kullaContyBall[i].getModel().Draw(kullaContyBall[i].modelShader);

			cookTorranceBall[i].modelShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentBox.textureID);
			cookTorranceBall[i].modelShader.setInt("shadowMap", 0);
			cookTorranceBall[i].modelShader.setInt("skyBox", 1);
			cookTorranceBall[i].modelShader.setFloat("uTextureLevel", level);
			cookTorranceBall[i].modelShader.setMat4("model", cookTorranceBall[i].modelMat);
			cookTorranceBall[i].getModel().Draw(cookTorranceBall[i].modelShader);

			IBLCookTorranceBall[i].modelShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentBox.textureID);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, LUTTexture.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			IBLCookTorranceBall[i].modelShader.setInt("shadowMap", 0);
			IBLCookTorranceBall[i].modelShader.setInt("skyBox", 1);
			IBLCookTorranceBall[i].modelShader.setInt("LUTTexture", 2);
			IBLCookTorranceBall[i].modelShader.setInt("irradianceMap", 3);
			IBLCookTorranceBall[i].modelShader.setInt("prefilterMap", 4);
			IBLCookTorranceBall[i].modelShader.setMat4("model", IBLCookTorranceBall[i].modelMat);
			IBLCookTorranceBall[i].getModel().Draw(IBLCookTorranceBall[i].modelShader);

			IBLKullaContyBall[i].modelShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, EmuIs.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, Eavg.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, EmuMc.id);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, LUTTexture.id);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentBox.textureID);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			IBLKullaContyBall[i].modelShader.setInt("shadowMap", 0);
			IBLKullaContyBall[i].modelShader.setInt("EmuIs", 1);
			IBLKullaContyBall[i].modelShader.setInt("Eavg", 2);
			IBLKullaContyBall[i].modelShader.setInt("EmuMc", 3);
			IBLKullaContyBall[i].modelShader.setInt("LUTTexture", 4);
			IBLKullaContyBall[i].modelShader.setInt("skyBox", 5);
			IBLKullaContyBall[i].modelShader.setInt("irradianceMap", 6);
			IBLKullaContyBall[i].modelShader.setInt("prefilterMap", 7);
			IBLKullaContyBall[i].modelShader.setMat4("model", IBLKullaContyBall[i].modelMat);
			IBLKullaContyBall[i].getModel().Draw(IBLKullaContyBall[i].modelShader);
		}

		pointLightModel.Draw();
		enviromentBox.Draw();
		//boxModel.Draw(hdrBox, skyboxShader, 0);
		
		//scene.Draw();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, MSAA4X4FrameBuffer.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO.fbo); //intermediateTexture
		glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//screenRenderModel.Draw(SSRTMipMapTexture, 5, textureShader);

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
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, gViewPosBuffer.id);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, gViewDepthBuffer.id);
		SSRTShader.setInt("MipMap", 0);
		SSRTShader.setInt("gNormalBuffer", 1);
		SSRTShader.setInt("gKdBuffer", 2);
		SSRTShader.setInt("gPosBuffer", 3);
		SSRTShader.setInt("gColorBuffer", 4);
		SSRTShader.setInt("gViewPosBuffer", 5);
		SSRTShader.setInt("gViewDepthBuffer", 6);
		SSRTShader.setVec2("SCREEN_SIZE", glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT));
		SSRTShader.setInt("maxLevel", mipMapLevel);
		screenRenderModel.Draw(SSRTShader);

		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void pbrPrtTexture(const GLuint& IBL, GLuint* irradianceMap, GLuint* prefilterMap)
{
	BOXModel box;
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	Shader irradianceShader("IBLIrradianceVertexShader.glsl", "IBLIrradianceFragmentShader.glsl");
	irradianceShader.use();
	irradianceShader.setMat4("projection", captureProjection);

	const GLuint IRRADIANCE_WIDTH = 32, IRRADIANCE_HEIGHT = 32;
	FrameBuffer preComputeBuffer;
	preComputeBuffer.bufferStorage(IRRADIANCE_WIDTH, IRRADIANCE_HEIGHT, GL_DEPTH24_STENCIL8);

	GLuint& irMap = *irradianceMap;
	glGenTextures(1, &irMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irMap);
	for (GLint i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, IRRADIANCE_WIDTH, IRRADIANCE_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (GLuint i = 0; i < 6; i++)
	{
		irradianceShader.setMat4("view", captureViews[i]);
		preComputeBuffer.BindTexture(irMap, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COLOR_ATTACHMENT0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		preComputeBuffer.use();
		box.Draw(IBL, irradianceShader);
	}

	Shader prefilterShader("prefilterVertexShader.glsl", "prefilterFragmentShader.glsl");
	const GLuint FILTERWIDTH = 1024, FILTERHEIGHT = 1024;
	GLuint maxMipLevels = 11;
	GLuint& preMap = *prefilterMap;
	glGenTextures(1, &preMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, preMap);
	for (GLuint i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, FILTERWIDTH, FILTERHEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	prefilterShader.use();
	prefilterShader.setMat4("projection", captureProjection);

	for (GLuint mipLevel = 0; mipLevel < maxMipLevels; mipLevel++)
	{
		GLuint mipWidth = static_cast<GLuint>(FILTERWIDTH >> mipLevel);
		GLuint mipHeight = static_cast<GLuint>(FILTERHEIGHT >> mipLevel);
		preComputeBuffer.bufferStorage(mipWidth, mipHeight, GL_DEPTH24_STENCIL8);
		
		float roughness = float(mipLevel) / float(maxMipLevels - 1);
		prefilterShader.setFloat("uRoughness", roughness);
		for (GLuint i = 0; i < 6; i++)
		{
			prefilterShader.setMat4("view", captureViews[i]);
			preComputeBuffer.BindTexture(preMap, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipLevel, GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			preComputeBuffer.use();
			box.Draw(IBL, prefilterShader);
		}
	}
}

void generateHdrTexture(const std::string& path, GLuint* IBL)
{
	BOXModel box;
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	stbi_set_flip_vertically_on_load(true);
	GLuint& result = *IBL;
	GLuint hdrTexture;
	
	const GLuint& skyBoxWidth = 1024, skyBoxHeight = 1024;
	int width = 0, height = 0, nrComponents = 0;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);




		Shader hdrShader("hdrMapToSkyboxVertexShader.glsl", "hdrMapToSkyboxFragmentShader.glsl");
		FrameBuffer fbo;
		fbo.bufferStorage(skyBoxWidth, skyBoxHeight, GL_DEPTH24_STENCIL8);

		glGenTextures(1, &result);
		glBindTexture(GL_TEXTURE_CUBE_MAP, result);

		for (GLuint i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, skyBoxWidth, skyBoxHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		hdrShader.use();
		hdrShader.setMat4("projection", captureProjection);
		for (GLuint i = 0; i < 6; i++)
		{
			hdrShader.setMat4("view", captureViews[i]);
			fbo.BindTexture(result, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COLOR_ATTACHMENT0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			hdrShader.setInt("skyBox", 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			fbo.use();
			box.render();
		}
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}
}