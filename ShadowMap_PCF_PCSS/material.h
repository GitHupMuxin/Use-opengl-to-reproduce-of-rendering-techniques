#pragma once
#include <GLAD/glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

struct Material
{
public:
	glm::vec3 Ka, Kd, Ks;
	std::vector<std::pair<std::string, std::string>> texutre;
	Material();
};

class MaterialArray
{
public:
	std::vector<std::pair<std::string, Material>> materialArray;
	int findMaterial(const std::string& name);
	void loadMaterial(const std::string& path);
};