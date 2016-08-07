#include "Shader.h"

#include "OcelotUtils.h"

Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::load(ShaderType shaderType, const std::string & filename)
{
	std::vector<char> shaderBytes = OcelotUtils::readFile(filename);

	if (shaderBytes.size() == 0)
	{
		return false;
	}
	return true;
}
