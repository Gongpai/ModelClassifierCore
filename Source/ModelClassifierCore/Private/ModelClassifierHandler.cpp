#include "ModelClassifierHandler.h"

#include "Utilities/BoyerMooreMajority.h"

namespace ModelClassifierCore
{
	FModelClassifierHandler::FModelClassifierHandler(FModelClassifierCoreModule* InCore, FString Name, EFileType Type,  FString ModelSavePath)
	{
		Core = InCore;
		SetName(Name);
		SetFileType(Type, ModelSavePath);
	}

	bool FModelClassifierHandler::RunRenderAndClassifier(TSharedPtr<FNNEImageFeatures> NNERuntime, TSharedPtr<FAssimpScene> AiScene, int& OutResult)
	{
		TSharedPtr<FRenderMesh> RenderMesh = MakeShareable(new FRenderMesh());
		TArray<int32> Predicts;
		RenderMesh.Get()->SetAssimpScene(AiScene);
		RenderMesh->SetImageFormat(ImageFormat);
		RenderMesh.Get()->Render(RenderCount, RotateMode, [&NNERuntime, &Predicts, this](std::vector<unsigned char> OutPixel, std::vector<float> OutFloatPixels)
		{    
			if (OutFloatPixels.size() > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Render Complete!"));
				
				AsyncTask(ENamedThreads::GameThread, [this, OutPixel]()
				{
					PixelData.Add(OutPixel);
					
					UE_LOG(LogTemp, Log, TEXT("Trigger OnImageRendered"));
					if (OnImageRendered.IsBound())
					{
						UE_LOG(LogTemp, Log, TEXT("OnImageRendered Can Trigger"));
						OnImageRendered.Broadcast(OutPixel);
					}
				});

				if (NNERuntime.IsValid())
				{
					NNERuntime->SetClassifierNNEModelData(Core->GetClassifierNNEModelData().Get());
					NNERuntime->SetImageEncoderDataAsset(Core->GetImageEncoderDataAsset().Get());
					NNERuntime->SetTextFeaturesAsset(Core->GetTextFeaturesAsset().Get());
					NNERuntime->SetInputData(OutPixel, GetRenderSize(), true);
					NNERuntime->SetRuntimeType(ENNEInstanceType::CPU);
					NNERuntime->Initialize();
					
					int OutResultIndex = -1;
					
					if (NNERuntime->RunClassifyRenderedImage(GetRenderSize(), OutResultIndex))
					{
						UE_LOG(LogTemp, Log, TEXT("Run classify complete!"));
						
						AsyncTask(ENamedThreads::GameThread, [this, OutResultIndex]()
						{
							if (OnImageClassified.IsBound())
							{
								OnImageClassified.Broadcast(OutResultIndex);
							}
						});
					}else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to run classify"));
					}
					
					Predicts.Add(OutResultIndex);
				}else
				{
					UE_LOG(LogTemp, Error, TEXT("NNE Model is invalid"));
				}
			}else
			{
				UE_LOG(LogTemp, Error, TEXT("Render failure!"));
			}
		});

		OutResult = FBoyerMooreMajority::FindMajorityOrFirst<int32>(Predicts);
		return true;
	}

	void FModelClassifierHandler::RunClassifier(TFunction<void(int)> OnComplete)
	{
		StartClassifier(ModelPath, FileType, OnComplete);
	}

	void FModelClassifierHandler::SetName(const FString& InName)
	{
		Name = InName;
	}

	FString FModelClassifierHandler::GetName() const
	{
		return Name;
	}

	void FModelClassifierHandler::SetFileType(EFileType Type, FString InPath)
	{
		FileType = Type;
		ModelPath = InPath;
	}

	EFileType FModelClassifierHandler::GetFileType() const
	{
		return FileType;
	}

	FString FModelClassifierHandler::GetFileTypeString(EFileType InFileType)
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

	void FModelClassifierHandler::StartClassifier(FString Path, EFileType Type, TFunction<void(int)> OnComplete)
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
									
									AsyncTask(ENamedThreads::GameThread, [this, bIsComplete, OnComplete, &Result]()
									{
										if (bIsComplete)
										{
											UE_LOG(LogTemp, Log, TEXT("Classifier Complete!"));

											if (OnComplete)
											{
												ResultIndex = bIsComplete ? Result : -1;
												
												OnComplete(ResultIndex);
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
					
					AsyncTask(ENamedThreads::GameThread, [this, bIsValid, Result, OnComplete]()
					{
						if (bIsValid)
						{
							if (OnComplete)
							{
								UE_LOG(LogTemp, Log, TEXT("Classifier Complete!"));
								ResultIndex = bIsValid ? Result : -1;
								
								OnComplete(ResultIndex);
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

	TArray<std::vector<unsigned char>> FModelClassifierHandler::GetPixelData()
	{
		return PixelData;
	}

	FImageRendered& FModelClassifierHandler::GetImageRenderedAction()
	{
		return OnImageRendered;
	}

	void FModelClassifierHandler::ClearImageRenderedAction()
	{
		OnImageRendered.Clear();
	}

	FImageClassified& FModelClassifierHandler::GetImageClassifiedAction()
	{
		return OnImageClassified;
	}

	void FModelClassifierHandler::ClearImageClassifiedAction()
	{
		OnImageClassified.Clear();
	}

	FString FModelClassifierHandler::GetModelPath() const
	{
		return ModelPath;
	}

	void FModelClassifierHandler::SetRenderCount(int InRenderCount)
	{
		RenderCount = InRenderCount;
	}

	int FModelClassifierHandler::GetRenderCount() const
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

	void FModelClassifierHandler::SetRotateMode(ERotateMode InRotateMode)
	{
		RotateMode = InRotateMode;
	}

	FString FModelClassifierHandler::GetRenderPath() const
	{
		return ImageRenderPath;
	}

	void FModelClassifierHandler::SetModelPath(const FString& InPath)
	{
		ImageRenderPath = InPath;
	}

	void FModelClassifierHandler::SetRenderSize(ImageSize InSize)
	{
		RenderSize = InSize;
	}

	ImageSize FModelClassifierHandler::GetRenderSize()
	{
		return RenderSize;
	}

	int FModelClassifierHandler::GetResultIndex() const
	{
		return ResultIndex;
	}

	void FModelClassifierHandler::SetImageFormat(EImageFormat InFormat)
	{
		ImageFormat = InFormat;
	}
}
