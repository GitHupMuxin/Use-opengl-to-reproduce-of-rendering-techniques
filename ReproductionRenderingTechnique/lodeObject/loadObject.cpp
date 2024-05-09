#include "loadObject.h"
#include "objectMesh.h"

void LoadObject::funtionMap(const std::string& Line, std::ifstream& istream)
{
	if (Line.empty())
		return;
	if (memcmp(Line.c_str(), "mtllib", 6))
		this->loadMaterial(Line, istream);
	else if (Line[0] == 'o')
		this->loadVertex(Line, istream);
	else if (memcmp(Line.c_str(), "usemtl", 6))
		this->loadMesh(Line, istream);
}

void LoadObject::loadVertex(const std::string& Line, std::ifstream& fileStream)
{
	std::istringstream istream = std::istringstream(Line);
	std::string typeName = std::string(Line.begin() + 7, Line.end()), line = "";
	this->Mesh.emplace_back();
	this->Mesh.back().meshName = typeName;
	while (std::getline(fileStream, line))
	{
		istream = std::istringstream(line);
		std::string typeName = "";
		istream >> typeName;
		if (typeName == "v")
		{
			glm::vec3 position = glm::vec3(0.0f);
			istream >> position.x >> position.y >> position.z;
			this->positions.push_back(position);
		}
		else if (typeName == "vt")
		{
			glm::vec2 texCoord = glm::vec2(0.0f);
			istream >> texCoord.x >> texCoord.y;
			this->texCoords.push_back(texCoord);
		}
		else if (typeName == "vn")
		{
			glm::vec3 normal = glm::vec3(0.0f);
			istream >> normal.x >> normal.y >> normal.z;
			this->normals.push_back(normal);
		}
		else if (typeName == "usemtl")
		{
			std::string materialName = "";
			istream >> materialName;
			this->Mesh.back().materialIndex = this->materialArray.findMaterial(materialName);
			while (std::getline(fileStream, line))
			{
				if (line.empty())
					break;
				if (line[0] == 'f')
				{
					float triangle[9];
					sscanf_s(line.c_str(), "f %f/%f/%f %f/%f/%f %f//%f/%f",
						&triangle[0], &triangle[1], &triangle[2],
						&triangle[3], &triangle[4], &triangle[5],
						&triangle[6], &triangle[7], &triangle[8]);
					this->Mesh.back().triangle.emplace_back(Triangle(triangle));
				}
				else if (line[0] == 's')
					continue;
			}
			break;
		}
	}
	this->funtionMap(line, fileStream);
}

void LoadObject::loadMaterial(const std::string& Line, std::ifstream& fileStream)
{
	std::string typeVale = "";
	std::istringstream istream(Line);
	istream >> typeVale;
	int index = path.find_last_of('/');
	std::string materialPath = std::string(path.begin(), path.begin() + index + 1);
	istream >> typeVale;
	materialPath = materialPath + typeVale;
	this->materialArray.loadMaterial(materialPath);
}

void LoadObject::loadMesh(const std::string& Line, std::ifstream& istream)
{
	std::string material(Line.begin() + 7, Line.end());
}

LoadObject::LoadObject(const std::string& path)
{
	this->init(path);
}

void LoadObject::init(const std::string& path)
{
	this->path = path;
	std::string line = "";
	std::ifstream fileStream;
	fileStream.open(path);
	while (std::getline(fileStream, line))
		this->funtionMap(line, fileStream);

}


