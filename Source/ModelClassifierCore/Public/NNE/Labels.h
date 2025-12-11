#pragma once

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FLabels
	{
	public:
		virtual ~FLabels() = default;
		virtual FString GetLabel(int Index);
		virtual int32 Num();
		virtual void Empty();
	};
}