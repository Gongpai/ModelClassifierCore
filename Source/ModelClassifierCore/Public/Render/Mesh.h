#pragma once

#include <string>
#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glad/glad.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <unordered_map>

namespace ModelClassifierCore
{
	struct MODELCLASSIFIERCORE_API Vertex {
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Normal = glm::vec3(0.0f);
	glm::vec4 Tangent = glm::vec4(0.0f);
	glm::vec2 TexCoords = glm::vec2(0.0f);
	glm::ivec4 BoneIDs = glm::ivec4(0,0,0,0); 
	glm::vec4   Weights= glm::vec4(0.0f);
	};

	struct MODELCLASSIFIERCORE_API MeshData {
		std::string name;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		int materialIndex = -1;

		GLuint VAO = 0;
		GLuint VBO = 0;
		GLuint EBO = 0;

		MeshData()
		{}
		~MeshData()
		{
			if (VAO) glDeleteVertexArrays(1, &VAO);
			if (VBO) glDeleteBuffers(1, &VBO);
			if (EBO) glDeleteBuffers(1, &EBO);
		}
		
		void InitializeMeshBuffers()
		{
			if (VAO != 0) return;

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.empty() ? nullptr : vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.empty() ? nullptr : indices.data(), GL_STATIC_DRAW);

			// location 0: Position (vec3)
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

			// location 1: Normal (vec3)
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

			// location 2: Tangent (vec4)
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

			// location 3: TexCoords (vec2)
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

			// location 4: BoneIDs (ivec4)
			glEnableVertexAttribArray(4);
			glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

			// location 5: Weights (vec4)
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));

			glBindVertexArray(0);
		}
	};
}