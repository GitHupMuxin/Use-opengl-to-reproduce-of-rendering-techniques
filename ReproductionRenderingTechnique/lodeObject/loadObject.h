#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include  "objectMesh.h"

class LoadObject
{
private:
	void funtionMap(const std::string& Line, std::ifstream& istream);
	void loadVertex(const std::string& Line, std::ifstream& istream);
	void loadMaterial(const std::string& Line, std::ifstream& istream);
	void loadMesh(const std::string& Line, std::ifstream& istream);
public:
	std::string path;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	MaterialArray materialArray;
	std::vector<ObjectMesh> Mesh;
	LoadObject(const std::string& path);
	void init(const std::string& path);

};
