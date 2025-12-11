#include "ModelClassifierData.h"

namespace ModelClassifierCore
{
	FModelClassifierData::FModelClassifierData(FModelClassifierCoreModule* InCore, FString Name, EFileType Type,  FString ModelSavePath)
	{
		Core = InCore;
		SetName(Name);
		SetFileType(Type, ModelSavePath);
	}

	bool FModelClassifierData::RunRenderAndClassifier(TSharedPtr<FNNEImageFeatures> NNERuntime, TSharedPtr<FAssimpScene> AiScene, int& OutResult)
	{
		TSharedPtr<FRenderMesh> RenderMesh = MakeShareable(new FRenderMesh());
		TMap<int32, int32> PredictCount;
		RenderMesh.Get()->SetAssimpScene(AiScene);
					
		RenderMesh.Get()->Render(RenderCount, RotateMode, [&NNERuntime, &AiScene, this, &PredictCount](std::vector<unsigned char> OutPixel, std::vector<float> OutFloatPixels)
		{    
			if (OutFloatPixels.size() > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Render Complete!"));

				/*AsyncTask(ENamedThreads::GameThread, [this, OutPixel]()
				{
					UE_LOG(LogTemp, Log, TEXT("Make Brush From Texture!"));
					
					Brushes.Add(FRenderMesh::MakeBrushFromTexture(FRenderMesh::MakeTexture2DFromPixels(OutPixel, GetRenderSize())));
					
					if (GetImageRenderedAction().IsBound())
					{
						GetImageRenderedAction().Broadcast(OutPixel);
					}
				});*/

				if (NNERuntime.IsValid())
				{
					NNERuntime->SetClassifierNNEModelData(Core->GetClassifierNNEModelData().Get());
					NNERuntime->SetImageEncoderDataAsset(Core->GetImageEncoderDataAsset().Get());
					NNERuntime->SetTextFeaturesAsset(Core->GetTextFeaturesAsset().Get());
					NNERuntime->SetInputData(OutPixel, GetRenderSize(), true);
					NNERuntime->SetRuntimeType(ENNEInstanceType::CPU);
					NNERuntime->Initialize();
					
					int OutResultIndex = 0;
					
					if (NNERuntime->RunClassifyRenderedImage(GetRenderSize(), OutResultIndex))
					{
						/*PredictCount.FindOrAdd(OutResultIndex)++;

						AsyncTask(ENamedThreads::GameThread, [this, OutResultIndex]()
						{
							if (GetImageClassifiedAction().IsBound())
							{
								GetImageClassifiedAction().Broadcast(OutResultIndex);
							}
						});*/
						
					}else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to run AI model"));
					}
				}else
				{
					UE_LOG(LogTemp, Error, TEXT("NNE Model is invalid"));
				}
			}else
			{
				UE_LOG(LogTemp, Error, TEXT("Render failure!"));
			}
		});

		int32 MajorityLabel = -1;
		int32 MaxCount = 0;
		for (TPair<int32, int32> Predicted : PredictCount)
		{
			if (Predicted.Value > MaxCount)
			{
				MaxCount = Predicted.Value;
				MajorityLabel = Predicted.Key;
			}
		}

