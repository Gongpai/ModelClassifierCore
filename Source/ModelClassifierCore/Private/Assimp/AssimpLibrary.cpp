#include "Assimp/AssimpLibrary.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include "assimp/mesh.h"

namespace ModelClassifierCore
{
	class FbxMemoryStream : public Assimp::IOStream
	{
		TArray<uint8> Buffer;
		size_t Cursor = 0;
	
	public:
		FbxMemoryStream(const TArray<uint8>& InData)
			: Buffer(InData), Cursor(0) {}

		size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override
		{
			size_t Total = pSize * pCount;
			size_t CopySize = FMath::Min(Total, Buffer.Num() - Cursor);
			FMemory::Memcpy(pvBuffer, Buffer.GetData() + Cursor, CopySize);
			Cursor += CopySize;
			return CopySize / pSize;
		}

		size_t Write(const void*, size_t, size_t) override { return 0; }
		aiReturn Seek(size_t Offset, aiOrigin Origin) override
		{
			switch (Origin)
			{
			case aiOrigin_SET: Cursor = Offset; break;
			case aiOrigin_CUR: Cursor += Offset; break;
			case aiOrigin_END: Cursor = Buffer.Num() + Offset; break;
			}
			return aiReturn_SUCCESS;
		}

		size_t Tell() const override { return Cursor; }
		size_t FileSize() const override { return Buffer.Num(); }
		void Flush() override {}
	};

	class FbxMemoryIOSystem : public Assimp::IOSystem
	{
		TArray<uint8> Buffer;

	public:
		FbxMemoryIOSystem(const TArray<uint8>& InData)
			: Buffer(InData) {}

		bool Exists(const char*) const override { return true; }
		char getOsSeparator() const override { return '/'; }

		Assimp::IOStream* Open(const char*, const char*) override
		{
			return new FbxMemoryStream(Buffer);
		}

		void Close(Assimp::IOStream* pFile) override
		{
			delete pFile;
		}
	};

	glm::vec2 FAssimpLibrary::aiVec2ToGlm(const aiVector3D& v)
	{
		return glm::vec2(v.x, v.y);
	}

	glm::vec3 FAssimpLibrary::aiVec3ToGlm(const aiVector3D& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	glm::mat4 FAssimpLibrary::AiMat4ToGlm(const aiMatrix4x4& from)
	{
		glm::mat4 to(1.0f);
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	void FAssimpLibrary::AddBoneData(Vertex& vertex, int boneID, float weight)
	{
		// find first empty slot
		for (int i = 0; i < 4; ++i) {
			if (vertex.Weights[i] < 1e-6f) {
				vertex.BoneIDs[i] = boneID;
				vertex.Weights[i] = weight;
				return;
			}
		}
		// none empty -> replace smallest if new weight larger
		int smallestIdx = 0;
		float smallestVal = vertex.Weights[0];
		for (int i = 1; i < 4; ++i) {
			if (vertex.Weights[i] < smallestVal) {
				smallestVal = vertex.Weights[i];
				smallestIdx = i;
			}
		}
		if (weight > smallestVal) {
			vertex.BoneIDs[smallestIdx] = boneID;
			vertex.Weights[smallestIdx] = weight;
		}
	}

	MeshData FAssimpLibrary::ProcessMesh(aiMesh* mesh, const glm::mat4& transform, TSharedPtr<FAssimpScene>& AiScene, MinMax<glm::vec3, glm::vec3>& AiMinMax, const std::string& nodeName)
	{
		MeshData out;
		out.name = mesh->mName.C_Str();
		if (out.name.empty()) out.name = nodeName;

		// reserve
		out.vertices.resize(mesh->mNumVertices);
		out.indices.reserve(mesh->mNumFaces * 3);

		// precompute normal matrix
		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(transform)));

		// vertices
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
			Vertex vert;

			// position (apply transform)
			glm::vec4 p = transform * glm::vec4(aiVec3ToGlm(mesh->mVertices[v]), 1.0f);
			vert.Position = glm::vec3(p) / p.w;

			// normal
			if (mesh->HasNormals()) {
				glm::vec3 n = aiVec3ToGlm(mesh->mNormals[v]);
				vert.Normal = glm::normalize(normalMat * n);
			} else {
				vert.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
			}
			
			// tangent + handedness
			if (mesh->HasTangentsAndBitangents()) {
				glm::vec3 T = aiVec3ToGlm(mesh->mTangents[v]);
				glm::vec3 B = aiVec3ToGlm(mesh->mBitangents[v]);

				glm::vec3 Tt = glm::mat3(transform) * T;
				glm::vec3 Bt = glm::mat3(transform) * B;

				// orthonormalize Tt w.r.t N
				Tt = glm::normalize(Tt - vert.Normal * glm::dot(vert.Normal, Tt));
				Bt = glm::normalize(Bt - vert.Normal * glm::dot(vert.Normal, Bt) - Tt * glm::dot(Tt, Bt));

				float handedness = (glm::dot(glm::cross(vert.Normal, Tt), Bt) < 0.0f) ? -1.0f : 1.0f;
				vert.Tangent = glm::vec4(glm::normalize(Tt), handedness);
			} else {
				vert.Tangent = glm::vec4(0.0f,0.0f,0.0f, 1.0f);
			}

			// texcoords
			if (mesh->HasTextureCoords(0)) {
				auto &tc = mesh->mTextureCoords[0][v];
				vert.TexCoords = glm::vec2(tc.x, 1.0f - tc.y);
			} else {
				vert.TexCoords = glm::vec2(0.0f);
			}

			// bones: default zeros (will be filled later below)
			vert.BoneIDs = glm::ivec4(0);
			vert.Weights = glm::vec4(0.0f);

			out.vertices[v] = vert;
			UpdateMinMax(vert, AiMinMax);
		}

