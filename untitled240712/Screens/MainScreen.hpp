#pragma once

#include "../UI/Screen.hpp"
#include "../UI/Elements.hpp"
#include "../UI/Globals.hpp"

#include "../xmPlayer/xmPlayer.hpp"

namespace Screens
{
	namespace Main
	{
		class MainScreen : public UI::Screen
		{
		public:
			UI::Elements::Button btnExit;
			UI::Elements::Button btnOpen;
			UI::Elements::Button btnPlay;
			UI::Elements::Button btnPause;
			UI::Elements::Button btnReplay;
			UI::Elements::Button btnUnmuteAll;

			UI::Elements::Label lblMsg;

			UI::Elements::Viewport viewportScope;
			UI::Elements::Viewport viewportPattern;
			UI::Elements::Viewport viewportPatternOrder;

			UI::Elements::CheckBox chkBoxInterpolation;

			UI::Elements::Label lblAmp;
			UI::Elements::Slider<float> sliderAmp;
			UI::Elements::Label lblAmpVal;

			void Init() override;

			void UpdateSettings() override;

			void UpdateCustom() override;

			void DrawCustom() override;

			MainScreen();
		};

		extern MainScreen mainScreen;

		extern Glacc::XMPlayer xmPlayer;
	}
}
