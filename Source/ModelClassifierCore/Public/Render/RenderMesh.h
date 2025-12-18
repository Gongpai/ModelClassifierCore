#pragma once
#include "Camera.h"
#include "Material.h"
#include "Render/Light.h"
#include "ModelClassifierCore.h"
#include "Assimp/AssimpScene.h"
#include "OpenGL/OpenGL.h"
#include "Utilities/Collections.h"
#include "ImageFormat.h"

namespace ModelClassifierCore
{
	class FOpenGL;
	class FModelClassifierCoreModule;
	class FAssimpScene;
	
	class MODELCLASSIFIERCORE_API FRenderMesh
	{
		// Viewport size
		int width;
		int height;

		// Materials Data
		std::vector<Material> Materials;

		// Camera Data
		TArray<Camera> Cameras;

		// Light Data
		Light Lights;
		
		EImageFormat ImageFormat;

		TSharedPtr<FOpenGL> OpenGLContext;
		TSharedPtr<FAssimpScene> AssimpScene;
		FModelClassifierCoreModule& ModelClassifierCoreModule;
		
	public:
		FRenderMesh();
		~FRenderMesh()
		{
			UE_LOG(LogTemp, Warning, TEXT("FRenderMesh has destroy!"));
		}

		void SetImageFormat(EImageFormat InImageFormat);
		void Render(int RenderCount, ERotateMode InRotateMode, TFunction<void(std::vector<unsigned char>, std::vector<float>)> OnImageRendered);
		void SetAssimpScene(TSharedPtr<FAssimpScene> Scene);
		std::vector<Material> LoadAllMaterials();
		TArray<Camera> CreateSphericalCameraOrbit(int Count, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, ERotateMode RotateMode);
		float CalDegree(int Count, int Step);
		void CalAzimuthCameraOrbit(int numAzimuth, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, TArray<Camera>& OutCameras, bool bIsOverlap = false);
		void CalElevationCameraOrbit(int numElevation, float CameraDistance, glm::vec3 Center, float CameraFOV, ImageSize RenderSize, TArray<Camera>& OutCameras, bool bIsOverlap = true);
		float GetAutoCameraDistanceToFit(const glm::vec3& BBoxMin, const glm::vec3& BBoxMax, float CameraFOV, float AspectRatio, float Padding = 1.0f);

	public:
		static UTexture2D* MakeTexture2DFromPixels(std::vector<unsigned char> Pixels, ImageSize InImageSize);
		static FSlateBrush MakeBrushFromTexture(UTexture2D* Texture);
	};
}
