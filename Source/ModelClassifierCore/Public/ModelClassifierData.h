#pragma once
#include "NNE/ClassifierNNEModel.h"
#include "FileType.h"
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
	
	struct MODELCLASSIFIERCORE_API FModelClassifierData
	{
	private:
		FString Name;
		FString ModelPath;
		EFileType FileType = RawFBX;
		FString ImageRenderPath;
		FModelClassifierCoreModule* Core;
		ERotateMode RotateMode = ERotateMode::All;
		TArray<std::vector<unsigned char>> PixelData;
		TArray<FSlateBrush> Brushes;
		int ResultIndex;
		int RenderCount = 4;
		ImageSize RenderSize;

		FImageRendered OnImageRendered;
		FImageClassified OnImageClassified;

		bool RunRenderAndClassifier(TSharedPtr<FNNEImageFeatures> NNERuntime, TSharedPtr<FAssimpScene> AiScene, int& OutResult);
		void StartClassifier(FString Path, EFileType Type, TFunction<void(int)> OnComplete);
	
	public:
		FModelClassifierData() : Core(nullptr), ResultIndex(0){}
		FModelClassifierData(FModelClassifierCoreModule* InCore) : Core(InCore), ResultIndex(0) {}
		FModelClassifierData(FModelClassifierCoreModule* InCore, FString Name, EFileType Type, FString ModelSavePath);
		~FModelClassifierData()
		{
			UE_LOG(LogTemp, Warning, TEXT("**** FModelClassifierData has destroy!"));
			
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
		void SetResultClassifier(const int& InResultIndex);

		TArray<std::vector<unsigned char>> GetPixelData();
		TArray<FSlateBrush> GetImageRenderBrushes();

		FImageRendered& GetImageRenderedAction();
		FImageClassified& GetImageClassifiedAction();
		
	public:
		static FString GetFileTypeString(EFileType InFileType);
	};
}