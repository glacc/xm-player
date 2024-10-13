#include "MainScreen.hpp"

#include <fstream>
#include <Windows.h>

#include "PatternViewer.hpp"
#include "OrderTable.hpp"
#include "Scope.hpp"

namespace Screens
{
	namespace Main
	{
		MainScreen mainScreen;

		Glacc::XMPlayer xmPlayer;

		void Exit()
		{
			UI::Globals::exitReq = true;
		}

		void LoadSong()
		{
			char szFile[256];

			OPENFILENAMEA open;
			ZeroMemory(&open, sizeof(open));

			open.lStructSize = sizeof(OPENFILENAMEA);
			open.lpstrFilter = "Fasttracker II module (*.xm)\0*.xm\0";
			open.nFilterIndex = 1;
			open.nFileOffset = 1;
			open.lpstrFile = szFile;
			open.lpstrDefExt = "xm";
			open.lpstrFile[0] = '\0';
			open.nMaxFile = sizeof(szFile);
			open.lpstrTitle = "Open module";
			open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameA(&open))
			{
				std::string filePath = szFile;
				std::ifstream fin(filePath, std::ios::binary | std::ios::in);

				if (fin)
				{
					fin.seekg(0, std::ios_base::end);
					int fileSize = fin.tellg();

					char *fileData = (char *)malloc(fileSize);
					fin.seekg(0, std::ios_base::beg);
					fin.read(fileData, fileSize);

					xmPlayer.StopModule();
					xmPlayer.CleanUp();
					if (xmPlayer.LoadModule((uint8_t *)fileData, fileSize, mainScreen.chkBoxInterpolation.checked))
					{
						mainScreen.lblMsg.label = std::string("").append("Module ").append(filePath).append(" loaded");
						mainScreen.lblMsg.UpdateSettings();
						PatternViewer::patternViewerScreen.lblInstruction.visible = false;

						Glacc::XMPlayer::Stat stat;
						Screens::Main::xmPlayer.GetCurrentStat(stat);
						Glacc::Global::window->setTitle(Glacc::Global::winTitle + " - " + stat.songName);
					}
					else
					{
						mainScreen.lblMsg.label = std::string("").append("Unable to load module ").append(filePath);
						mainScreen.lblMsg.UpdateSettings();

						Glacc::Global::window->setTitle(Glacc::Global::winTitle);
					}

					PatternViewer::patternViewerScreen.chOffset = 0;

					free(fileData);
				}
				else
				{
					mainScreen.lblMsg.label = std::string("").append("Unable to open module ").append(filePath);
					mainScreen.lblMsg.UpdateSettings();
				}
			}
		}

		void PlaySong()
		{
			if (xmPlayer.IsLoaded())
				xmPlayer.PlayModule();
			else
			{
				mainScreen.lblMsg.label = std::string("").append("No module loaded");
				mainScreen.lblMsg.UpdateSettings();
			}
		}

		void PauseToggleSong()
		{
			if (xmPlayer.IsLoaded())
				xmPlayer.PlayPause(!xmPlayer.IsPlaying());
			else
			{
				mainScreen.lblMsg.label = std::string("").append("No module loaded");
				mainScreen.lblMsg.UpdateSettings();
			}
		}

		void ReplaySong()
		{
			if (xmPlayer.IsLoaded())
			{
				bool isPlaying = xmPlayer.IsPlaying();

				xmPlayer.ResetModule();

				if (isPlaying)
					xmPlayer.PlayModule();
			}
			else
			{
				mainScreen.lblMsg.label = std::string("").append("No module loaded");
				mainScreen.lblMsg.UpdateSettings();
			}
		}

		void UnmuteAll()
		{
			xmPlayer.UnmuteAll();
		}

		void SetInterpolation()
		{
			xmPlayer.interpolation = mainScreen.chkBoxInterpolation.checked;	
		}

