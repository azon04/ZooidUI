#include "GL_UIRenderer.h"

#include "UI/ZooidUI.h"
#include "Common/Timer.h"
#include "Shader.h"

#include <iostream>
#include <assert.h>

#if WIN32 || WIN64
extern "C"
{
#include <windows.h>
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

namespace ZE
{

	GL_UIRenderer::~GL_UIRenderer()
	{

	}

	void GL_UIRenderer::Init(int width, int height)
	{
		// Init Library
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		// Setup HW MSAA
		glfwWindowHint(GLFW_SAMPLES, 4);

		// Create Window
		m_window = glfwCreateWindow(width, height, "Zooid UI Sample", nullptr, nullptr);
		if (m_window == nullptr)
		{
			glfwTerminate();
			std::cout << "Failed to create GLFW Window" << std::endl;
			return;
		}
		glfwMakeContextCurrent(m_window);

		// Init Glew
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			glfwTerminate();
			std::cout << "Failed to init GLEW: " << glewGetErrorString(err) << std::endl;
			return;
		}

		// Viewport setup
		glfwGetFramebufferSize(m_window, &width, &height);

		// Setup Shaders
		m_drawShader = UINEW(Shader("Shaders/BaseShapeShader.vs", "Shaders/BaseShapeShader_Color.frag"));
		m_drawInstanceShader = UINEW(Shader("Shaders/BaseShapeShader_Instance.vs", "Shaders/BaseShapeShader_Color_Instance.frag"));
		m_drawTexShader = UINEW(Shader("Shaders/BaseShapeShader.vs", "Shaders/BaseShapeShader_Texture.frag"));
		m_drawInstanceTexShader = UINEW(Shader("Shaders/BaseShapeShader_Texture_Instance.vs", "Shaders/BaseShapeShader_Texture_Instance.frag"));
		m_textShader = UINEW(Shader("Shaders/BaseTextShader.vs", "Shaders/BaseTextShader.frag"));
		m_textInstanceShader = UINEW(Shader("Shaders/BaseTextShader_Instance.vs", "Shaders/BaseTextShader_Instance.frag"));

		// Initialize Buffer
		
		// Rect Buffer
		glGenBuffers(1, &VBO_rect);
		glGenVertexArrays(1, &VAO_rect);

		UIVertex rectArray[6] = { 
								{ UIVector2{ 1.0f, 0.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } }, 
								{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
								{ UIVector2{ 0.0f, 1.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } } 
								};
		
		
		glBindVertexArray(VAO_rect);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_rect);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rectArray), rectArray, GL_STATIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(0 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);

		// Tex Coord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		// Instance Data
		glGenBuffers(1, &VBO_instance);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_instance);
		glBufferData(GL_ARRAY_BUFFER, maxInstanceDraw * sizeof(UIDrawInstance), nullptr, GL_DYNAMIC_DRAW);
		
		// Position
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)0);
		glEnableVertexAttribArray(3);

		// Dimension + Radius
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(4);

		// Color
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(5);

		// UV Coord + Dimension
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(10 * sizeof(GLfloat)));
		glEnableVertexAttribArray(6);
		
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Rect Text Instance Buffer : Y Flipped
		glGenBuffers(1, &VBO_rect_text);
		glGenVertexArrays(1, &VAO_rect_text);

		UIVertex rectTextArray[6] = { { UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ UIVector2{ 1.0f, 0.0f }, 0.0f, UIVector2{ 1.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ UIVector2{ 0.0f, 0.0f }, 0.0f, UIVector2{ 0.0f, 1.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ UIVector2{ 1.0f, 1.0f }, 0.0f, UIVector2{ 1.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ UIVector2{ 0.0f, 1.0f }, 0.0f, UIVector2{ 0.0f, 0.0f }, UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f } } };


		glBindVertexArray(VAO_rect_text);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_rect_text);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rectTextArray), rectTextArray, GL_STATIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(0 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);

		// Tex Coord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		// Instance Data
		glBindBuffer(GL_ARRAY_BUFFER, VBO_instance);

		// Position
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)0);
		glEnableVertexAttribArray(3);

		// Dimension + Radius
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(4);

		// Color
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(5);

		// UV Coord + Dimension
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(UIDrawInstance), (GLvoid*)(10 * sizeof(GLfloat)));
		glEnableVertexAttribArray(6);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Draw Buffer
		glGenBuffers(1, &VBO_draw);
		glGenVertexArrays(1, &VAO_draw);

		glBindVertexArray(VAO_draw);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_draw);
		glBufferData(GL_ARRAY_BUFFER, maxDrawSize * sizeof(UIVertex), nullptr, GL_DYNAMIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*) (0 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);

		// Tex Coord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Text Buffer
		glGenBuffers(1, &VBO_text);
		glGenVertexArrays(1, &VAO_text);

		glBindVertexArray(VAO_text);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
		glBufferData(GL_ARRAY_BUFFER, maxTextSize * sizeof(UIVertex), nullptr, GL_DYNAMIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(0 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);

		// Tex Coord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (GLvoid*)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);		

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glfwGetFramebufferSize(m_window, &width, &height);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		
		glViewport(0, 0, width, height);

		m_width = width;
		m_height = height;
	}

	void GL_UIRenderer::ProcessCurrentDrawList()
	{
		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		UIArray<UIDrawItem*> secondPass;
		for (int i = 0; i < m_drawList->itemCount(); i++)
		{
			UIDrawItem* drawItem = m_drawList->getDrawItem(i);
			if (drawItem->getLayer() > 0)
			{
				secondPass.push_back(drawItem);
				continue;
			}
			processDrawItem(drawItem);
		}

		for (unsigned int i = 0; i < secondPass.size(); i++)
		{
			UIDrawItem* drawItem = secondPass[i];
			processDrawItem(drawItem);
		}
		
		glfwSwapBuffers(m_window);
		maskCount = 0;
	}

	void GL_UIRenderer::processDrawItem(UIDrawItem* drawItem)
	{
		bool isFont = drawItem->getTextureHandle() && drawItem->isFont();
		bool isUsingRect = drawItem->isUsingRectInstance();
		bool isInstance = isUsingRect;
		GLuint VAO = isUsingRect ? VAO_rect : VAO_draw;

		if (drawItem->isDrawMask())
		{
			if (drawItem->getDrawMask() == DRAW_MASK_PUSH)
			{
				pushMask();
				glStencilFunc(GL_ALWAYS, maskCount, 0xFF);
				glStencilMask(0xFF);
				glDisable(GL_DEPTH_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			}
			else
			{
				glStencilFunc(GL_ALWAYS, maskCount, 0xFF);
				glStencilMask(0xFF);
				glDisable(GL_DEPTH_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
			}
		}
		else if (maskCount)
		{
			glStencilFunc(GL_EQUAL, maskCount, 0xFF);
			glStencilMask(0x00);
			glEnable(GL_DEPTH_TEST);
		}

		Shader* shader = m_drawShader;
		if (drawItem->getTextureHandle() > 0)
		{
			if (drawItem->isFont())
			{
				VAO = isUsingRect ? VAO_rect_text : VAO_text;
				shader = isUsingRect ? m_textInstanceShader : m_textShader;
				shader->Use();
			}
			else
			{
				shader = isUsingRect ? m_drawInstanceTexShader : m_drawTexShader;
				shader->Use();
			}
			shader->setInt("InTexture", 0);
			glBindTexture(GL_TEXTURE_2D, drawItem->getTextureHandle());
			glActiveTexture(GL_TEXTURE0);
		}
		else if (isUsingRect)
		{
			VAO = VAO_rect;
			shader = m_drawInstanceShader;
			shader->Use();
		}
		else
		{
			shader->Use();
			shader->setFloat("roundness", drawItem->getRoundness());
			shader->setVec2("shapeDimension", drawItem->getDimension());
		}

		shader->setVec2("screenDimension", UIVector2{ (Float32)m_width, (Float32)m_height });

		shader->setBool("bCrop", drawItem->isCrop());

		if (drawItem->isCrop())
		{
			shader->setVec4("CropBox", drawItem->getCropDimension());
		}

		if (drawItem->getVertices().size() > 0)
		{
			if (isFont)
			{
				setTextData(drawItem->getVertices());
			}
			else
			{
				setDrawData(drawItem->getVertices());
			}
		}

		if (drawItem->getInstances().size() > 0)
		{
			setInstanceDrawData(drawItem->getInstances());
		}

		if (!isInstance)
		{
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, drawItem->getVertices().size());
			glBindVertexArray(0);
		}
		else
		{
			// Draw Instances
			glBindVertexArray(VAO);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, drawItem->getInstances().size());
			glBindVertexArray(0);
		}

		if (drawItem->isDrawMask())
		{
			if (drawItem->getDrawMask() == DRAW_MASK_POP)
			{
				popMask();
			}
		}
	}

	void GL_UIRenderer::pushMask()
	{
		maskCount++;
		if (maskCount == 1)
		{
			glEnable(GL_STENCIL_TEST);
		}
	}

	void GL_UIRenderer::popMask()
	{
		assert(maskCount > 0);
		maskCount--;
		if (maskCount == 0)
		{
			glDisable(GL_STENCIL_TEST);
		}
	}

	void GL_UIRenderer::Destroy()
	{
		// Deleting Shaders and Buffer
		UIFREE(m_drawShader);
		UIFREE(m_drawInstanceShader);
		UIFREE(m_drawTexShader);
		UIFREE(m_drawInstanceTexShader);
		UIFREE(m_textShader);
		UIFREE(m_textInstanceShader);

		glDeleteVertexArrays(1, &VAO_draw);
		glDeleteBuffers(1, &VBO_draw);
		glDeleteVertexArrays(1, &VAO_text);
		glDeleteBuffers(1, &VBO_text);
		glDeleteVertexArrays(1, &VAO_rect);
		glDeleteBuffers(1, &VBO_rect);
		glDeleteVertexArrays(1, &VAO_rect_text);
		glDeleteBuffers(1, &VBO_rect_text);
		glDeleteBuffers(1, &VBO_instance);


		glfwTerminate();
	}

	bool GL_UIRenderer::requestToClose()
	{
		return glfwWindowShouldClose(m_window) != 0;
	}

	ZE::UInt32 GL_UIRenderer::createRendererTexture(void* pAddress, UInt32 width, UInt32 height , UInt32 channelCount)
	{
		GLuint textureHandle;

		glGenTextures(1, &textureHandle);

		glBindTexture(GL_TEXTURE_2D, textureHandle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		GLenum imageFormat = channelCount == 4 ? GL_RGBA : channelCount == 3 ? GL_RGB : channelCount == 2 ? GL_RG : GL_RED;
		glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, pAddress);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureHandle;
	}

	void GL_UIRenderer::destroyTexture(UInt32 textureHandle)
	{
		auto iter = std::find(m_textures.begin(), m_textures.end(), textureHandle);
		if (iter != m_textures.end())
		{
			glDeleteTextures(1, &textureHandle);
			m_textures.erase(iter);
		}
	}

	void GL_UIRenderer::destroyTextures()
	{
		glDeleteTextures(m_textures.size(), m_textures.data());
	}

	void GL_UIRenderer::resizeWindow(int width, int height)
	{
		m_width = width;
		m_height = height;
		glViewport(0, 0, width, height);
	}

	void GL_UIRenderer::setDrawData(const UIArray<UIVertex>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO_draw);
		if (maxDrawSize < vertices.size())
		{
			while (maxDrawSize < vertices.size())
			{
				maxDrawSize *= 2;
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxDrawSize, nullptr, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void GL_UIRenderer::setTextData(const UIArray<UIVertex>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
		if (maxTextSize < vertices.size())
		{
			while (maxTextSize < vertices.size())
			{
				maxTextSize *= 2;
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex) * maxTextSize, nullptr, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(UIVertex) * vertices.size(), vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void GL_UIRenderer::setInstanceDrawData(const UIArray<UIDrawInstance>& instances)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO_instance);
		if (maxInstanceDraw < instances.size())
		{
			while (maxInstanceDraw < instances.size())
			{
				maxInstanceDraw *= 2;
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(UIDrawInstance) * maxInstanceDraw, nullptr, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(UIDrawInstance) * instances.size(), instances.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	

	// Specific Platform Implementaion
	ZE::UIRenderer* UI::Platform::CreateRenderer()
	{
		return UINEW(GL_UIRenderer);
	}
}