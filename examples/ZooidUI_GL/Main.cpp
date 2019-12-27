#include "UI/ZooidUI.h"
#include "Common/Timer.h"
#include "GL_UIRenderer.h"
#include <iostream>
#include <cstring>

double mouseX = 0.0f;
double mouseY = 0.0f;
ZE::EButtonState buttonState = ZE::BUTTON_UP;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// ignore Shift/Alt/Ctrl
	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT
		|| key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT
		|| key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		return;
	}

	// Convert some key
	if (key == GLFW_KEY_BACKSPACE)
	{
		key = ZOOID_KEY_BACKSPACE;
	}
	else if (key == GLFW_KEY_ENTER)
	{
		key = ZOOID_KEY_ENTER;
	}
	else if (key == GLFW_KEY_DELETE)
	{
		key = ZOOID_KEY_DELETE;
	}
	else if (key == GLFW_KEY_LEFT)
	{
		key = ZOOID_KEY_ARROW_LEFT;
	}
	else if (key == GLFW_KEY_RIGHT)
	{
		key = ZOOID_KEY_ARROW_RIGHT;
	}
	else if (key == GLFW_KEY_HOME)
	{
		key = ZOOID_KEY_HOME;
	}
	else if (key == GLFW_KEY_END)
	{
		key = ZOOID_KEY_END;
	}

	int keyState = -1;
	switch (action)
	{
	case GLFW_PRESS:
		keyState = 0;
		break;
	case GLFW_RELEASE:
		keyState = 1;
		break;
	case GLFW_REPEAT:
		keyState = 2;
		break;
	}

	if (keyState != -1)
	{
		ZE::UI::RecordKeyboardButton((ZE::UIChar)key, action == GLFW_RELEASE);
	}
}

void charInputCallback(GLFWwindow* window, unsigned int codepoint)
{
	ZE::UI::RecordTextInput((ZE::UIChar)codepoint);
}

void mousePositionCallback(GLFWwindow* window, double xPos, double yPos)
{
	mouseX = xPos;
	mouseY = yPos;
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	ZE::UI::RecordMouseScroll(yOffset);
}

void mouseButtonUpdateCallback(GLFWwindow* window, int button, int action, int mods)
{
	buttonState = action == GLFW_RELEASE ? ZE::BUTTON_UP : ZE::BUTTON_DOWN;
}

void windowFrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	ZE::UI::ResizeWindow(width, height);
}

