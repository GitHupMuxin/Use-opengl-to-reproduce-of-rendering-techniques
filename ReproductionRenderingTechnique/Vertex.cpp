#include "Vertex.h"

Vertex::Vertex() { }

Vertex::Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord) : Position(position), Normal(normal), TexCoord(texCoord) { }
