#include "Scene.h"


Scene::Scene() { }

void Scene::Draw()
{
	for (auto& it : models)
	{
		it->modelShader.use();
		it->modelShader.setMat4("model", it->modelMat);
		it->model.Draw(it->modelShader);
	}
}
