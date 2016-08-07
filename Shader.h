#pragma once

#include "stdafx.h"

enum ShaderType
{
	kShaderTypeVertex,
	kShaderTypePixel,
};

class Shader
{
public:
	Shader();
	~Shader();

	bool load(ShaderType shaderType, const std::string &filename);

private:
	ShaderType mType;

	VkShaderModule mShaderModule;
};

