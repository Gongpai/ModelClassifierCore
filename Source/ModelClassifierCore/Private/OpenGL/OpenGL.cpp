#include "OpenGL/OpenGL.h"
#include <iostream>
#include "Assimp/AssimpScene.h"
#include "assimp/Vertex.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "stb/stb_image_write.h"
#include "stb/stb_image.h"
#include "glm/gtc/type_ptr.hpp"

namespace ModelClassifierCore
{
	TMap<FString, FString> FOpenGL::ShaderFiles;
	
	void FOpenGL::GetAllShaderFiles()
	{
		if (ShaderFiles.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("All shader loaded!"));
			return;
		}
		
		TArray<FString> AllShaderFiles;

		FString ShaderPaths = FPaths::ConvertRelativePathToFull(ModelClassifierCoreModule->GetThirdPartyAssetPath(0));
		IFileManager::Get().FindFilesRecursive(
			AllShaderFiles,
			*ShaderPaths,
			TEXT("*.*"),
			true, 
			false
		);

		UE_LOG(LogTemp, Log, TEXT("Found %d shader(s) in %s"), AllShaderFiles.Num(), *ShaderPaths);

		for (int i = 0; i < AllShaderFiles.Num(); ++i)
		{
			const FString& FilePath = AllShaderFiles[i];
			const FString& FileName = FPaths::GetCleanFilename(FilePath);
					
			UE_LOG(LogTemp, Log, TEXT("Shader: %s"), *FileName);
			UE_LOG(LogTemp, Log, TEXT("Path: %s"), *FilePath);

			ShaderFiles.Add(*FileName, LoadShaderFileToString(FilePath));
		}

