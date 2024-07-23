#pragma once

//#include "UIRoot.hpp"
#include "Element.hpp"
#include <vector>

#include <SFML/Graphics.hpp>

namespace UI
{
	class Screen
	{
	private:
		void UpdateElements();

		void DrawElements();

	protected:
		std::vector<Element *> elements;

		virtual void UpdateCustom();

		virtual void DrawCustom();

	public:
		bool init = false;

		sf::Color bgColor = sf::Color::Transparent;

		bool mouseInbound = false;
		UI::Element *mouseHoverOn = NULL;

		sf::RenderTarget *renderTarget = NULL;

		void InitCheck();

		virtual void Init();

		virtual void UpdateSettings();

		void MouseHoverCheck();

		void Update();

		void Draw();

		void UpdateAndDraw();

		Screen();
	};
}
