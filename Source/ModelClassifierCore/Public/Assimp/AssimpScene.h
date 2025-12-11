#pragma once
#include "assimp/cimport.h"
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "Render/Material.h"
#include <memory>
#include "assimp/Importer.hpp"
#include "assimp/IOSystem.hpp"
#include "Utilities/Collections.h"
#include "Render/Mesh.h"

namespace ModelClassifierCore
{
	struct MeshData;

	class MODELCLASSIFIERCORE_API FAssimpScene
	{
		Assimp::Importer AiImporter;
		std::unique_ptr<Assimp::IOSystem, std::function<void(Assimp::IOSystem*)>> IOHandler;
		
	public:
		FAssimpScene(){}
		~FAssimpScene()
		{
			UE_LOG(LogTemp, Warning, TEXT("Start Clear All AiScene Data"));

			// make sure importer not holding pointer we will delete
			if (IOHandler) {
				IOHandler.reset();
				AiImporter.SetIOHandler(nullptr);
			}
			
			AiImporter.FreeScene();
		}

		void SetIOHandler(Assimp::IOSystem* handler)
		{
			// ensure importer doesn't hold dangling pointer (just in case)
			if (IOHandler) {
				AiImporter.SetIOHandler(nullptr);
				IOHandler.reset();
			}
			IOHandler.reset(handler);

			GetImporter().SetIOHandler(handler);
		}

		const aiScene* GetAiScene()
		{
			return AiImporter.GetScene();
		}
		
		aiMesh* GetAiMesh(int i)
		{
			return GetAiScene()->mMeshes[i];
		}
		
		Assimp::Importer& GetImporter()
		{
			return AiImporter;
		}

		FString ModelName;
		FString FilePath;
		ImageSize Size = ImageSize(224, 224);
		std::vector<MeshData> Meshes;
		std::unordered_map<std::string, int> BoneMapping;
		std::vector<glm::mat4> BoneOffsets;
		glm::vec3 Center;
		MinMax<glm::vec3, glm::vec3> BBox;
		std::vector<Material> Materials;
	};
}