		UE_LOG(LogTemp, Log, TEXT("All shader loaded!"));
	}

	FString FOpenGL::LoadShaderFileToString(const FString& FilePath)
	{
		FString ShaderSource;
		if (FFileHelper::LoadFileToString(ShaderSource, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Loaded shader from: %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load shader: %s"), *FilePath);
		}
		return ShaderSource;
	}

	GLFWwindow* FOpenGL::CreateContext()
	{
		return CreateContext(RenderSize);
	}

	GLFWwindow* FOpenGL::CreateContext(ImageSize InRenderSize)
	{
		if (!glfwInit())
		{
			//std::cerr << "Failed to initialize GLFW\n";
			UE_LOG(LogTemp, Error, TEXT("Failed to initialize GLFW"));
			return nullptr;
		}

		glfwWindowHint(GLFW_SAMPLES, 16);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(InRenderSize.width, InRenderSize.height, "", nullptr, nullptr);
		if (!window)
		{
			//std::cerr << "Failed to create GLFW window\n";
			UE_LOG(LogTemp, Error, TEXT("Failed to create GLFW window"));
			glfwTerminate();
			return nullptr;
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(0);
		
		if (!gladLoadGLLoader((GLADloadproc)(void*)glfwGetProcAddress))
		{
			//std::cerr << "Failed to initialize GLAD\n";
			UE_LOG(LogTemp, Error, TEXT("Failed to initialize GLAD"));
			return nullptr;
		}

		//std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
		const GLubyte* VersionStr = glGetString(GL_VERSION);
		FString OpenGLVersion = ANSI_TO_TCHAR(reinterpret_cast<const char*>(VersionStr));
		UE_LOG(LogTemp, Log, TEXT("OpenGL Version: %s"), *OpenGLVersion);
		UE_LOG(LogTemp, Log, TEXT("Create Context Complete."));

		if (!CreateFramebuffer())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create FBO!"));
		}

		return window;
	}

	bool FOpenGL::CreateFramebuffer()
	{
		return CreateFramebuffer(RenderSize);
	}

	bool FOpenGL::CreateFramebuffer(ImageSize InRenderSize)
	{
		// --- Color Texture ---
		GLuint colorTexture = 0;
		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, InRenderSize.width, InRenderSize.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// --- Create FBO ---
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, InRenderSize.width, InRenderSize.height);
		
		// --- Depth + Stencil Renderbuffer ---
		glGenRenderbuffers(1, &depthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, InRenderSize.width, InRenderSize.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			//std::cerr << "Framebuffer is not complete!\n";
			auto glError = std::to_string(glGetError());
			FString ErrorString = FString(UTF8_TO_TCHAR(glError.c_str()));
			UE_LOG(LogTemp, Error, TEXT("Framebuffer is incomplete: %s"), *ErrorString);
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Framebuffer is complete!"));
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		colorTex = colorTexture;
		return true;
	}

	void FOpenGL::ReleaseFramebuffer()
	{
		glDeleteRenderbuffers(1, &depthRBO);
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &colorTex);
	}

	bool FOpenGL::RenderSceneToFBO(GLuint program, TSharedPtr<FAssimpScene> AssimpScene, FString OutputPath,  Camera InCamera, Light& InLights, std::vector<unsigned char>& OutPixels)
	{
		return RenderSceneToFBO(program, AssimpScene, OutputPath, RenderSize, InCamera, InLights, OutPixels);
	}

	bool FOpenGL::RenderSceneToFBO(GLuint program, TSharedPtr<FAssimpScene> AssimpScene, FString OutputPath, ImageSize InRenderSize, Camera InCamera, Light& InLights, std::vector<unsigned char>& OutPixels)
	{
		if (fbo == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FBO not create!"));
			return false;
		}

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		
		glViewport(0, 0, InRenderSize.width, InRenderSize.height);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		glUseProgram(program);

		BindCameraUniforms(program, InCamera);
		BindLightingUniforms(program, InLights);

		std::vector<MeshData>& Meshes = AssimpScene->Meshes;
		for (int i = 0; i < Meshes.size(); i++)
		{
			MeshData& Mesh = Meshes[i];
			if (Mesh.VAO == 0) Mesh.InitializeMeshBuffers();
			
			auto Material = AssimpScene->Materials[Mesh.materialIndex];
			
			BindPBRTextures(program, Material);
			DrawMesh(Mesh);
		}

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		
		std::vector<unsigned char> pixels(InRenderSize.width * InRenderSize.height * 4);
		std::vector<unsigned char> pixelsForPNG(InRenderSize.width * InRenderSize.height * 4);
		
		glReadPixels(0, 0, InRenderSize.width, InRenderSize.height, ImageFormat == RGBA ? GL_RGBA : GL_BGRA, GL_UNSIGNED_BYTE, pixels.data());
		UE_LOG(LogTemp, Log, TEXT("Render readpixels size: %d (expected %d)"), pixels.size(), InRenderSize.width * InRenderSize.height * 4);

		UE_LOG(LogTemp, Log, TEXT("Save image to: %s"), *OutputPath);
		std::string OutputPathStr = TCHAR_TO_ANSI(*OutputPath);
		
		if (ImageFormat == BGRA)
		{
			glReadPixels(0, 0, InRenderSize.width, InRenderSize.height, GL_RGBA, GL_UNSIGNED_BYTE, pixelsForPNG.data());
		}else
		{
			pixelsForPNG = pixels;
		}
		stbi_flip_vertically_on_write(1);
		stbi_write_png(OutputPathStr.c_str(), InRenderSize.width, InRenderSize.height, 4, pixelsForPNG.data(), InRenderSize.width * 4);

		OutPixels = pixels;
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);

		return true;
	}

	std::vector<float> FOpenGL::ConvertToFloat(std::vector<unsigned char>& InPixels, const std::vector<float> mean, const std::vector<float> std)
	{
		std::vector<float> NewPixels;
		
		UE_LOG(LogTemp, Log, TEXT("Pixel size: %d, expected %d (w=%d, h=%d)"),
		InPixels.size(),
		RenderSize.width * RenderSize.height * 4,
		RenderSize.width,
		RenderSize.height);
		
		check(InPixels.size() == RenderSize.width * RenderSize.height * 4);

		NewPixels.resize(RenderSize.width * RenderSize.height * 3);
		std::vector<unsigned char> pixels(RenderSize.width * RenderSize.height * 4);

		for (int y = 0; y < RenderSize.height; y++)
		{
			for (int x = 0; x < RenderSize.width; x++)
			{
				int idx = (y * RenderSize.width + x) * 4;
				float r = InPixels[idx + 0] / 255.0f;
				float g = InPixels[idx + 1] / 255.0f;
				float b = InPixels[idx + 2] / 255.0f;

				r = (r - mean[0]) / std[0];
				g = (g - mean[1]) / std[1];
				b = (b - mean[2]) / std[2];

				int outIndexR = 0 * RenderSize.width * RenderSize.height + y * RenderSize.width + x;
				int outIndexG = 1 * RenderSize.width * RenderSize.height + y * RenderSize.width + x;
				int outIndexB = 2 * RenderSize.width * RenderSize.height + y * RenderSize.width + x;

				NewPixels[outIndexR] = r;
				NewPixels[outIndexG] = g;
				NewPixels[outIndexB] = b;
			}
		}

		return NewPixels;
	}

	bool FOpenGL::LoadShader(const FString& FileName, GLenum type, GLuint& OutShader)
	{
		if (!ShaderFiles.Contains(FileName)) {
			UE_LOG(LogTemp, Error, TEXT("Shader %s not found in ShaderFiles"), *FileName);
			return false;
		}
		
		FTCHARToUTF8 UTF8String(*ShaderFiles[FileName]);
		const char* src = UTF8String.Get();

		//UE_LOG(LogTemp, Log, TEXT("Loaded shader %s file:\n%s"), *FileName, *FString(src));
		
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		
		if (!success) {
			char log[512];
			glGetShaderInfoLog(shader, 512, nullptr, log);

			//std::cerr << "Shader compile error: " << log << "\n";
			UE_LOG(LogTemp, Error, TEXT("Shader compilation failed: %s"), *FString(log));
			OutShader = 0;
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Shader compilation complete!"));
		OutShader = shader;

		return true;
	}

	bool FOpenGL::CreateProgram(const FString& VertName, const FString& FragName, GLuint& OutProgram)
	{
		if (!ShaderFiles.Contains(VertName)) {
			UE_LOG(LogTemp, Error, TEXT("Shader %s not found in ShaderFiles"), *VertName);
			return false;
		}else
		{
			UE_LOG(LogTemp, Log, TEXT("Loaded shader %s with %d chars"), *VertName, ShaderFiles[VertName].Len());
		}

		if (!ShaderFiles.Contains(FragName)) {
			UE_LOG(LogTemp, Error, TEXT("Shader %s not found in ShaderFiles"), *FragName);
			return false;
		}else
		{
			UE_LOG(LogTemp, Log, TEXT("Loaded shader %s with %d chars"), *FragName, ShaderFiles[FragName].Len());
		}
		
		GLuint vs;
		bool bIsVartValid = LoadShader(VertName, GL_VERTEX_SHADER, vs);

		GLuint fs;
		bool bIsFragValid = LoadShader(FragName, GL_FRAGMENT_SHADER, fs);

		if (!bIsFragValid || !bIsVartValid)
		{
			return false;
		}

		GLuint prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);

		GLint success;
		glGetProgramiv(prog, GL_LINK_STATUS, &success);
		if (!success) {
			char log[512];
			glGetProgramInfoLog(prog, 512, nullptr, log);
			//std::cerr << "Program link error: " << log << "\n";
			UE_LOG(LogTemp, Error, TEXT("Program link failed: %s"), *FString(log));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Program link complete!"));
		glDeleteShader(vs);
		glDeleteShader(fs);
		OutProgram = prog;

		return true;
	}

	void FOpenGL::BindPBRTextures(GLuint& program, const Material& mat)
	{
		glUseProgram(program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mat.albedoTex);
		glUniform1i(glGetUniformLocation(program, "albedoMap"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mat.normalTex);
		glUniform1i(glGetUniformLocation(program, "normalMap"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mat.metallicTex);
		glUniform1i(glGetUniformLocation(program, "metallicMap"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mat.roughnessTex);
		glUniform1i(glGetUniformLocation(program, "roughnessMap"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, mat.aoTex);
		glUniform1i(glGetUniformLocation(program, "aoMap"), 4);
	}

	bool FOpenGL::LoadTextureFromAssimpMaterial(TSharedPtr<FAssimpScene> Scene, aiMaterial* material, aiTextureType type, GLuint& OutTexture)
	{
		if (!material) return false;

		aiString texPath;
		if (material->GetTexture(type, 0, &texPath) != AI_SUCCESS)
			return false;

		FString modelDir = FPaths::GetPath(Scene->FilePath);
		FString texFile = UTF8_TO_TCHAR(texPath.C_Str());
		FString FullPath = FPaths::IsRelative(texFile)
			? FPaths::ConvertRelativePathToFull(modelDir / texFile)
			: texFile;

		int width, height, channels;
		bool isHDR = FullPath.EndsWith(TEXT(".hdr"), ESearchCase::IgnoreCase);

		stbi_set_flip_vertically_on_load(true);

		void* data = nullptr;
		if (isHDR)
			data = stbi_loadf(TCHAR_TO_ANSI(*FullPath), &width, &height, &channels, 0);
		else
			data = stbi_load(TCHAR_TO_ANSI(*FullPath), &width, &height, &channels, 0);

		if (!data) return false;

		GLenum format = GL_RGB;
		GLenum internalFormat = GL_RGB8;

		switch (channels) {
		case 1:
			format = GL_RED;
			internalFormat = isHDR ? GL_R16F : GL_RED;
			break;
		case 2:
			format = GL_RG;
			internalFormat = isHDR ? GL_RG16F : GL_RG;
			break;
		case 3:
			format = GL_RGB;
			internalFormat = isHDR ? GL_RGB16F : GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = isHDR ? GL_RGBA16F : GL_RGBA;
			break;
		default:
			stbi_image_free(data);
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("Texture loaded successfully: %s"), *FullPath);

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(
			GL_TEXTURE_2D, 0, internalFormat,
			width, height, 0,
			format,
			isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE,
			data
		);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		OutTexture = textureID;
		return true;
	}

	GLuint FOpenGL::CreateSolidColorTexture(unsigned char r, unsigned char g, unsigned char b)
	{
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		unsigned char data[4] = { r, g, b, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		return tex;
	}

	void FOpenGL::BindCameraUniforms(GLuint program, Camera InCamera)
	{
		glUseProgram(program);

		GLint loc_model = glGetUniformLocation(program, "model");
		GLint loc_view = glGetUniformLocation(program, "view");
		GLint loc_proj = glGetUniformLocation(program, "projection");
		GLint loc_cam = glGetUniformLocation(program, "uCameraPos");

		glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(InCamera.model));
		glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(InCamera.view));
		glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(InCamera.projection));
		glUniform3fv(loc_cam, 1, glm::value_ptr(InCamera.cameraPos));
	}

	void FOpenGL::BindLightingUniforms(GLuint program, Light& InLights)
	{
		glUseProgram(program);

		GLint locCol = glGetUniformLocation(program, "lightColor");
		glUniform3fv(locCol, 1, glm::value_ptr(InLights.color));

		GLint locInt = glGetUniformLocation(program, "lightIntensity");
		glUniform1f(locInt, InLights.intensity);
	}

	void FOpenGL::DrawMesh(MeshData& Meshes)
	{
		glBindVertexArray(Meshes.VAO);
		glDrawElements(GL_TRIANGLES, Meshes.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
