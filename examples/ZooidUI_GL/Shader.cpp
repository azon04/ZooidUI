#include "Shader.h"

#include <string>

namespace ZE
{
	Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
	{
		// 1. Retreive the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's biffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			std::cout << vertexPath << std::endl;
			std::cout << fragmentPath << std::endl;
			std::cout << geometryPath << std::endl;
		}

		if (geometryPath)
		{
			std::ifstream gShaderFile;
			gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
				std::cout << geometryPath << std::endl;
			}
		}

		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		const GLchar* gShaderCode = geometryCode.c_str();

		// Compile shader
		GLuint vertex, fragment, geometry;
		GLint success;
		GLchar infoLog[512];

		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			std::cout << vertexPath << std::endl;
		}

		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			std::cout << fragmentPath << std::endl;
		}

		if (geometryPath)
		{
			// Geometry shader
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			// Print compile errors if any
			glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(geometry, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
				std::cout << geometryPath << std::endl;
			}
		}

		// Shader Program
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		if (geometryPath)
		{
			glAttachShader(this->Program, geometry);
		}
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << std::endl;
		}

		if (geometryPath)
		{
			glDeleteShader(geometry);
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	Shader::~Shader()
	{
		glDeleteProgram(Program);
	}

	GLint Shader::getUniformPosition(const char* _varName)
	{
		return glGetUniformLocation(Program, _varName);
	}


	void Shader::setVec2(const char* _varName, UIVector2 _value)
	{
		glUniform2f(getUniformPosition(_varName), _value.x, _value.y);
	}

	void Shader::setFloat(const char* _varName, float _value)
	{
		glUniform1f(getUniformPosition(_varName), _value);
	}

	void Shader::setInt(const char* _varName, int _value)
	{
		glUniform1i(getUniformPosition(_varName), _value);
	}

	void Shader::setVec4(const char* _varName, UIVector4 _value)
	{
		glUniform4f(getUniformPosition(_varName), _value.x, _value.y, _value.z, _value.w);
	}

	void Shader::setBool(const char* _varName, bool _value)
	{
		glUniform1i(getUniformPosition(_varName), _value);
	}

	void Shader::Use()
	{
		glUseProgram(this->Program);
	}
}
