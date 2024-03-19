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

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
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
	Texture2D shadowMapTexture(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, GL_RGBA);
	shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	shadowMapTexture.use();
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	shadowMapBuffer.BindTexture(shadowMapTexture);
	shadowMapBuffer.bufferStorage(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	Shader shadowMapShader("shadowMapVertexShader.glsl", "shadowMapFragmentShader.glsl");

	TextureRenderModel textureRenderModel;

	Shader bilingPhongShader("bilingPhongVertexShader.glsl", "bilingPhongFragmentShader.glsl");
	Shader textureShader("textureVertexShader.glsl", "textureFragmentShader.glsl");

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

	SceneModel mary;
	mary.model.init("Prefab/mary/Marry.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	mary.modelMat = model;
	mary.modelShader.init("maryVertexShader.glsl", "maryFragmentShader.glsl");
	scene.models.push_back(&mary);

	SceneModel floor;
	floor.model.init("Prefab/floor/floor.obj");
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	floor.modelMat = model;
	floor.modelShader.init("floorVertexShader.glsl", "floorFragmentShader.glsl");
	scene.models.push_back(&floor);
	 
	Light pointLight;
	pointLight.lightIntansity = glm::vec3(25.0f);
	pointLight.position = glm::vec3(0.2f, 1.0f, 1.0f) * glm::vec3(4.0f);
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
	cameraUniformBlock.bindingUniformBlock(mary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(floor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(bilingPhongShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(pointLightModel.shader, "Scene");
	cameraUniformBlock.bindingUniformBlock(enviromentBox.shader, "Scene");

	while (!glfwWindowShouldClose(window))
	{
		glm::mat4x4 view = camera.getViewMat4();
		glm::mat4x4 projection = glm::perspective(glm::radians(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

		cameraUniformBlock.insert(camera.position, 0);
		cameraUniformBlock.insert(view, sizeof(glm::vec4));
		cameraUniformBlock.insert(projection, sizeof(glm::vec4) + sizeof(glm::mat4x4));

		shadowMapBuffer.use();
		shadowMapShader.use();
		shadowMapShader.setMat4("model", floor.modelMat);
		floor.model.Draw(shadowMapShader);

		shadowMapShader.use();
		shadowMapShader.setMat4("model", mary.modelMat);
		mary.model.Draw(shadowMapShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		//textureRenderModel.Draw(shadowMapTexture);
		
		floor.modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		floor.modelShader.setInt("shadowMap", shadowMapTexture.id);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		floor.modelShader.setMat4("model", floor.modelMat);
		floor.model.Draw(floor.modelShader);

		mary.modelShader.use();
		glActiveTexture(GL_TEXTURE1);
		mary.modelShader.setInt("shadowMap", shadowMapTexture.id);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
		mary.modelShader.setMat4("model", mary.modelMat);
		mary.model.Draw(mary.modelShader);

		pointLightModel.Draw();

		enviromentBox.Draw();

		//scene.Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}