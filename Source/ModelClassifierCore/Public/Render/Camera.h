#pragma once
#include <glm/glm.hpp>
#include "glm/vec3.hpp"

namespace ModelClassifierCore
{
	struct MODELCLASSIFIERCORE_API Camera
	{
		glm::vec3 cameraPos;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	enum MODELCLASSIFIERCORE_API ERotateMode
	{
		All,
		Azimuth,
		Elevation
	};
}
