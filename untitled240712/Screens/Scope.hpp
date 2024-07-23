#pragma once

#include "../UI/Screen.hpp"
#include "../UI/Elements.hpp"
#include "../UI/Globals.hpp"

#include "../xmPlayer/xmPlayer.hpp"

namespace Screens
{
	namespace Scope
	{
		class Scope : public UI::Screen
		{
			Glacc::XMPlayer *xmPlayer;
			Glacc::XMPlayer::Stat stat;

			bool mouseHold = false;

			const int fontSize = 18;
			const int fontWidth = 8;
			const int spacing = 2;

			uint32_t *pixelBuffer = NULL;
			sf::Texture *scopeTexture = NULL;

			int width, height;
			int bufferSize;

			int borders[33] = { 0 };
			bool narrow = false;

		public:
			void Init() override;

			void UpdateSettings() override;

			void UpdateCustom() override;

			void DrawCustom() override;

			~Scope();
		};

		extern Scope scope;
	}
}