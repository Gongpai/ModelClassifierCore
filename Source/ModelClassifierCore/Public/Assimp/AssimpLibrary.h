#pragma once
#include "AssimpScene.h"
#include "FBX/InMemoryFBXStream.h"
#include "glm/fwd.hpp"
#include <glm/mat4x4.hpp>
#include "glm/geometric.hpp"
#include "glm/vec3.hpp"
#include <algorithm>
#include <cfloat>
#include <limits>
#include <memory>
#include "Utilities/Collections.h"
#include "Render/Mesh.h"

namespace ModelClassifierCore
{
	struct Vertex;
	struct MeshData;
	
	class MODELCLASSIFIERCORE_API FAssimpLibrary
	{
		static glm::vec2 aiVec2ToGlm(const aiVector3D& v);
		static glm::vec3 aiVec3ToGlm(const aiVector3D& v);
		static glm::mat4 AiMat4ToGlm(const aiMatrix4x4& from);
		static void AddBoneData(Vertex &vertex, int boneID, float weight);
		static MeshData ProcessMesh(aiMesh* mesh, const glm::mat4& transform, TSharedPtr<FAssimpScene>& AiScene, MinMax<glm::vec3, glm::vec3>& AiMinMax, const std::string& nodeName = "");
		static void ProcessNode(TSharedPtr<FAssimpScene>& AiScene, const aiScene* scene, MinMax<glm::vec3, glm::vec3>& AiMinMax);
		static void ProcessNode(TSharedPtr<FAssimpScene>& AiScene, aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, MinMax<glm::vec3, glm::vec3>& AiMinMax);
		static void UpdateMinMax(const Vertex& vert, MinMax<glm::vec3, glm::vec3>& AiMinMax);
		
	public:
		static bool LoadModelAssetFromMemory(TSharedPtr<FInMemoryFBXStream> MObject, TSharedPtr<FAssimpScene>& AiScene);
		static bool LoadModelAssetFromFile(const FString& FilePath, TSharedPtr<FAssimpScene>& AiScene);
		static bool ConvertAssimpToMesh(TSharedPtr<FAssimpScene>& AiScene);
	};
}
