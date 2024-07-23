#pragma once

#include "../Element.hpp"

namespace UI
{
	namespace Elements
	{
		class Label : public Element
		{
		private:
			sf::Text text;

			void UpdateOrigin() override;

			void InitSettings(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, sf::Color newColor);

		public:

			std::string label;
			sf::Font *font;
			int fontSize;
			sf::Color color;

			int px, py;

			void UpdateSettings() override;

			void Draw(sf::RenderTarget *renderTarget) override;

			void UpdateAndDraw(sf::RenderTarget *renderTarget) override;

			Label(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, sf::Color newColor = sf::Color(0x66, 0x66, 0x66));

			Label(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, sf::Color newColor = sf::Color(0x66, 0x66, 0x66));

			Label();
		};
	}
}
