#pragma once
#include <vector>
#include "Camera.h"
#include "Model.h"
#include "Light.h"

class baseSceneModel
{
public:
	glm::mat4x4 modelMat;
	Shader modelShader;
	virtual Model& getModel() = 0;
};



class SceneModel : public baseSceneModel
{
private:
	Model model;
public:
	void initModel(const std::string& path);
	virtual Model& getModel();
};

class SceneSHModel : public baseSceneModel
{
private:
	SHModel model;
public:
	void initModel(const std::string& path, const std::string& fileName);
	virtual Model& getModel();
};

class Scene
{
public:
	std::vector<baseSceneModel*> models;
	std::vector<Light> lights;
	Shader SceneShader;
	Scene();
	void Draw();
};