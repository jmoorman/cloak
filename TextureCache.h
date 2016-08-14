#pragma once

#include "stdafx.h"

struct Texture
{
	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
	VkImageView mTextureImageView;
};

class TextureCache
{
public:
	TextureCache();
	~TextureCache();
};

