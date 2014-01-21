#include "TextureManager.h"


TextureManager::TextureManager(cl::Context _context)
{
	context = _context;
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	image = ilGenImage();
	ilBindImage(image);
}

TextureManager::~TextureManager()
{
	ilDeleteImage(image);
	ilShutDown();
}

void TextureManager::releaseAllLoadedTextures()
{
	loadedTextures.clear();
}

cl::Image2D TextureManager::loadTexture(const std::string& _filename)
{
	for (auto &texture : loadedTextures)
	{
		if(texture.first == _filename)
			return texture.second;
	}
	ilLoadImage(_filename.c_str());

	static const std::pair<int, int> formatOrders[] =
	{
		std::make_pair(IL_RGBA, CL_RGBA),
		std::make_pair(IL_RGB, CL_RGB),
	};

	static const std::pair<int, int> formatTypes[] =
	{
		std::make_pair(IL_FLOAT, CL_FLOAT),
		std::make_pair(IL_UNSIGNED_BYTE, CL_UNORM_INT8),
	};

	cl::ImageFormat format(0, 0);
	ILint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
	ILint imageType = ilGetInteger(IL_IMAGE_TYPE);
	for (auto& f : formatOrders)
	{
		if (f.first == imageFormat)
		{
			format.image_channel_order = f.second;
			break;
		}
	}

	for (auto& f : formatTypes)
	{
		if (f.first == imageType)
		{
			format.image_channel_data_type = f.second;
			break;
		}
	}

	if (format.image_channel_data_type == 0 || format.image_channel_order == 0)
	{
		throw std::exception("Invalid image format");
	}

	ILubyte* data = ilGetData();
	ILint width = ilGetInteger(IL_IMAGE_WIDTH);
	ILint height = ilGetInteger(IL_IMAGE_HEIGHT);

	cl_int err = CL_SUCCESS;
	cl::Image2D texture(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, format, ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetData(), &err);

	if (err != CL_SUCCESS)
	{
		throw std::exception("Hey! Listen!");
	}

	loadedTextures.push_back(std::make_pair(_filename, texture));

	return texture;
}