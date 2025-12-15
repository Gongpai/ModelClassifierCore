#pragma once
#include "NNE/ClassifierNNEModel.h"
#include "FileType.h"
#include "ImageFormat.h"
#include "FBX/ExportFBX.h"
#include "FBX/StaticMeshCache.h"
#include "ModelClassifierCore.h"
#include "NNEModelData.h"
#include "Assimp/AssimpLibrary.h"
#include "NNE/Labels.h"
#include "Render/RenderMesh.h"
#include "NNE/NNEImageFeatures.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FImageRendered, const std::vector<unsigned char>);
DECLARE_MULTICAST_DELEGATE_OneParam(FImageClassified, const int);

namespace ModelClassifierCore
{
	class FModelClassifierCoreModule;
	class FStaticMeshCache;
	class FExportFBX;
	
	struct MODELCLASSIFIERCORE_API FModelClassifierHandler
	{
		FModelClassifierCoreModule* Core;
		
	private:
		FString Name;
		FString ModelPath;
		FString ImageRenderPath;
		
		EFileType FileType = RawFBX;
		ERotateMode RotateMode = ERotateMode::All;
		EImageFormat ImageFormat = EImageFormat::RGBA;
		
		TArray<std::vector<unsigned char>> PixelData;
		
		int ResultIndex = -1;
		int RenderCount = 4;
		ImageSize RenderSize;

		FImageRendered OnImageRendered;
		FImageClassified OnImageClassified;

		bool RunRenderAndClassifier(TSharedPtr<FNNEImageFeatures> NNERuntime, TSharedPtr<FAssimpScene> AiScene, int& OutResult);
		void StartClassifier(FString Path, EFileType Type, TFunction<void(int)> OnComplete);
	
	public:
		FModelClassifierHandler() : Core(nullptr){}
		FModelClassifierHandler(FModelClassifierCoreModule* InCore) : Core(InCore){}
		FModelClassifierHandler(FModelClassifierCoreModule* InCore, FString Name, EFileType Type, FString ModelSavePath);
		~FModelClassifierHandler()
		{
			Core = nullptr;
		}

		void RunClassifier(TFunction<void(int)> OnComplete);
	
		void SetName(const FString& InName);
		FString GetName() const;

		void SetFileType(EFileType Type, FString InPath);
		EFileType GetFileType() const;
		FString GetModelPath() const;

		void SetRenderCount(int InRenderCount = 4);
		int GetRenderCount() const;
		void SetRotateMode(ERotateMode InRotateMode);
		FString GetRenderPath() const;
		void SetModelPath(const FString& InPath);
		
		void SetRenderSize(ImageSize InSize);
		ImageSize GetRenderSize();

		int GetResultIndex() const;

		void SetImageFormat(EImageFormat InFormat);
		TArray<std::vector<unsigned char>> GetPixelData();

		FImageRendered& GetImageRenderedAction();
		void ClearImageRenderedAction();
		FImageClassified& GetImageClassifiedAction();
		void ClearImageClassifiedAction();
		
	public:
		static FString GetFileTypeString(EFileType InFileType);
	};
}