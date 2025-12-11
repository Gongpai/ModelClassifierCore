#include "Utilities/StringHelper.h"
#include <chrono>
#include <random>
#include <string>

namespace ModelClassifierCore
{
	FString FStringHelper::GenerateRandomStringWithDate(size_t length)
	{
		const std::string chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789";
		
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, chars.size() - 1);

		std::string randomPart;
		randomPart.reserve(length);
		for (size_t i = 0; i < length; ++i) {
			randomPart += chars[distrib(gen)];
		}

		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm tm{};
#ifdef _WIN32
		localtime_s(&tm, &now_time);
#else
		localtime_r(&now_time, &tm);
#endif

		std::ostringstream dateStream;
		dateStream << std::put_time(&tm, "%Y%m%d");

		std::string randomResult = randomPart + "_" + dateStream.str();

		return FString(randomResult.c_str());
	}
}
