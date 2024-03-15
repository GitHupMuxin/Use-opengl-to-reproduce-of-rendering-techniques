#include "Scene.h"


Scene::Scene() { }

void Scene::Draw()
{
	for (auto& it : models)
	{
		it->modelShader.use();
		it->model.Draw(it->modelShader);
	}
}