int main()
{
	ZE::GL_UIRenderer* renderer;

	ZE::UI::Init(1000, 800);

	renderer = (ZE::GL_UIRenderer*) ZE::UI::GetRenderer();

	glfwSetCursorPosCallback((GLFWwindow*)renderer->getWindowContext(), mousePositionCallback);
	glfwSetMouseButtonCallback((GLFWwindow*)renderer->getWindowContext(), mouseButtonUpdateCallback);
	glfwSetFramebufferSizeCallback((GLFWwindow*)renderer->getWindowContext(), windowFrameBufferSizeCallback);
	glfwSetKeyCallback((GLFWwindow*)renderer->getWindowContext(), keyCallback);
	glfwSetCharCallback((GLFWwindow*)renderer->getWindowContext(), charInputCallback);
	glfwSetScrollCallback((GLFWwindow*)renderer->getWindowContext(), scrollCallback);

	ZE::UITexture* panelBg = ZE::UI::LoadTexture("../../Resource/Textures/PanelBg.png");

	bool bChecked = false;
	ZE::Int32 selectedRadioButton = 0;
	ZE::Float32 sliderPercent = 0.0;

	const char* radioTexts[5] = { "Radio Button 1", "Radio Button 2", "Radio Button 3", "Radio Button 4", "Radio Button 5" };
	ZE::Float32 cpuTime = 0.0f;
	ZE::Float32 drawTime = 0.0f;
	ZE::Float32 totalTime = 0.0f;
	ZE::Timer timer;

	char buffer[256];
	
	const char* dropdownText[5] = { "Option 1", "Option 2", "Option 3", "Option 4", "Option 5" };
	ZE::Int32 dropdownIndex = 1;

	ZE::UIChar bufferInput[256];
	bufferInput[0] = 0;

	ZE::Float32 number = 1.0f;
	ZE::Float32 number2 = 1.0f;
	
	ZE::Float32 vec2[2] = { 0.0f, 0.0f };
	ZE::Float32 vec3[3] = { 0.0f, 0.0f, 0.0f };
	ZE::Float32 vec4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	while (!renderer->requestToClose())
	{
		timer.Reset();

		// Immediate UI Logic
		ZE::UI::BeginFrame();

		// Update State
		ZE::UI::UpdateMouseState((ZE::Float32)mouseX, (ZE::Float32)mouseY, buttonState);

		sprintf_s(buffer, "CPU Time: %.2f ms", cpuTime);
		ZE::UI::DrawTextInPos(ZE::UIVector2{ 0.0f, ZE::UI::GetScreenHeight() - 4.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "Draw Time: %.2f ms", drawTime);
		ZE::UI::DrawTextInPos(ZE::UIVector2{ 0.0f, ZE::UI::GetScreenHeight() - 3.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "Total Time: %.2f ms", totalTime);
		ZE::UI::DrawTextInPos(ZE::UIVector2{ 0.0f, ZE::UI::GetScreenHeight() - 2.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "FPS: %.1f", 1.0f / (totalTime / 1000.0f));
		ZE::UI::DrawTextInPos(ZE::UIVector2{ 0.0f, ZE::UI::GetScreenHeight() - 1.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		if (ZE::UI::BeginPanel("Text Panel...", ZE::UIRect(ZE::UIVector2(150.0f, 100.f), ZE::UIVector2(250.0f, 300.0f))))
		{
			ZE::UI::DoText("Test Text");
			if (ZE::UI::DoButton("Button"))
			{
				std::cout << "Button Clicked" << std::endl;
			}
			bChecked = ZE::UI::DoCheckBox("Checkbox", bChecked);
			ZE::UI::DoRadioButtons(radioTexts, 5, &selectedRadioButton);
			ZE::UI::DoSlider(&sliderPercent);
			ZE::UI::DoDropDown(&dropdownIndex, dropdownText, 5);
			ZE::UI::DoTextInput(bufferInput, 256);
			ZE::UI::DoNumberStepper(&number, 1.0f, true);
			ZE::UI::DoNumberInput(&number2);
			ZE::UI::DoVector2Input(vec2);
			ZE::UI::DoVector3Input(vec3);
			ZE::UI::DoVector4Input(vec4);
			ZE::UI::EndPanel();
		}

#if 0
		if (ZE::UI::BeginPanel("Image Scaling...", ZE::UIRect(ZE::UIVector2(350.0f, 100.f), ZE::UIVector2(250.0f, 500.0f))))
		{
			ZE::UI::DrawTexture({ 200, 70 }, panelBg, ZE::UIVector4{ 1.0f });
			ZE::UI::DrawTexture({ 200, 70 }, panelBg, ZE::UIVector4{ 1.0f }, ZE::SCALE_9SCALE, ZE::UIVector4{ 0.40f, 0.15f, 0.15f, 0.15f });
			ZE::UI::DrawTexture({ 200, 70 }, panelBg, ZE::UIVector4{ 1.0f }, ZE::SCALE_BORDER, ZE::UIVector4{ 0.40f, 0.15f, 0.15f, 0.15f });
			ZE::UI::EndPanel();
		}

		if (ZE::UI::BeginPanel("Text Sample...", ZE::UIRect(ZE::UIVector2(600.0f, 100.f), ZE::UIVector2(250.0f, 500.0f))))
		{
			ZE::UI::DrawMultiLineText("Text Align Left.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f });
			ZE::UI::DrawMultiLineText("Text Align Center.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f }, ZE::TEXT_CENTER);
			ZE::UI::DrawMultiLineText("Text Align Right.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f }, ZE::TEXT_RIGHT);
			ZE::UI::EndPanel();
		}
#endif

		ZE::UI::BeginMenu();
		if (ZE::UI::DoMenu("File"))
		{
			ZE::UI::BeginSubMenu(100.0f);
			if (ZE::UI::DoSubMenu("New", true))
			{
				ZE::UI::BeginSubMenu(100.0f);
				if (ZE::UI::DoSubMenu("Document"))
				{
					std::cout << "File|New|Document" << std::endl;
				}
				if (ZE::UI::DoSubMenu("Project", true))
				{
					ZE::UI::BeginSubMenu(100.0f);
					ZE::UI::DoSubMenu("C++");
					ZE::UI::DoSubMenu("Java");
					ZE::UI::EndSubMenu();
				}
				ZE::UI::EndSubMenu();
			}
			if (ZE::UI::DoSubMenu("Save..."))
			{
				std::cout << "Saving...." << std::endl;
			}
			ZE::UI::EndSubMenu();
		}
		ZE::UI::DoMenu("View");
		ZE::UI::EndMenu();

		ZE::UI::EndFrame();
		cpuTime = ZE::UI::Lerp( cpuTime, timer.ResetAndGetDeltaMS(), cpuTime == 0.0f ? 1.0f : .01f);
		
		ZE::UI::ProcessDrawList();
		drawTime = ZE::UI::Lerp( drawTime, timer.ResetAndGetDeltaMS(), drawTime == 0.0f ? 1.0f : .01f);

		totalTime = cpuTime + drawTime;
	}

	ZE::UI::Destroy();

	return 0;
}