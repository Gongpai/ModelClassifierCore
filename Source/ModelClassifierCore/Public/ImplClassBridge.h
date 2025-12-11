#pragma once

#include <any>

namespace ModelClassifierCore
{
	template<typename T1>
	class TImplClassBridge
	{
	protected:
		TSharedPtr<T1> Impl;
		std::any ClassBridge;
	
	public:
		TImplClassBridge() {};

		template<typename C>
		C GetClassBridge() const
		{
			return std::any_cast<C>(ClassBridge);
		}

		template<typename C>
		bool TryGetClassBridge(C& Out)
		{
			if (C* Ptr = std::any_cast<C>(&ClassBridge))
			{
				if (Ptr != nullptr)
				{
					Out = *Ptr;
					return true;
				}
			}
	
			return false;
		}
	};
}
