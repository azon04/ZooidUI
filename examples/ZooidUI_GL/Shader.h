#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h> // Include glew to get all the required OpenGL Headers

#include "UI/ZooidUI.h"

namespace ZE
{
	class Shader
	{
	public:
		// The program ID
		GLuint Program;

		// Constructor reads and builds the shader
		Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);

		virtual ~Shader();

		GLint getUniformPosition(const char* _varName);

		void setVec2(const char* _varName, UIVector2 _value);
		void setFloat(const char* _varName, float _value);
		void setInt(const char* _varName, int _value);
		void setVec4(const char* _varName, UIVector4 _value);
		void setBool(const char* _varName, bool _value);

		// Use the program
		void Use();
	};
}
#endif

