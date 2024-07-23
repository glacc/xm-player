#pragma once

#include "../xmPlayer/xmPlayer.hpp"

#include "../UI/Screen.hpp"
#include "../UI/Elements.hpp"
#include "../UI/Globals.hpp"

namespace Screens
{
	namespace PatternViewer
	{
		class PatternViewerScreen : public UI::Screen
		{
			bool mouseHold = false;

			sf::RectangleShape cursorRect;

			Glacc::XMPlayer *xmPlayer;
			Glacc::XMPlayer::Stat stat;

		public:
			UI::Elements::Label lblInstruction;

			bool detailedView = true;

			int chOffset = 0;

			void Init() override;

			void UpdateCustom() override;

			void DrawCustom() override;

			PatternViewerScreen();
		};

		extern PatternViewerScreen patternViewerScreen;
	}
}

