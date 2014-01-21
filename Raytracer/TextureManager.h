#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

#include <string>
#include <vector>
#include <IL/il.h>

class TextureManager
{
private:
	cl::Context context;
	ILuint image;
	std::vector<std::pair<std::string, cl::Image2D>> loadedTextures;

public:
	TextureManager(cl::Context context);
	~TextureManager();

	void releaseAllLoadedTextures();

	cl::Image2D loadTexture(const std::string& _filename);
};