#ifndef __ZE_GL_UI_RENDERER__
#define __ZE_GL_UI_RENDERER__

#include "UI/UIRenderer.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>

namespace ZE
{
	class Shader;

	struct Vertex
	{
		struct Position
		{
			GLfloat x;
			GLfloat y;
			GLfloat z;
		} pos;

		struct Color
		{
			GLfloat r;
			GLfloat g;
			GLfloat b;
			GLfloat a;
		} color;

		struct TexCoord
		{
			GLfloat u;
			GLfloat v;
		} texCoord;
	};

	class GL_UIRenderer : public UIRenderer
	{

	public:
		virtual ~GL_UIRenderer();

		virtual void Init(int width, int height) override;
		virtual void ProcessCurrentDrawList() override;
		virtual void Destroy() override;
		virtual bool requestToClose() override;
		virtual UInt32 createRendererTexture(void* pAddress, UInt32 width, UInt32 height, UInt32 channelCount) override;
		virtual void destroyTexture(UInt32 textureHandle);
		virtual void destroyTextures();
		virtual void* getWindowContext() override { return m_window; }

	protected:

		void setDrawData(const UIArray<UIVertex>& vertices);
		void setTextData(const UIArray<UIVertex>& vertices);
		void setInstanceDrawData(const UIArray<UIInstance>& instances);
		void processDrawItem(UIDrawItem* drawItem);

		int m_width;
		int m_height;

		GLFWwindow* m_window;
		Shader* m_drawShader;
		Shader* m_drawInstanceShader;
		Shader* m_drawTexShader;
		Shader* m_textShader;
		Shader* m_drawInstanceTexShader;
		Shader* m_textInstanceShader;

		GLuint VAO_rect_text;
		GLuint VBO_rect_text;

		GLuint VAO_rect;
		GLuint VBO_rect;
		GLuint VBO_instance;
		size_t maxInstanceDraw = 256;

		GLuint VBO_draw;
		GLuint VAO_draw;
		size_t maxDrawSize = 256;

		GLuint VBO_text;
		GLuint VAO_text;
		size_t maxTextSize = 1024;

		UIArray<UInt32> m_textures;
	};
}
#endif
