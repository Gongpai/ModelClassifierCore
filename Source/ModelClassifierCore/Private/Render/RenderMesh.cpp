#include "Render/RenderMesh.h"
#include "Assimp/AssimpScene.h"
#include "glm/gtx/component_wise.inl"
#include "ThirdParty/ModelClassifierCoreLibrary/assimp/contrib/rapidjson/include/rapidjson/reader.h"

namespace ModelClassifierCore
{
	FRenderMesh::FRenderMesh() : ModelClassifierCoreModule(FModuleManager::GetModuleChecked<FModelClassifierCoreModule>("ModelClassifierCore"))
	{
	}

	void FRenderMesh::SetImageFormat(EImageFormat InImageFormat)
	{
		ImageFormat = InImageFormat;
	}

	void FRenderMesh::Render(int RenderCount, ERotateMode InRotateMode, TFunction<void(std::vector<unsigned char>, std::vector<float>)> OnImageRendered)
	{
		ImageSize& Size = AssimpScene->Size;
		UE_LOG(LogTemp, Log, TEXT("Input size: W(%d) : H(%d)"), Size.width, Size.height);
		
		OpenGLContext = MakeShareable(new FOpenGL(Size));
		
		GLFWwindow* window = OpenGLContext.Get()->CreateContext();
		if (!window)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create window context"));
			return;
		}

		glfwMakeContextCurrent(window);

		if (!AssimpScene.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Assimp scene has not been assigned!"));
			return;
		}

		// Load All Meshes
		FAssimpLibrary::ConvertAssimpToMesh(AssimpScene);
		
		AssimpScene->Materials = LoadAllMaterials();

		// Create Shader + Materials
		GLuint pbrProgram;
		bool bIsProgramSuccess = OpenGLContext->CreateProgram("pbr.vert", "pbr.frag", pbrProgram);

