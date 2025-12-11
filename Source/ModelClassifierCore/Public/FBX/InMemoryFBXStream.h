#pragma once

#include "CoreMinimal.h"
#include "InMemoryFBXStream.h"
#include "ModelClassifierCore/Public/ImplClassBridge.h"

namespace ModelClassifierCore
{
	class FInMemoryFBXStreamImpl;

	class MODELCLASSIFIERCORE_API FInMemoryFBXStream : public TImplClassBridge<FInMemoryFBXStreamImpl>
	{
	public:
		FInMemoryFBXStream();
		~FInMemoryFBXStream()
		{
			UE_LOG(LogTemp, Warning, TEXT("FInMemoryFBXStream has destroy!"));
		}

		bool IsValid();
		
		/** Export buffer as raw bytes after writing FBX */
		TArray<uint8> GetBuffer() const;
		void SetName(const FString& InName);
		FString GetName() const;
	};
}
