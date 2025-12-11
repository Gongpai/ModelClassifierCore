#pragma once

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FStaticMeshCache
	{
		TWeakObjectPtr<UStaticMesh> StaticMesh;
		FString CurrentAssetPath;
		TSharedPtr<FMeshDescription> MeshDescription;
		
	public:
		~FStaticMeshCache();
		void LoadStaticMesh(FString Path, TFunction<void(bool)> CallBack);
		bool ConvertMeshDescription();
		TArray<TMap<FString, TWeakObjectPtr<UTexture>>> GetTextures();
		TArray<FLinearColor> GetColors();

		TSharedPtr<FMeshDescription> GetMeshDescription();
		bool TryGetMeshDescription(TSharedPtr<FMeshDescription>& OutMeshDescription);
		TWeakObjectPtr<UStaticMesh> GetStaticMesh();

		int32 LODIndex = 0;
	};

	inline struct ChannelInfo
	{
		EMaterialProperty Property;
		const TCHAR* Name;
	} Channels[] = {
		{ MP_BaseColor, TEXT("BaseColor") },
		{ MP_Normal, TEXT("Normal") },
		{ MP_Roughness, TEXT("Roughness") }
	};
}