		OutResult = MajorityLabel;
		return true;
	}

	void FModelClassifierData::RunClassifier(TFunction<void(int)> OnComplete)
	{
		StartClassifier(ModelPath, FileType, OnComplete);
	}

	void FModelClassifierData::SetName(const FString& InName)
	{
		Name = InName;
	}

	FString FModelClassifierData::GetName() const
	{
		return Name;
	}

	void FModelClassifierData::SetFileType(EFileType Type, FString InPath)
	{
		FileType = Type;
		ModelPath = InPath;
	}

	EFileType FModelClassifierData::GetFileType() const
	{
		return FileType;
	}

	FString FModelClassifierData::GetFileTypeString(EFileType InFileType)
	{
		switch (InFileType)
		{
			case RawFBX:
				return FString("RawFBX");
			case FBXStream:
				return FString("FBXStream");
			case RawOBJ:
				return FString("RawOBJ");
				default:
				return FString("Unknown");
		}
	}

	void FModelClassifierData::StartClassifier(FString Path, EFileType Type, TFunction<void(int)> OnComplete)
	{
		if (Core == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Core Is Not Valid"));
			return;
		}
		
		if (Core != nullptr && !Core->GetConfig()->ClassifierNNEModelData)
		{
			UE_LOG(LogTemp, Error, TEXT("NNEModelData or Core Is Not Valid"));
			return;
		}
		
		TSharedPtr<FStaticMeshCache> InMemoryCache = MakeShareable(new FStaticMeshCache());
		
		FString RandomTempPath = Core->GenerateRandomFolderName(18);
		if (!FPaths::FileExists(RandomTempPath))
		{
			IFileManager::Get().MakeDirectory(*RandomTempPath, true);
		}
		
		if (!InMemoryCache.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("MemoryCache Is Not Valid"));
			return;
		}

		if (Type == FBXStream)
		{
			InMemoryCache.Get()->LoadStaticMesh(Path, [OnComplete, InMemoryCache, this](bool bIsLoaded)
			{
				if (bIsLoaded)
				{
					UE_LOG(LogTemp, Log, TEXT("Start Writer StaticMesh To FBX Memory"));
					FExportFBX::Writer(InMemoryCache, [OnComplete, this](bool bIsValid, TSharedPtr<FInMemoryFBXStream> InMemoryStream)
					{
						Async(EAsyncExecution::Thread, [OnComplete, InMemoryStream, bIsValid, this]()
						{
							TSharedPtr<FAssimpLibrary> AssimpLibrary = MakeShareable(new FAssimpLibrary());
							TSharedPtr<FAssimpScene> AiScene;
							UE_LOG(LogTemp, Log, TEXT("Begin Writer StaticMesh To FBX Memory"));

							if (AssimpLibrary.IsValid() && bIsValid)
							{
								UE_LOG(LogTemp, Log, TEXT("AssimpLibrary Is Valid"));
								
								if (AssimpLibrary->LoadModelAssetFromMemory(InMemoryStream, AiScene))
								{
									UE_LOG(LogTemp, Log, TEXT("Assimp import complete!"));
									AiScene->Size = RenderSize;
								
									int Result;
									TSharedPtr<FNNEImageFeatures> NNERuntime = MakeShareable(new FNNEImageFeatures());

									if (!NNERuntime.IsValid())
									{
										UE_LOG(LogTemp, Error, TEXT("NNERuntime Is Valid"));
										return;
									}
									
									bool bIsComplete = RunRenderAndClassifier(NNERuntime, AiScene, Result);
								
									AsyncTask(ENamedThreads::GameThread, [bIsComplete, OnComplete, &Result]()
									{
										if (bIsComplete)
										{
											UE_LOG(LogTemp, Log, TEXT("Classifier Complete!"));

											if (OnComplete)
											{
												OnComplete(bIsComplete ? Result : -1);
											}
										}else
										{
											UE_LOG(LogTemp, Error, TEXT("Classifier failure!"));

											if (OnComplete)
											{
												OnComplete(-1);
											}
										}
									});
								}else
								{
									UE_LOG(LogTemp, Log, TEXT("Assimp import failure!"));

									if (OnComplete)
									{
										OnComplete(-1);
									}
								}
							}else
							{
								UE_LOG(LogTemp, Error, TEXT("AssimpLibrary Is Not Valid"));

								if (OnComplete)
								{
									OnComplete(-1);
								}
							}
						});
					});
				}else
				{
					UE_LOG(LogTemp, Error, TEXT("Can't Start Writer StaticMesh To FBX Memory"));

					if (OnComplete)
					{
						OnComplete(-1);
					}
					return;
				}

				UE_LOG(LogTemp, Log, TEXT("All Process Success!"));
			});
		}else
		{
			Async(EAsyncExecution::Thread, [this, Path, OnComplete]()
			{
				FString FilePath = Path;
				TSharedPtr<FAssimpScene> AiScene;

				TSharedPtr<FAssimpLibrary> AssimpLibrary = MakeShareable(new FAssimpLibrary());
				if (AssimpLibrary->LoadModelAssetFromFile(FilePath, AiScene))
				{
					UE_LOG(LogTemp, Log, TEXT("Assimp import complete!"));
					AiScene->Size = RenderSize;
				
					int Result;
					TSharedPtr<FNNEImageFeatures> NNERuntime = MakeShareable(new FNNEImageFeatures());

					if (!NNERuntime.IsValid())
					{
						UE_LOG(LogTemp, Error, TEXT("NNERuntime Is Valid"));
						return;
					}
					
					bool bIsValid = RunRenderAndClassifier(NNERuntime, AiScene, Result);
					
					AsyncTask(ENamedThreads::GameThread, [bIsValid, Result, OnComplete]()
					{
						if (bIsValid)
						{
							if (OnComplete)
							{
								UE_LOG(LogTemp, Log, TEXT("Classifier Complete!"));
								
								OnComplete(bIsValid ? Result : -1);
							}
						}else
						{
							UE_LOG(LogTemp, Error, TEXT("Classifier failure!"));

							if (OnComplete)
							{
								OnComplete(-1);
							}
						}
					});
				}else
				{
					UE_LOG(LogTemp, Log, TEXT("Assimp import failure!"));

					if (OnComplete)
					{
						OnComplete(-1);
					}
				}
			});
		}
	}

	TArray<std::vector<unsigned char>> FModelClassifierData::GetPixelData()
	{
		return PixelData;
	}

	TArray<FSlateBrush> FModelClassifierData::GetImageRenderBrushes()
	{
		return Brushes;
	}

	FImageRendered& FModelClassifierData::GetImageRenderedAction()
	{
		return OnImageRendered;
	}

	FImageClassified& FModelClassifierData::GetImageClassifiedAction()
	{
		return OnImageClassified;
	}

	FString FModelClassifierData::GetModelPath() const
	{
		return ModelPath;
	}

	void FModelClassifierData::SetRenderCount(int InRenderCount)
	{
		RenderCount = InRenderCount;
	}

	int FModelClassifierData::GetRenderCount() const
	{
		switch (RotateMode)
		{
			case All:
				return RenderCount + (RenderCount % 2 == 0 ? RenderCount - 2 : RenderCount - 1);
			case Azimuth:
				return RenderCount;
			case Elevation:
				return RenderCount;
		}

		return RenderCount;
	}

	void FModelClassifierData::SetRotateMode(ERotateMode InRotateMode)
	{
		RotateMode = InRotateMode;
	}

	FString FModelClassifierData::GetRenderPath() const
	{
		return ImageRenderPath;
	}

	void FModelClassifierData::SetModelPath(const FString& InPath)
	{
		ImageRenderPath = InPath;
	}

	void FModelClassifierData::SetRenderSize(ImageSize InSize)
	{
		RenderSize = InSize;
	}

	ImageSize FModelClassifierData::GetRenderSize()
	{
		return RenderSize;
	}

	int FModelClassifierData::GetResultIndex() const
	{
		return ResultIndex;
	}

	void FModelClassifierData::SetResultClassifier(const int& InResultIndex)
	{
		ResultIndex = InResultIndex;
	}
}
