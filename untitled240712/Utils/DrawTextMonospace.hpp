#pragma once

#include <SFML/Graphics.hpp>

namespace Utils
{
	void DrawTextMonospace(sf::RenderTarget *renderTarget, std::string str, int px, int py, int fontSize, int fontWidth, int maxX, int maxY, sf::Font *font, sf::Color color = sf::Color(0x66, 0x66, 0x66));
}
