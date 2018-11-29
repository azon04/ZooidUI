#include "UI/ZooidUI.h"
#include "Common/Timer.h"
#include "GL_UIRenderer.h"
#include <iostream>
#include <cstring>

double mouseX = 0.0f;
double mouseY = 0.0f;
ZE::EButtonState buttonState = ZE::BUTTON_UP;

void mousePositionCallback(GLFWwindow* window, double xPos, double yPos)
{
	mouseX = xPos;
	mouseY = yPos;
}

void mouseButtonUpdateCallback(GLFWwindow* window, int button, int action, int mods)
{
	buttonState = action == GLFW_RELEASE ? ZE::BUTTON_UP : ZE::BUTTON_DOWN;
}

int main()
{
	ZE::UIState state;
	ZE::GL_UIRenderer* renderer;

	ZE::UI::Init(1000, 800);

	renderer = (ZE::GL_UIRenderer*) ZE::UI::GetUIState()->renderer;

	glfwSetCursorPosCallback((GLFWwindow*)renderer->getWindowContext(), mousePositionCallback);
	glfwSetMouseButtonCallback((GLFWwindow*)renderer->getWindowContext(), mouseButtonUpdateCallback);

	ZE::UITexture* panelBg = ZE::UI::LoadTexture("../../Resource/Textures/PanelBg.png");

	bool bChecked = false;
	ZE::Int32 selectedRadioButton = 4;
	ZE::Float32 sliderPercent = 0.0;
	bool bPanelClosed = false;

	ZE::UIRect panelRect;
	panelRect.m_pos = { 100, 100 };
	panelRect.m_dimension = { 250, 275 };
	panelRect.m_roundness = 10;

	ZE::UIRect panel2Rect = panelRect;
	panel2Rect.m_pos.x += 300;

	ZE::UIRect panel3Rect = panel2Rect;
	panel3Rect.m_pos.x += 300;
	panel3Rect.m_dimension.y += 120;
	panel3Rect.m_dimension.x += 100;

	const char* radioTexts[5] = { "Radio Button 1", "Radio Button 2", "Radio Button 3", "Radio Button 4", "Radio Button 5" };
	ZE::Float32 cpuTime = 0.0f;
	ZE::Float32 drawTime = 0.0f;
	ZE::Float32 totalTime = 0.0f;
	ZE::Timer timer;

	char buffer[256];

	ZE::UIRect sliderRect;
	sliderRect.m_dimension.x = panelRect.m_dimension.x - 50;
	sliderRect.m_dimension.y = 20;


	ZE::UIVector2 contentPos;

	ZE::UIRect buttonRect;
	buttonRect.m_dimension = { 150, 40 };
	buttonRect.m_roundness = 10.0f;

	while (!renderer->requestToClose())
	{
		timer.Reset();

		// Immediate UI Logic
		ZE::UI::BeginFrame();

		// Update State
		ZE::UI::UpdateMouseState((ZE::Float32)mouseX, (ZE::Float32)mouseY, buttonState);

		sprintf_s(buffer, "CPU Time: %.2f ms", cpuTime);
		ZE::UI::DrawTextInPos(-99, ZE::UIVector2{ 0.0f, .0f }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "Draw Time: %.2f ms", drawTime);
		ZE::UI::DrawTextInPos(-99, ZE::UIVector2{ 0.0f, 1.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "Total Time: %.2f ms", totalTime);
		ZE::UI::DrawTextInPos(-99, ZE::UIVector2{ 0.0f, 2.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

		sprintf_s(buffer, "FPS: %.1f", 1.0f / (totalTime / 1000.0f));
		ZE::UI::DrawTextInPos(-99, ZE::UIVector2{ 0.0f, 3.0f * ZE::UI::DefaultFont->calculateTextHeight(1.0f) }, buffer, ZE::UIVector4{ 1.0f });

 		ZE::UI::DoDragablePanel(0, panelRect, "Test Panel...", 10, contentPos);

		buttonRect.m_pos = contentPos;
		if (ZE::UI::DoButton(1, buttonRect))
		{
			std::cout << "Button Clicked" << std::endl;
		}

		ZE::UI::DrawTextInRect(2, buttonRect, ZE::UI::GetUIState()->hotItem.id == 1 ? "Hovered" : "Test", ZE::UIVector4(1.0f), ZE::TEXT_CENTER);

		contentPos.y += buttonRect.m_dimension.y + 10;

		bChecked = ZE::UI::DoCheckBox(3, contentPos, "This is a CheckBox", bChecked);

		contentPos.y += 25;

		ZE::Int32 startIndex = 4;
		for (ZE::Int32 i = 0; i < 5; i++)
		{
			selectedRadioButton = ZE::UI::DoRadioButton(startIndex + i, contentPos, radioTexts[i], selectedRadioButton);
			contentPos.y += 25;
		}
 
		sliderRect.m_pos = contentPos;
		sliderPercent = ZE::UI::DoSlider(9, sliderRect, sliderPercent);

		{
			ZE::UI::DoPanel(10, panel2Rect, "Image Scaling...", 10, contentPos, bPanelClosed);

			if (!bPanelClosed)
			{
				ZE::UIRect imageRect;
				imageRect.m_pos = contentPos;
				imageRect.m_dimension = { 200, 70 };

				ZE::UI::DrawTexture(11, imageRect, panelBg, ZE::UIVector4{1.0f});

				imageRect.m_pos.y += imageRect.m_dimension.y + 5;

				ZE::UI::DrawTexture(12, imageRect, panelBg, ZE::UIVector4{ 1.0f }, ZE::SCALE_9SCALE, ZE::UIVector4{ 0.40f, 0.15f, 0.15f, 0.15f });

				imageRect.m_pos.y += imageRect.m_dimension.y + 5;

				ZE::UI::DrawTexture(13, imageRect, panelBg, ZE::UIVector4{ 1.0f }, ZE::SCALE_BORDER, ZE::UIVector4{ 0.40f, 0.15f, 0.15f, 0.15f });
			}
		}

		{
			ZE::UI::DoDragablePanel(14, panel3Rect, "Text Sample...", 10, contentPos);

			ZE::UIRect textRect;
			textRect.m_pos = contentPos;
			textRect.m_dimension = { 320, 100 };
			
			ZE::UI::DrawMultiLineText(15, textRect, "Text Align Left.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f });
			
			textRect.m_pos.y += 120;
			ZE::UI::DrawMultiLineText(15, textRect, "Text Align Center.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f }, ZE::TEXT_CENTER);
			
			textRect.m_pos.y += 120;
			ZE::UI::DrawMultiLineText(15, textRect, "Text Align Right.\nLorem Ipsum Dolor Sit Amet. Lorem Ipsum Dolor Sit Amet.\nLorem Ipsum Dolor Sit Amet. Anpan. Anpan. Anpan. Anpan. Anpan. Anpan.", ZE::UIVector4{ 1.0f, 1.0f, 1.0f, 1.0f }, ZE::TEXT_RIGHT);
		
		}
		
		ZE::UI::EndFrame();
		cpuTime = ZE::UI::Lerp( cpuTime, timer.ResetAndGetDeltaMS(), cpuTime == 0.0f ? 1.0f : .01f);
		
		ZE::UI::ProcessDrawList();
		drawTime = ZE::UI::Lerp( drawTime, timer.ResetAndGetDeltaMS(), drawTime == 0.0f ? 1.0f : .01f);

		totalTime = cpuTime + drawTime;
	}

	ZE::UI::Destroy();

	return 0;
}