		// indices
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			const aiFace& face = mesh->mFaces[f];
			for (unsigned int idx = 0; idx < face.mNumIndices; ++idx) {
				out.indices.push_back(face.mIndices[idx]);
			}
		}

		// bones (unchanged) - fill BoneIDs and Weights
		if (mesh->mNumBones > 0) {
			for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
				aiBone* aibone = mesh->mBones[b];
				std::string boneName = aibone->mName.C_Str();
				int boneIndex = -1;
				auto it = AiScene->BoneMapping.find(boneName);
				if (it == AiScene->BoneMapping.end()) {
					boneIndex = static_cast<int>(AiScene->BoneOffsets.size());
					AiScene->BoneMapping[boneName] = boneIndex;
					AiScene->BoneOffsets.push_back(AiMat4ToGlm(aibone->mOffsetMatrix));
				} else boneIndex = it->second;

				for (unsigned int w = 0; w < aibone->mNumWeights; ++w) {
					aiVertexWeight vw = aibone->mWeights[w];
					unsigned int vertexId = vw.mVertexId;
					float weight = vw.mWeight;
					if (vertexId < out.vertices.size()) {
						AddBoneData(out.vertices[vertexId], boneIndex, weight);
					}
				}
			}

			// normalize weights
			for (auto &vv : out.vertices) {
				float sum = vv.Weights.x + vv.Weights.y + vv.Weights.z + vv.Weights.w;
				if (sum > 0.0f) vv.Weights /= sum;
			}
		}

		out.materialIndex = mesh->mMaterialIndex;

		UE_LOG(LogTemp, Log, TEXT("Mesh: %s | Verts: %d | Faces: %d"),
		*FString(UTF8_TO_TCHAR(mesh->mName.C_Str())),
		static_cast<int>(mesh->mNumVertices),
		static_cast<int>(mesh->mNumFaces));

		return out;
	}

	void FAssimpLibrary::ProcessNode(TSharedPtr<FAssimpScene>& AiScene, const aiScene* scene, MinMax<glm::vec3, glm::vec3>& AiMinMax)
	{
		ProcessNode(AiScene, scene->mRootNode, scene, glm::mat4(1.0f), AiMinMax);
	}

	void FAssimpLibrary::ProcessNode(TSharedPtr<FAssimpScene>& AiScene, aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, MinMax<glm::vec3, glm::vec3>& AiMinMax)
	{
		if (!node)
		{
			UE_LOG(LogTemp, Warning, TEXT("Assimp node is null or invalid."));
			return;
		}

		// node-local transform -> convert & combine with parent
		glm::mat4 nodeTransform = AiMat4ToGlm(node->mTransformation);
		glm::mat4 globalTransform = parentTransform * nodeTransform;
		
		// process all meshes referenced by this node
		for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
			unsigned int meshIndex = node->mMeshes[i];
			aiMesh* mesh = scene->mMeshes[meshIndex];
			MeshData meshData = ProcessMesh(mesh, globalTransform, AiScene, AiMinMax, node->mName.C_Str());
			AiScene->Meshes.push_back(std::move(meshData));
		}
		
		// recurse children
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			ProcessNode(AiScene, node->mChildren[i], scene, globalTransform, AiMinMax);
		}
	}

	void FAssimpLibrary::UpdateMinMax(const Vertex& vert, MinMax<glm::vec3, glm::vec3>& AiMinMax)
	{
		AiMinMax.Min.x = std::min(AiMinMax.Min.x, vert.Position.x);
		AiMinMax.Min.y = std::min(AiMinMax.Min.y, vert.Position.y);
		AiMinMax.Min.z = std::min(AiMinMax.Min.z, vert.Position.z);
		AiMinMax.Max.x = std::max(AiMinMax.Max.x, vert.Position.x);
		AiMinMax.Max.y = std::max(AiMinMax.Max.y, vert.Position.y);
		AiMinMax.Max.z = std::max(AiMinMax.Max.z, vert.Position.z);
	}

	bool FAssimpLibrary::LoadModelAssetFromMemory(TSharedPtr<FInMemoryFBXStream> MObject, TSharedPtr<FAssimpScene>& AiScene)
	{
		if (!MObject)
		{
			UE_LOG(LogTemp, Error, TEXT("MObject is null or invalid."));
			return false;
		}
		
		if (!MObject->IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("InMemoryStream is invalid"));
			return false;
		}

		if (MObject->GetBuffer().Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("MemoryStream is empty."));
			return false;
		}
		
		AiScene = MakeShareable<FAssimpScene>(new FAssimpScene());

		std::string FileName = TCHAR_TO_UTF8(*(MObject->GetName() + TEXT(".fbx")));
		FbxMemoryIOSystem* IOSystem = new FbxMemoryIOSystem(MObject->GetBuffer());
		AiScene->SetIOHandler(IOSystem);

		const aiScene* Scene = AiScene->GetImporter().ReadFile(
			FileName.c_str(),
			aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
		);

		if (!Scene || !Scene->HasMeshes())
		{
			UE_LOG(LogTemp, Error, TEXT("Assimp Load Failed: %s"), *FString(UTF8_TO_TCHAR(AiScene->GetImporter().GetErrorString())));
			return false;
		}
		
		if (!MObject->IsValid() || MObject->GetName().IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("MObject is null."));
			return false;
		}
		
		return true;
	}

	bool FAssimpLibrary::LoadModelAssetFromFile(const FString& FilePath, TSharedPtr<FAssimpScene>& AiScene)
	{
		AiScene = MakeShareable<FAssimpScene>(new FAssimpScene());

		std::string PathStr = TCHAR_TO_UTF8(*FilePath);
		const aiScene* Scene = AiScene->GetImporter().ReadFile(PathStr,
	aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals
		| aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

		if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
			//std::cout << "ERROR::ASSIMP::" << AiScene->Importer.GetErrorString() << std::endl;
			FString AssimpError = UTF8_TO_TCHAR(AiScene->GetImporter().GetErrorString());
			UE_LOG(LogTemp, Error, TEXT("AiScene error: %s"), *AssimpError);
			return false;
		}
		
		if (!Scene || !Scene->HasMeshes())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load FBX file: %s"), *FilePath);
			return false;
		}

		AiScene->FilePath = FilePath;
		return true;
	}

	bool FAssimpLibrary::ConvertAssimpToMesh(TSharedPtr<FAssimpScene>& AiScene)
	{
		const FString& FilePath = AiScene->FilePath;
		const aiScene* Scene = AiScene->GetAiScene();
		
		if (!Scene || (Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !Scene->mRootNode) {
			if (AiScene.IsValid()) {
				UE_LOG(LogTemp, Error, TEXT("Assimp Load Failed: %s"), *FString(UTF8_TO_TCHAR(AiScene->GetImporter().GetErrorString())));
			} else {
				UE_LOG(LogTemp, Error, TEXT("Assimp Load Failed: invalid scene and null AiScene"));
			}
			return false;
		}

		if (!AiScene.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("AiScene is null."));
			return false;
		}

		AiScene->Meshes.clear();
		AiScene->BoneMapping.clear();
		AiScene->BoneOffsets.clear();

		MinMax<glm::vec3, glm::vec3> globalMinMax = MinMax<glm::vec3, glm::vec3>(glm::vec3(std::numeric_limits<float>::max()),
			glm::vec3(std::numeric_limits<float>::lowest()));

		ProcessNode(AiScene, Scene, globalMinMax);

		AiScene->BBox = globalMinMax;
		AiScene->Center = (globalMinMax.Min + globalMinMax.Max) * 0.5f;

		UE_LOG(LogTemp, Log, TEXT("Path: %s"),*FilePath);
		
		return true;
	}
}