		void SetAmplifier()
		{
			xmPlayer.SetAmp(mainScreen.sliderAmp.value);

			std::string strAmp = std::to_string(
					(int)mainScreen.sliderAmp.value
				).append(".").append(std::to_string(
					(int)(mainScreen.sliderAmp.value * 10.0F + 0.5F) % 10
				)).append("x");
			mainScreen.lblAmpVal.label = strAmp;
			mainScreen.lblAmpVal.UpdateSettings();
		}
	}
}

using namespace Screens;

void Main::MainScreen::Init()
{
	const int btnWidth = 128;
	const int btnHeight = 32;
	const int btnFontSize = 20;

	// Exit Button
	elements.push_back(
		&(btnExit =
			UI::Elements::Button("Exit",
				UI::Globals::font, btnFontSize, UI::Element::TopRight, btnWidth, btnHeight, Exit))
	);

	// Control Buttons
	elements.push_back(
		&(btnOpen =
			UI::Elements::Button("Open",
				UI::Globals::font, btnFontSize, btnWidth, btnHeight, LoadSong))
	);
	elements.push_back(
		&(btnPlay =
			UI::Elements::Button("Play",
				UI::Globals::font, btnFontSize, btnWidth, btnHeight, PlaySong))
	);
	elements.push_back(
		&(btnPause =
			UI::Elements::Button("Pause",
				UI::Globals::font, btnFontSize, btnWidth, btnHeight, PauseToggleSong))
	);
	elements.push_back(
		&(btnReplay =
			UI::Elements::Button("Reset",
				UI::Globals::font, btnFontSize, btnWidth, btnHeight, ReplaySong))
	);
	elements.push_back(
		&(btnUnmuteAll =
			UI::Elements::Button("Unmute All",
				UI::Globals::font, btnFontSize, btnWidth, btnHeight, UnmuteAll))
	);
	// Settings
	elements.push_back(
		&(chkBoxInterpolation =
			UI::Elements::CheckBox())
	);
	chkBoxInterpolation.checked = true;
	// Amplifier
	elements.push_back(
		&(lblAmp =
			UI::Elements::Label("Amp.", UI::Globals::font, btnFontSize, UI::Element::Left))
	);
	elements.push_back(
		&(sliderAmp =
			UI::Elements::Slider<float>())
	);
	sliderAmp.minValue = 1.0F;
	sliderAmp.maxValue = 12.0F;
	sliderAmp.value = 1.0F;
	sliderAmp.valInc = 0.25F;
	elements.push_back(
		&(lblAmpVal =
			UI::Elements::Label("1.0x", UI::Globals::font, btnFontSize, UI::Element::Left))
	);

	// Pattern Order Table
	elements.push_back(
		&(viewportPatternOrder =
			UI::Elements::Viewport(&Screens::OrderTable::orderTable, UI::Globals::root->rootViewport))
	);

	// Message Label
	const int msgLblSize = 18;
	elements.push_back(
		&(lblMsg =
			UI::Elements::Label("uwu",
				UI::Globals::font, msgLblSize, UI::Element::Bottom))
	);

	// Scopes
	elements.push_back(
		&(viewportScope =
			UI::Elements::Viewport(&Screens::Scope::scope, UI::Globals::root->rootViewport))
	);
	// Pattern Viewport
	elements.push_back(
		&(viewportPattern = 
			UI::Elements::Viewport(&Screens::PatternViewer::patternViewerScreen, UI::Globals::root->rootViewport))
	);

	UpdateSettings();
}