		if (!bIsProgramSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("Shader system initialization failed!"));
			OpenGLContext->ReleaseFramebuffer();
			return;
		}

		float CameraFOV = 45.0f;
		MinMax<glm::vec3, glm::vec3> BBox = AssimpScene.Get()->BBox;
		float camDistance = GetAutoCameraDistanceToFit(BBox.Min, BBox.Max, CameraFOV,
			static_cast<float>(Size.width) / static_cast<float>(Size.height), 0.75f);
		Cameras = CreateSphericalCameraOrbit(RenderCount, camDistance, AssimpScene->Center, CameraFOV, Size, InRotateMode);

		UE_LOG(LogTemp, Log, TEXT("Camera Distance: %f"), camDistance);

		Lights.color = glm::vec3(1.0f, 1.0f, 1.0f);
		Lights.intensity = 1.0f;

		int num = 0;
		FString RandomFolderName = ModelClassifierCoreModule.GetTempRenderPath();
		for (auto Preset : Cameras)
		{
			FString OutputPath = FPaths::Combine(RandomFolderName, FString::Printf(TEXT("%d_output.png"), num));
			num++;
			
			std::vector<unsigned char> OutPixels;
			OpenGLContext->SetImageFormat(ImageFormat);
			
			if (!OpenGLContext.Get()->RenderSceneToFBO(pbrProgram, AssimpScene, OutputPath, Preset, Lights, OutPixels))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to render scene fbo!"));
				OpenGLContext->ReleaseFramebuffer();
				return;
			}

			std::vector<float> OutFloatPixels;

			OutFloatPixels = OpenGLContext->ConvertToFloat(OutPixels);
			
			if (OnImageRendered)
			{
				OnImageRendered(OutPixels, OutFloatPixels);
			}
		}

		OpenGLContext.Get()->ReleaseFramebuffer();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void FRenderMesh::SetAssimpScene(TSharedPtr<FAssimpScene> Scene)
	{
		AssimpScene = Scene;
	}

	std::vector<Material> FRenderMesh::LoadAllMaterials()
	{
		if (!OpenGLContext.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("OpenGL Context Is Invalid!"));
			return Materials;
		}
		
		Materials.clear();
		Materials.reserve(AssimpScene->GetAiScene()->mNumMaterials);

		for (unsigned int i = 0; i < AssimpScene->GetAiScene()->mNumMaterials; i++)
		{
			aiMaterial* material = AssimpScene->GetAiScene()->mMaterials[i];
			Material mat{};

			// Albedo
			if (!OpenGLContext.Get()->LoadTextureFromAssimpMaterial(AssimpScene, material, aiTextureType_DIFFUSE, mat.albedoTex))
			{
				aiColor3D color(1.0f, 1.0f, 1.0f);
				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				GLuint tex = OpenGLContext->CreateSolidColorTexture(
					(unsigned char)(color.r * 255),
					(unsigned char)(color.g * 255),
					(unsigned char)(color.b * 255));

				mat.albedoTex = tex;
			}
			
			// Normal
			if (!OpenGLContext->LoadTextureFromAssimpMaterial(AssimpScene, material, aiTextureType_NORMALS, mat.normalTex))
			{
				mat.normalTex = OpenGLContext->CreateSolidColorTexture(128, 128, 255);
			}

			// Roughness
			if (!OpenGLContext->LoadTextureFromAssimpMaterial(AssimpScene, material, aiTextureType_SHININESS, mat.roughnessTex))
			{
				mat.roughnessTex = OpenGLContext->CreateSolidColorTexture(255, 255, 255);
			}

			// Metallic
			if (!OpenGLContext->LoadTextureFromAssimpMaterial(AssimpScene, material, aiTextureType_SPECULAR, mat.metallicTex))
			{
				mat.metallicTex = OpenGLContext->CreateSolidColorTexture(0, 0, 0);
			}

			// AO
			if (!OpenGLContext->LoadTextureFromAssimpMaterial(AssimpScene, material, aiTextureType_AMBIENT, mat.aoTex))
			{
				mat.aoTex = OpenGLContext->CreateSolidColorTexture(255, 255, 255);
			}
			
			Materials.push_back(mat);
		}

		return Materials;
	}

	TArray<Camera> FRenderMesh::CreateSphericalCameraOrbit(int Count, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, ERotateMode RotateMode)
	{
		TArray<Camera> NewCameras;

		switch (RotateMode)
		{
			case All:
			{
				CalElevationCameraOrbit(Count, CameraDistance, Center, CameraFOV, RenderSize, NewCameras, false);
				CalAzimuthCameraOrbit(Count, CameraDistance, Center, CameraFOV, RenderSize, NewCameras, true);
				return NewCameras;
			}
			case Azimuth:
			{
				CalAzimuthCameraOrbit(Count, CameraDistance, Center, CameraFOV, RenderSize, NewCameras, false);
				return NewCameras;
			}
			case Elevation:
			{
				CalElevationCameraOrbit(Count, CameraDistance, Center, CameraFOV, RenderSize, NewCameras, false);
				return NewCameras;
			}
		}

		return NewCameras;
	}

	float FRenderMesh::CalDegree(int Count, int Step)
	{
		return 360.0f * static_cast<float>(Step) / static_cast<float>(Count);
	}

	void FRenderMesh::CalAzimuthCameraOrbit(int numAzimuth, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, TArray<Camera>& OutCameras, bool bIsOverlap)
	{
		float elevation = 0.f;

		for (int i = (bIsOverlap ? 1 : 0); i < numAzimuth; ++i)
		{
			float deg = CalDegree(numAzimuth, i);

			if (bIsOverlap && fabs(deg - 180.0f) < 0.001f)
			{
				continue;
			}
			
			float theta = glm::radians(deg);
			float phi   = glm::radians(elevation);
			Camera NewCamera;

			float x = CameraDistance * cos(phi) * sin(theta);
			float y = CameraDistance * sin(phi);
			float z = CameraDistance * cos(phi) * cos(theta);

			glm::vec3 pos = Center + glm::vec3(x, y, z);
			NewCamera.cameraPos = pos;
			NewCamera.projection = glm::perspective(glm::radians(CameraFOV), static_cast<float>(RenderSize.width)/static_cast<float>(RenderSize.height), 1.0f, 1000000.0f);
			NewCamera.view = glm::lookAt(pos, Center, glm::vec3(0,1,0));
			NewCamera.model = glm::mat4(1.0f);

			OutCameras.Add(NewCamera);
		}
	}

	void FRenderMesh::CalElevationCameraOrbit(int numElevation, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, TArray<Camera>& OutCameras, bool bIsOverlap)
	{
		float azimuth = 0.f;
		for (int i = (bIsOverlap ? 1 : 0); i < numElevation; ++i)
		{
			float deg = CalDegree(numElevation, i);

			if (bIsOverlap && fabs(deg - 180.0f) < 0.001f)
			{
				continue;
			}
			
			float phi   = glm::radians(deg);
			float theta = glm::radians(azimuth);
			Camera NewCamera;

			float x = CameraDistance * cos(phi) * sin(theta);
			float y = CameraDistance * sin(phi);
			float z = CameraDistance * cos(phi) * cos(theta);

			glm::vec3 pos = Center + glm::vec3(x, y, z);
			NewCamera.cameraPos = pos;
			NewCamera.projection = glm::perspective(glm::radians(CameraFOV), static_cast<float>(RenderSize.width)/static_cast<float>(RenderSize.height), 1.0f, 1000000.0f);
			NewCamera.view = glm::lookAt(pos, Center, glm::vec3(0,1,0));
			NewCamera.model = glm::mat4(1.0f);

			OutCameras.Add(NewCamera);
		}
	}

	float FRenderMesh::GetAutoCameraDistanceToFit(const glm::vec3& BBoxMin, const glm::vec3& BBoxMax, float CameraFOV,
		float AspectRatio, float Padding)
	{
		glm::vec3 size = BBoxMax - BBoxMin;
		float radius = glm::length(size) * 0.5f * Padding;
		
		float fovRad = glm::radians(CameraFOV);

		float distanceV = radius / std::sin(fovRad * 0.5f);

		float fovH = 2.0f * std::atan(std::tan(fovRad * 0.5f) * AspectRatio);
		float distanceH = radius / std::sin(fovH * 0.5f);

		float cameraDistance = glm::max(distanceV, distanceH);

		return cameraDistance;
	}

	UTexture2D* FRenderMesh::MakeTexture2DFromPixels(std::vector<unsigned char> Pixels, ImageSize InImageSize)
	{
		UTexture2D* Texture = UTexture2D::CreateTransient(InImageSize.width, InImageSize.height, PF_R8G8B8A8);
		if (!Texture)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create texture"));
			return nullptr;
		}

#if WITH_EDITORONLY_DATA
		Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
		Texture->SRGB = true;

		void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, Pixels.data(), Pixels.size());
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		Texture->UpdateResource();

		return Texture;
	}

	FSlateBrush FRenderMesh::MakeBrushFromTexture(UTexture2D* Texture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Texture);
		Brush.ImageSize.X = Texture->GetSizeX();
		Brush.ImageSize.Y = Texture->GetSizeY();
		return Brush;
	}
}
