#pragma once

#include "../UI/Screen.hpp"
#include "../UI/Elements.hpp"
#include "../UI/Globals.hpp"

#include "../xmPlayer/xmPlayer.hpp"

namespace Screens
{
	namespace OrderTable
	{
		class OrderTable : public UI::Screen
		{
			sf::RectangleShape cursorRect;

			Glacc::XMPlayer *xmPlayer;
			Glacc::XMPlayer::Stat stat;

			bool drag = false;

			int movY;

			const int fontSize = 18;
			const int fontWidth = 8;
			const int spacing = 2;
			const int totalWidth = fontWidth * 5 + spacing * 2;

		public:
			void Init() override;

			void UpdateCustom() override;

			void DrawCustom() override;
		};

		extern OrderTable orderTable;
	}
}