void Main::MainScreen::UpdateSettings()
{
	int sizeX = renderTarget->getSize().x;
	int sizeY = renderTarget->getSize().y;
	int centerX = sizeX / 2;
	int centerY = sizeY / 2;

	const int spacing = 12;

	const int btnWidth = 128;
	const int btnHeight = 32;
	const int btnFontSize = 20;
	const int offsetIncX = spacing + btnWidth;
	const int offsetIncY = spacing + btnHeight;
	int offsetX;
	int offsetY;

	offsetX = sizeX - spacing, offsetY = spacing;
	btnExit.px = offsetX;
	btnExit.py = offsetY;
	btnExit.UpdateSettings();

	offsetX = spacing, offsetY = spacing;
	viewportPatternOrder.px = offsetX;
	viewportPatternOrder.py = offsetY;
	viewportPatternOrder.width = btnWidth / 2;
	viewportPatternOrder.height = btnHeight * 2 + spacing;
	viewportPatternOrder.UpdateSettings();

	const int btnStartX = btnWidth / 2 + spacing * 2;
	offsetX = btnStartX;
	btnOpen.px = offsetX;
	btnOpen.py = offsetY;
	btnOpen.UpdateSettings();
	offsetX += offsetIncX;
	btnPlay.px = offsetX;
	btnPlay.py = offsetY;
	btnPlay.UpdateSettings();
	offsetX += offsetIncX;
	btnPause.px = offsetX;
	btnPause.py = offsetY;
	btnPause.UpdateSettings();
	offsetX += offsetIncX;
	btnReplay.px = offsetX;
	btnReplay.py = offsetY;
	btnReplay.UpdateSettings();
	// Preventing "Exit" button overlays on other buttons.
	btnExit.visible = !(offsetX + offsetIncX > sizeX - spacing - btnWidth);


	offsetX = btnStartX, offsetY += offsetIncY;
	btnUnmuteAll.px = offsetX;
	btnUnmuteAll.py = offsetY;
	btnUnmuteAll.UpdateSettings();
	offsetX += offsetIncX;
	chkBoxInterpolation.label = "Interpol.";
	chkBoxInterpolation.font = UI::Globals::font;
	chkBoxInterpolation.fontSize = btnFontSize;
	chkBoxInterpolation.px = offsetX;
	chkBoxInterpolation.py = offsetY;
	chkBoxInterpolation.width = btnWidth;
	chkBoxInterpolation.height = btnHeight;
	chkBoxInterpolation.onClick = SetInterpolation;
	chkBoxInterpolation.UpdateSettings();

	offsetX += offsetIncX;
	lblAmp.px = offsetX;
	lblAmp.py = offsetY + btnHeight / 2;
	lblAmp.UpdateSettings();
	offsetX += btnWidth / 2 - spacing;
	sliderAmp.px = offsetX;
	sliderAmp.py = offsetY;
	sliderAmp.width = btnWidth;
	sliderAmp.height = btnHeight;
	sliderAmp.sliderWidth = 12;
	sliderAmp.sliderHeight = btnHeight * 3 / 4;
	sliderAmp.bgThickness = 8;
	sliderAmp.direction = UI::Elements::Slider<float>::Horizontal;
	sliderAmp.onValueChange = SetAmplifier;
	sliderAmp.UpdateSettings();
	offsetX += btnWidth + spacing;
	lblAmpVal.px = offsetX;
	lblAmpVal.py = offsetY + btnHeight / 2;
	lblAmpVal.UpdateSettings();

	int viewerTop = offsetY + offsetIncY;

	const int msgLblSize = 18;
	int viewerBottom = sizeY - msgLblSize - spacing * 2;
	lblMsg.px = sizeX / 2;
	lblMsg.py = sizeY - spacing;
	lblMsg.UpdateSettings();

	int viewerLeft = spacing;
	int viewerRight = sizeX - spacing;
	int viewerWidth = viewerRight - viewerLeft;
	int viewerHeight = viewerBottom - viewerTop;
	viewportScope.px = viewerLeft;
	viewportScope.py = viewerTop;
	viewportScope.width = viewerRight - viewerLeft;
	viewportScope.height = btnHeight * 2 + spacing;
	viewportScope.UpdateSettings();

	viewerTop += offsetIncY * 2;
	viewportPattern.px = viewerLeft;
	viewportPattern.py = viewerTop;
	viewportPattern.width = viewerRight - viewerLeft;
	viewportPattern.height = viewerBottom - viewerTop;
	viewportPattern.UpdateSettings();
}

void Main::MainScreen::UpdateCustom()
{

}

void Main::MainScreen::DrawCustom()
{

}

Main::MainScreen::MainScreen() {};
