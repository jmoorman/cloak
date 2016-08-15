#include "Shader.h"

#include "CloakUtils.h"

Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::load(ShaderType shaderType, const std::string & filename)
{
	std::vector<char> shaderBytes = CloakUtils::readFile(filename);

	if (shaderBytes.size() == 0)
	{
		return false;
	}
	return true;
}
