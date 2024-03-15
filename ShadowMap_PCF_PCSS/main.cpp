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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	camera.init(window);


	Scene scene;

	UniformBlock cameraUniformBlock(2.0f * sizeof(glm::mat4x4) + sizeof(glm::vec4));
	cameraUniformBlock.insert(camera.position);
	cameraUniformBlock.insert(camera.getViewMat4());
	glm::mat4x4 cameraProjection = glm::perspective(glm::radians(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	cameraUniformBlock.insert(cameraProjection);

	//FrameBuffer shadowMapBuffer;
	//Texture2D shadowMapTexture(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, GL_RGBA);
	//shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
	//shadowMapTexture.Parameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
	//shadowMapBuffer.BindTexture(shadowMapTexture);
	//shadowMapBuffer.bufferStorage(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	//Shader shadowMapShader("shadowMapVertexShader.glsl", "shadowMapFragmentShader.glsl");

	Shader bilingPhongShader("bilingPhongVertexShader.glsl", "bilingPhongFragmentShader.glsl");
	Shader textureShader("textureVertexShader.glsl", "textureFragmentShader.glsl");

	SceneModel mary;
	mary.model.init("Prefab/mary/Marry.obj");
	mary.modelMat = glm::mat4x4(1.0f);
	mary.modelMat = glm::translate(mary.modelMat, glm::vec3(0.0f, -2.0f, 0.0f));
	mary.modelShader.init("maryVertexShader.glsl", "maryFragmentShader.glsl");
	scene.models.push_back(&mary);

	SceneModel floor;
	floor.model.init("Prefab/floor/floor.obj");
	floor.modelMat = glm::mat4x4(1.0f);
	floor.modelMat = glm::translate(floor.modelMat, glm::vec3(0.0f, -1.0f, 0.0f));
	floor.modelMat = glm::scale(floor.modelMat, glm::vec3(0.5f));
	floor.modelShader.init("floorVertexShader.glsl", "floorFragmentShader.glsl");
	scene.models.push_back(&floor);
	 
	Light pointLight;
	pointLight.model = glm::mat4x4(1.0f);
	pointLight.lightIntansity = glm::vec3(25.0f);
	pointLight.position = glm::vec3(0.0f, 1.0f, 1.0f) * glm::vec3(4.0f);
	pointLight.view = glm::lookAt(pointLight.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	pointLight.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);

	UniformBlock pointLightBlock(2.0f * sizeof(glm::mat4x4) + 2 * sizeof(glm::vec4));

	pointLightBlock.insert(pointLight.lightIntansity);
	pointLightBlock.insert(pointLight.position);
	pointLightBlock.insert(pointLight.view);
	pointLightBlock.insert(pointLight.projection);

	
	pointLightBlock.bindingUniformBlock(mary.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(floor.modelShader, "LightScene");
	pointLightBlock.bindingUniformBlock(bilingPhongShader, "LightScene");
	cameraUniformBlock.bindingUniformBlock(mary.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(floor.modelShader, "Scene");
	cameraUniformBlock.bindingUniformBlock(bilingPhongShader, "Scene");

	glm::mat4 projection = glm::mat4(1.0f);

	glm::mat4x4 model;
	floor.modelShader.use();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	floor.modelShader.setMat4("model", model);

	mary.modelShader.use();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	mary.modelShader.setMat4("model", model);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4x4 view = camera.getViewMat4();
		projection = glm::perspective(glm::radians(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		cameraUniformBlock.insert(camera.position, 0);
		cameraUniformBlock.insert(view, sizeof(glm::vec4));
		cameraUniformBlock.insert(projection, sizeof(glm::vec4) + sizeof(glm::mat4x4));

		//shadowMapBuffer.use();
		//shadowMapShader.use();
		//shadowMapShader.setMat4("model", floor.modelMat);
		//floor.model.Draw(shadowMapShader);

		//shadowMapShader.use();
		//shadowMapShader.setMat4("model", mary.modelMat);
		//mary.model.Draw(shadowMapShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		scene.Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}