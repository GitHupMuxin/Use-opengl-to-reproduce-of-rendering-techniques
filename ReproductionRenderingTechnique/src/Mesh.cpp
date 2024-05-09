#include "../head/Mesh.h"


Mesh::Mesh() { }

Mesh::Mesh(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka, glm::vec3 Kd, glm::vec3 Ks) : verteices(vertex), indices(index), textures(texture), ka(Ka), kd(Kd), ks(Ks)
{
	this->textureCount = textures.size();
	setUpMesh();
}

void Mesh::initData(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka, glm::vec3 Kd, glm::vec3 Ks)
{
	this->verteices = vertex;
	this->indices = index;
	this->textures = texture;
	this->ka = Ka;
	this->kd = Kd;
	this->ks = Ks;
}

void Mesh::bindingSH(const std::vector<GLfloat>& sh, GLuint SHOrder, GLuint offset)
{
	std::cout << "Mesh don't have SH, please use MeshSH." << std::endl;
}

void Mesh::setUpMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, this->verteices.size() * sizeof(Vertex), &(this->verteices[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &(this->indices[0]), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

	glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader)
{
	GLuint diffuseNr = 1, specularNr = 1, reflectionNr = 1;
	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number, name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		else if (name == "texture_reflection")
			number = std::to_string(reflectionNr++);
		shader.setInt((name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);
	shader.setBool("haveDiffuseTexture", !textures.empty());
	shader.setVec3("ka", this->ka);
	shader.setVec3("kd", this->kd);
	shader.setVec3("ks", this->ks);
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::DrawInstance(const Shader& shader, const GLuint& amount)
{
	GLuint diffuseNr = 1, specularNr = 1, reflectionNr = 1;
	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number, name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		else if (name == "texture_reflection")
			number = std::to_string(reflectionNr++);
		shader.setInt((name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);
	shader.setBool("haveDiffuseTexture", !textures.empty());
	shader.setVec3("ka", this->ka);
	shader.setVec3("kd", this->kd);
	shader.setVec3("ks", this->ks);
	glBindVertexArray(this->VAO);
	glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount);
	glBindVertexArray(0);
}

void MeshWithSphericalHarmonics::setUpMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);	
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	GLuint verteicesSize = this->verteices.size() * sizeof(Vertex), shSize = this->SH.size() * sizeof(GLfloat), indexSize = this->indices.size() * sizeof(GLuint);

	glBufferData(GL_ARRAY_BUFFER, verteicesSize + shSize, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, this->verteices.size() * sizeof(Vertex), &(this->verteices[0]));
	glBufferSubData(GL_ARRAY_BUFFER, verteicesSize, shSize, &(this->SH[0]));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, &(this->indices[0]), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

	GLuint pointerIndex = 3;
	GLuint coeffsCount = (SHOrder + 1) * (SHOrder + 1);
	for (GLint i = 0; i <= this->SHOrder; i++)
	{
		glEnableVertexAttribArray(pointerIndex);
		glVertexAttribPointer(pointerIndex, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * coeffsCount, (void*)(verteicesSize + 3 * i * sizeof(GLfloat)));
		pointerIndex++;
	}
	glBindVertexArray(0);
}

void MeshWithSphericalHarmonics::bindingSH(const std::vector<GLfloat>& sh, GLuint SHSize, GLuint offset)
{
	std::vector<GLfloat>::const_iterator first = sh.begin() + offset;
	std::vector<GLfloat>::const_iterator seconrd = sh.begin() + offset + SHSize * this->verteices.size();
	this->SH.assign(first, seconrd);
	this->SHOrder = sqrt(SHSize) - 1;
	this->setUpMesh();
}

MeshWithSphericalHarmonics::MeshWithSphericalHarmonics(const std::vector<Vertex>& vertex, const std::vector<GLuint>& index, const std::vector<ModelTexture>& texture, glm::vec3 Ka, glm::vec3 Kd, glm::vec3 Ks)
{
	this->initData(vertex, index, texture, Ka, Kd, Ks);
}

