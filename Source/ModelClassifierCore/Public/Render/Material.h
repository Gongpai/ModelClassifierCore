#pragma once
#include "glad/glad.h"

namespace ModelClassifierCore
{
	struct Material
	{
		GLuint albedoTex = 0;
		GLuint normalTex = 0;
		GLuint metallicTex = 0;
		GLuint roughnessTex = 0;
		GLuint aoTex = 0;
	};
}
