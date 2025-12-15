#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Render/Camera.h"
#include "Render/Material.h"
#include "Render/Light.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "glm/gtc/type_ptr.hpp"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include "Utilities/Collections.h"
#include "ModelClassifierCore.h"
#include "Assimp/AssimpScene.h"
#include "Render/Mesh.h"

namespace ModelClassifierCore
{
	class FModelClassifierCoreModule;
	struct ImageSize;

	class FOpenGL
	{
		GLuint fbo = 0;
		GLuint colorTex = 0;
		GLuint depthRBO;
		ImageSize RenderSize;
		EImageFormat ImageFormat;
		FModelClassifierCoreModule* ModelClassifierCoreModule;

		void GetAllShaderFiles();
		FString LoadShaderFileToString(const FString& FilePath);

		static TMap<FString, FString> ShaderFiles;
		
	public:
		FOpenGL(ImageSize InRenderSize): RenderSize(InRenderSize), ModelClassifierCoreModule(FModuleManager::GetModulePtr<FModelClassifierCoreModule>("ModelClassifierCore"))
		{
			GetAllShaderFiles();
		}
		~FOpenGL()
		{
			UE_LOG(LogTemp, Warning, TEXT("FOpenGL has destroy!"));
		}

		void SetImageFormat(EImageFormat InImageFormat) { ImageFormat = InImageFormat; }
		
		GLFWwindow* CreateContext();
		GLFWwindow* CreateContext(ImageSize InRenderSize);

		bool CreateFramebuffer();
		bool CreateFramebuffer(ImageSize InRenderSize);
		void ReleaseFramebuffer();

		bool RenderSceneToFBO(GLuint program, TSharedPtr<FAssimpScene> AssimpScene, FString OutputPath, Camera InCamera, Light& InLights, std::vector<unsigned char>& OutPixels);
		bool RenderSceneToFBO(GLuint program, TSharedPtr<FAssimpScene> AssimpScene, FString OutputPath, ImageSize InRenderSize, Camera InCamera, Light& InLights, std::vector<unsigned char>& OutPixels);

		std::vector<float> ConvertToFloat(std::vector<unsigned char>& InPixels, const std::vector<float> mean = {0.48145466f, 0.4578275f, 0.40821073f}, const std::vector<float> std = {0.26862954f, 0.26130258f, 0.27577711f});
		
		bool LoadShader(const FString& FileName, GLenum type, GLuint& OutShader);
		bool CreateProgram(const FString& VertName, const FString& FragName, GLuint& OutProgram);

		void BindPBRTextures(GLuint& program, const Material& mat);
		bool LoadTextureFromAssimpMaterial(TSharedPtr<FAssimpScene> Scene, aiMaterial* material, aiTextureType type, GLuint& OutTexture);
		GLuint CreateSolidColorTexture(unsigned char r, unsigned char g, unsigned char b);

		void BindCameraUniforms(GLuint program, Camera InCamera);
		void BindLightingUniforms(GLuint program, Light& InLights);
		void DrawMesh(MeshData& Meshes);
	};
}
