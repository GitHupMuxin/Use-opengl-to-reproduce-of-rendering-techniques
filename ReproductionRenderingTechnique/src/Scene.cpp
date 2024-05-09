#include "../head/Scene.h"


Scene::Scene() { }

void Scene::Draw()
{
	for (auto& it : models)
	{
		it->modelShader.use();
		it->modelShader.setMat4("model", it->modelMat);
		it->getModel().Draw(it->modelShader);
	}
}

void SceneModel::initModel(const std::string& path)
{
	this->model.init(path.c_str());
}

Model& SceneModel::getModel()
{
	return this->model;
}

void SceneSHModel::initModel(const std::string& path, const std::string& fileName)
{
	this->model.init(path.c_str(), fileName);
}

Model& SceneSHModel::getModel()
{
	return this->model;
}
