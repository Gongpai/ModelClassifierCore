#include "NNE/TextFeatures/TextFeatures.h"
#include "Serialization/Csv/CsvParser.h"

namespace ModelClassifierCore
{
	TArray<FString> FTextFeatures::ParseNames(const FString& Input)
	{
		TArray<FString> Parts;
		TArray<FString> Aliases;
		Input.ParseIntoArray(Parts, TEXT(","), true);
            
		for (FString& Part : Parts)
		{
			Part.TrimStartAndEndInline();
			Aliases.Add(Part);
		}
		
		return Aliases;
	}

	TArray<FString> FTextFeatures::ConvertImageNetToStringArray(FString FileContent, const bool bIsPrimaryNameOnly)
	{
		TArray<FString> Labels;
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			bool bIsHasField = true;
			int i = 0;
			while (bIsHasField)
			{
				FString Key = FString::FromInt(i);
                
				bIsHasField = JsonObject->HasField(Key);
				if (bIsHasField)
				{
					FString FullName = JsonObject->GetStringField(Key);
					TArray<FString> Names;
					
					Names = ParseNames(*FullName);
					UE_LOG(LogTemp, Log, TEXT("Name: %s | At : %d | ParseNames Num : %d"), *FullName, i, Names.Num());
					
					if (Names.Num() == 0)
					{
						i++;
						continue;
					}
					
					if (bIsPrimaryNameOnly)
					{
						Labels.Add(Names[0]);
					}else
					{
						Labels.Append(Names);
					}
				}
				
				i++;
			}
            
			UE_LOG(LogTemp, Display, TEXT("Successfully loaded %d ImageNet labels"), Labels.Num());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
		}
		
		return Labels;
	}

	TArray<FString> FTextFeatures::ConvertCSVToStringArray(FString FileContent, const bool bIsPrimaryNameOnly)
	{
		TArray<FString> Labels;
		FCsvParser Parser(FileContent);
		const TArray<TArray<const TCHAR*>>& Rows = Parser.GetRows();
		
		for (const auto& Row : Rows)
		{
			TArray<FString> ParsedRow;
			for (const TCHAR* Field : Row)
			{
				TArray<FString> Names;
				Names = ParseNames(FString(Field));
					
				if (Names.Num() == 0)
				{
					continue;
				}
				
				if (bIsPrimaryNameOnly)
				{
					Labels.Add(Names[0]);
				}else
				{
					Labels.Append(Names);
				}
			}
		}
		
		return Labels;
	}

	void FTextFeatures::GenerateTextFeatures(FString VocabPath, FString MergesPath, UClassifierNNEModel* ClassifierNNEModel, UTextFeaturesAsset* TextFeaturesAsset, TFunction<void()> OnComplete)
	{
		if (TextFeaturesAsset->Labels.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("No labels found"));
			return;
		}
		
		/*Async(EAsyncExecution::Thread, [OnComplete, VocabPath, MergesPath, ClassifierNNEModel, TextFeaturesAsset]()
		{
			TSharedPtr<FCLIPTokenizer> TextEncoder = MakeShared<FCLIPTokenizer>();
			TextEncoder->Initialize(VocabPath, MergesPath);
		
			TArray<FString> Labels;
			TArray<FString> Templates = {
				"a photo of a {}",
				"a picture of a {}",
				"an image of a {}",
				"a {} in the photo",
				'a photo of the {}.',
			};
			for (const FString& Label : TextFeaturesAsset->Labels)
			{
				for (const FString& Template : Templates)
				{
					FString CLIPLabel = Template.Replace(TEXT("{}"), *Label);
					Labels.Add(CLIPLabel);
				}
			}
		
			TArray<FTextFeatureVector> TextFeatures;
			for (const FString& Label : Labels)
			{
				TArray<float> Features = TextEncoder->Encode(Label);
        
				if (Features.Num() > 0)
				{
					FTextFeatureVector FeatureVec(Features, Label);
					TextFeatures.Add(FeatureVec);
				}
			}
		});*/
	}

	void FTextFeatures::GenerateLabel(FString LabelClassPath, FileFormat Format, const bool bIsPrimaryNameOnly, UTextFeaturesAsset* TextFeaturesAsset, TFunction<void()> OnComplete)
	{
		Async(EAsyncExecution::Thread, [OnComplete, LabelClassPath, Format, bIsPrimaryNameOnly, TextFeaturesAsset]()
		{
			TArray<FString> Labels;
			FString FileContent;
		
			switch (Format)
			{
			case TEXT:
				{
					if (FFileHelper::LoadFileToStringArray(Labels, *LabelClassPath))
					{
						UE_LOG(LogTemp, Log, TEXT("Completed to load file"));
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to load file"));
					}
					break;
				}
				case IMAGE_NET_JSON:
				{
					if (FFileHelper::LoadFileToString(FileContent, *LabelClassPath))
					{
						Labels = ConvertImageNetToStringArray(FileContent, bIsPrimaryNameOnly);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to load file"));
					}
					break;
				}
				case CSV:
				{
					if (FFileHelper::LoadFileToString(FileContent, *LabelClassPath))
					{
						Labels = ConvertCSVToStringArray(FileContent, bIsPrimaryNameOnly);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to load file"));
					}
					break;
				}
			}
			
			AsyncTask(ENamedThreads::GameThread, [Labels, OnComplete, TextFeaturesAsset]()
			{
				TextFeaturesAsset->Labels = Labels;
				
				if (OnComplete)
				{
					OnComplete();
				}
			});
		});
	}

	void FTextFeatures::LoadCLIPTokenizer(FString FilePath, UTextFeaturesAsset* TextFeaturesAsset,
		TFunction<void()> OnComplete)
	{
		
	}
}
