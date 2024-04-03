#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "material.h"

Material::Material() : Ka(glm::vec3(0.0f)), Kd(glm::vec3(0.0f)), Ks(glm::vec3(0.0f)) { }

int MaterialArray::findMaterial(const std::string& name)
{
	for (int i = 0; i < this->materialArray.size(); i++)
		if (this->materialArray[i].first == name)
			return i;
	return -1;
}

void MaterialArray::loadMaterial(const std::string& path)
{
	std::ifstream fileStream;
	fileStream.open(path);
	std::string line;
	try
	{

		while (std::getline(fileStream, line))
		{
			if (!line.size() || line[0] == '#')
				continue;
			std::string MaterialName = "";
			char buffer[64] = "\0";
			if (!memcmp(line.c_str(), "newmtl", 6))
			{
				sscanf_s(line.c_str(), "newmtl %s", buffer, GLuint(line.size()));
				this->materialArray.push_back({});
				this->materialArray.back().first = buffer;
				Material& m = this->materialArray.back().second;
				buffer[0] = '\0';
				while (std::getline(fileStream, line))
				{
					if (!line.size())
						break;
					GLfloat x = 0, y = 0, z = 0;
					std::istringstream istream(line);
					std::string typeName = "";
					istream >> typeName;
					if (typeName == "Ka")
						istream >> m.Ka.x >> m.Ka.y >> m.Ka.z;
					else if (typeName == "Kd")
						istream >> m.Kd.x >> m.Kd.y >> m.Kd.z;
					else if (typeName == "Ks")
						istream >> m.Ks.x >> m.Ks.y >> m.Ks.z;
					else if (typeName == "map_Kd")
					{
						std::string path = "";
						istream >> path;
						m.texutre.push_back(std::make_pair(typeName, path));
					}
				}
			}
		}
	}
	catch (...)
	{
		std::cout << "Material file load error." << std::endl;
	}
}
