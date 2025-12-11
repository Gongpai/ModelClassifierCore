#pragma once

namespace ModelClassifierCore
{
	struct MODELCLASSIFIERCORE_API ImageSize
	{
		ImageSize(): width(0), height(0)
		{
		}

		ImageSize(int Width, int Height): width(Width), height(Height)
		{
		}
		
		int width;
		int height;
	};

	template <typename T1, typename T2>
	struct MODELCLASSIFIERCORE_API MinMax
	{
	public:
		T1 Min = T1();
		T2 Max = T2();
		MinMax() = default;
		MinMax(T1 InMin, T2 InMax) : Min(InMin), Max(InMax) {}
	};
}