#pragma once

#include "../Element.hpp"

namespace UI
{
	namespace Elements
	{
		class Button : public Element
		{
		private:
			sf::Text text;
			sf::RectangleShape bg;

			void UpdateOrigin() override;

			void InitSettings(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)());

		public:
			std::string label;
			sf::Font *font;
			int fontSize;

			int px, py, width, height;

			void (*onPress)() = NULL;
			void (*onClick)() = NULL;

			bool pressed = false;
			
			void UpdateSettings() override;

			bool MouseHoverCheck() override;

			void MouseEventHandler() override;

			void Update() override;

			void Draw(sf::RenderTarget *renderTarget) override;

			void UpdateAndDraw(sf::RenderTarget *renderTarget) override;

			Button(std::string newLabel, sf::Font *newFont, int newFontSize, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)() = NULL);

			Button(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)() = NULL);

			Button(std::string newLabel, sf::Font *newFont, int newFontSize, int newWidth, int newHeight, void (*newOnClick)() = NULL);

			Button(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newWidth, int newHeight, void (*newOnClick)() = NULL);

			Button();
		};
	}
}