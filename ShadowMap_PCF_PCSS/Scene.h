#pragma once
#include <vector>
#include "Camera.h"
#include "Model.h"
#include "Light.h"

struct SceneModel
{
	Model model;
	glm::mat4x4 modelMat;
	Shader modelShader;
};

class Scene
{
public:
	std::vector<SceneModel*> models;
	std::vector<Light> lights;
	Shader SceneShader;
	Scene();
	void Draw();
};