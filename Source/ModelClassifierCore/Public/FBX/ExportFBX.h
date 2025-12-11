#pragma once
#include "InMemoryFBXStream.h"

namespace ModelClassifierCore
{
	class FStaticMeshCache;
	class FExportFBXPrivate;

	class MODELCLASSIFIERCORE_API FExportFBX
	{
	public:
		static void Writer(TSharedPtr<FStaticMeshCache> MObject, TFunction<void(bool, TSharedPtr<FInMemoryFBXStream>)> CallBack);
		static void SaveFBXToFile(TSharedPtr<FInMemoryFBXStream> InMemoryStream, FString SavePath);
	};
}