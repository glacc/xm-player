#include "TestMenu.hpp"
#include "../xmPlayer/xmPlayer.hpp"

namespace Screens
{
	namespace TestMenu
	{
		TestMenuScreen testMenuScreen;
	}
}

using namespace Screens;

void TestMenu::ElemOverlayTest()
{
	testMenuScreen.btnOverlayTest.px += 16;
	testMenuScreen.btnOverlayTest.py -= 16;
	testMenuScreen.btnOverlayTest.UpdateSettings();
}

void TestMenu::TestMenuScreen::Init()
{
	int centerX = renderTarget->getSize().x / 2;
	int centerY = renderTarget->getSize().y / 2;
	
	const int btnWidth = 128;
	const int btnHeight = 32;
	const int btnSpacing = 12;
	const int btnFontSize = 18;
	const int offsetInc = btnSpacing + btnHeight;
	int offsetX = centerX - btnWidth / 2;
	int offsetY = centerY - btnHeight / 2 - offsetInc;

	btnChangeColor =
		UI::Elements::Button("Change Color",
			UI::Globals::font, btnFontSize, offsetX, offsetY, btnWidth, btnHeight, colorChangeFunc);
	offsetY += offsetInc;
	btnToggleSound = 
		UI::Elements::Button("Toggle Sound",
			UI::Globals::font, btnFontSize, offsetX, offsetY, btnWidth, btnHeight, soundToggleFunc);
	offsetY += offsetInc;
	btnOverlayTest =
		UI::Elements::Button("Overlay Test",
			UI::Globals::font, btnFontSize, offsetX, offsetY, btnWidth, btnHeight, TestMenu::ElemOverlayTest);
	offsetY += offsetInc;
	btnExit =
		UI::Elements::Button("Exit",
			UI::Globals::font, btnFontSize, offsetX, offsetY, btnWidth, btnHeight, exitFunc);

	elements.push_back(&btnChangeColor);
	elements.push_back(&btnToggleSound);
	elements.push_back(&btnOverlayTest);
	elements.push_back(&btnExit);

	lblTitle =
		UI::Elements::Label("Test xwx",
			UI::Globals::font, 28, UI::Elements::Label::Middle, centerX, centerY / 3);
	lblDebug =
		UI::Elements::Label("debug\ndebug\ndebug\ndebug\ndebug",
			UI::Globals::font, 16, UI::Elements::Label::BottomLeft, 8, renderTarget->getSize().y - 8);
	elements.push_back(&lblTitle);
	elements.push_back(&lblDebug);
}

void TestMenu::TestMenuScreen::UpdateCustom()
{
	lblDebug.label = "";
	//lblDebug.label = lblDebug.label.append(std::to_string(GXMPlayer::GetPos() >> 24)).append(", ").append(std::to_string(GXMPlayer::GetPos() & 0xFFFF)).append(", ").append(std::to_string(GXMPlayer::GetExcuteTime())).append("\n");
	lblDebug.label = lblDebug.label.append(std::to_string(UI::Event::mouseAbsX)).append(", ").append(std::to_string(UI::Event::mouseAbsY)).append("\n");
	lblDebug.label = lblDebug.label.append(std::to_string(UI::Event::mouseX)).append(", ").append(std::to_string(UI::Event::mouseY)).append("\n");
	lblDebug.label = lblDebug.label.append(std::to_string(UI::Event::mouseVelX)).append(", ").append(std::to_string(UI::Event::mouseVelY)).append("\n");
	lblDebug.label = lblDebug.label.append(UI::Event::mouseInbound ? "true" : "false");
	lblDebug.UpdateSettings();
}

void TestMenu::TestMenuScreen::DrawCustom()
{
	renderTarget->clear(sf::Color(0xDF, 0xDF, 0xDF));
}

TestMenu::TestMenuScreen::TestMenuScreen() {}
