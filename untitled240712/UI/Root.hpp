#pragma once

#include "Elements/Viewport.hpp"
#include "Screen.hpp"
#include <SFML/Graphics.hpp>

namespace UI
{
	class Root
	{
	public:
		UI::Elements::Viewport *rootViewport;

		UI::Screen *currentScreen;

		sf::RenderTarget *renderTarget;

		sf::Color bgColor = sf::Color(0xDF, 0xDF, 0xDF);

		void UpdateSettings();

		void Update();

		void Draw();

		Root(sf::RenderTarget *initRenderTarget, UI::Screen *initScreen);

		Root();

		~Root();
	};
}
