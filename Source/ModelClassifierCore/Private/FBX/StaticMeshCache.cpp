#include "FBX/StaticMeshCache.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "MaterialShared.h"
#include "MaterialCompiler.h"
#include "StaticMeshAttributes.h"

namespace ModelClassifierCore
{
	FStaticMeshCache::~FStaticMeshCache()
	{
		if (MeshDescription.IsValid())
		{
			MeshDescription.Reset();
		}

		if (StaticMesh.IsValid())
		{
			StaticMesh.Reset();
		}
	}

	void FStaticMeshCache::LoadStaticMesh(FString Path, TFunction<void(bool)> CallBack)
	{
		if (!CurrentAssetPath.IsEmpty() && CurrentAssetPath == Path && StaticMesh.IsValid())
		{
			bool isValid = true;
			UE_LOG(LogTemp, Log, TEXT("StaticMesh (%s) is loaded."), *StaticMesh->GetName());

			if (!MeshDescription.IsValid())
			{
				isValid = ConvertMeshDescription();
			}
			
			CallBack(StaticMesh.IsValid() && MeshDescription.IsValid() && isValid);
			UE_LOG(LogTemp, Warning, TEXT("Mesh is valid: %s"), *(StaticMesh.IsValid() && MeshDescription.IsValid() && isValid ? FString("True") : FString("False")));
			
			return;
		}
		
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		FSoftObjectPath MeshPath(Path);
	
		Streamable.RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([MeshPath, CallBack, Path, this]()
		{
			TWeakObjectPtr<UStaticMesh> Mesh = Cast<UStaticMesh>(MeshPath.ResolveObject());
			
			if (!Mesh.IsValid())
			{
				Mesh = Cast<UStaticMesh>(MeshPath.TryLoad());
			}

			if (Mesh.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Loaded StaticMesh: %s"), *Mesh->GetName());

				bool isValid = true;
				StaticMesh = Mesh;
				CurrentAssetPath = Path;
				isValid = ConvertMeshDescription();

				if (CallBack)
				{
					CallBack(isValid);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load StaticMesh at path: %s"), *MeshPath.ToString());
				if (CallBack)
				{
					CallBack(false);
				}
			}
		}));
	}

	bool FStaticMeshCache::ConvertMeshDescription()
	{
		if (StaticMesh.IsValid())
		{
			MeshDescription = MakeShareable(StaticMesh->GetMeshDescription(LODIndex));
		
			FStaticMeshAttributes Attributes(*MeshDescription);
			Attributes.Register();

			return true;
		}

		return false;
	}

	TArray<TMap<FString, TWeakObjectPtr<UTexture>>> FStaticMeshCache::GetTextures()
	{
		TArray<TMap<FString, TWeakObjectPtr<UTexture>>> Textures;

		if (StaticMesh == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("StaticMesh is not valid."));
			return Textures;
		}

		const int32 NumSections = StaticMesh->GetNumSections(0);
		UE_LOG(LogTemp, Log, TEXT("NumSections: %d"), NumSections);
		for (int i = 0; i < NumSections; i++)
		{
			int32 MaterialIndex = StaticMesh->GetSectionInfoMap().Get(0, i).MaterialIndex;
			UMaterial* RawMaterial = Cast<UMaterial>(StaticMesh->GetMaterial(MaterialIndex));
			TMap<FString, TWeakObjectPtr<UTexture>> TextureChannels;
			
			for (const auto& Channel : Channels)
			{
				FExpressionInput* Input = RawMaterial->GetExpressionInputForProperty(Channel.Property);
				
				if (!Input || !Input->Expression)
				{
					UE_LOG(LogTemp, Warning, TEXT("Input or Expression is not valid."));
					continue;
				}
				
				if (Input && Input->Expression)
				{
					UE_LOG(LogTemp, Log, TEXT("Texture: %s | %s"), Channel.Name, *Input->Expression->GetName());

					if (auto* FuncCall = Cast<UMaterialExpressionMaterialFunctionCall>(Input->Expression))
					{
						UE_LOG(LogTemp, Warning, TEXT("Material Expression Is Material FunctionCall."));
						continue;
					}
					else
					{
						if (auto* TextureExpr = Cast<UMaterialExpressionTextureSample>(Input->Expression))
						{
							if (TextureExpr->Texture)
							{
								UE_LOG(LogTemp, Log, TEXT("Texture: %s"), *TextureExpr->Texture->GetName());
								TextureChannels.Add(Channel.Name, TextureExpr->Texture);
							}
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Input or Expression is not valid."));
					UE_LOG(LogTemp, Warning, TEXT("%s is not found."), Channel.Name);
				}
			}

			if (TextureChannels.Num() > 0)
			{
				Textures.Add(TextureChannels);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Texture Count: %d"), Textures.Num());
		return Textures;
	}

	TArray<FLinearColor> FStaticMeshCache::GetColors()
	{
		TArray<FLinearColor> Colors;

		if (StaticMesh == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("StaticMesh is not valid."));
			return Colors;
		}

		const int32 NumSections = StaticMesh->GetNumSections(0);
		UE_LOG(LogTemp, Log, TEXT("NumSections: %d"), NumSections);
		for (int i = 0; i < NumSections; i++)
		{
			
		}

		UE_LOG(LogTemp, Log, TEXT("Color Count: %d"), Colors.Num());
		return Colors;
	}

	TSharedPtr<FMeshDescription> FStaticMeshCache::GetMeshDescription()
	{
		return MeshDescription;
	}

	bool FStaticMeshCache::TryGetMeshDescription(TSharedPtr<FMeshDescription>& OutMeshDescription)
	{
		if (MeshDescription.IsValid())
		{
			OutMeshDescription = MeshDescription;
			return true;
		}else
		{
			return false;
		}
	}


	TWeakObjectPtr<UStaticMesh> FStaticMeshCache::GetStaticMesh()
	{
		return StaticMesh;
	}
}
