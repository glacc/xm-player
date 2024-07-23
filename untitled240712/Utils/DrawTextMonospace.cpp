#include "DrawTextMonospace.hpp"
#include "../UI/Globals.hpp"

void Utils::DrawTextMonospace(sf::RenderTarget *renderTarget, std::string str, int px, int py, int fontSize, int fontWidth, int maxX, int maxY, sf::Font *font, sf::Color color)
{
	if (px < -fontSize || py > maxY + fontSize)
		return;

	int ofsX = px;
	int ofsY = py;
	for (int i = 0; i < str.length(); i ++)
	{
		if (str[i] == ' ' || ofsX < -fontWidth / 2)
		{
			ofsX += fontWidth;
			continue;
		}

		sf::Text char_(str[i], *font, fontSize);
		char_.setOrigin((int)(
			char_.getLocalBounds().left + char_.getLocalBounds().width / 2),
			(int)(char_.getLocalBounds().top + char_.getLocalBounds().height / 2)
		);

		char_.setFillColor(color);
		char_.setPosition((int)(ofsX + fontWidth / 2), (int)ofsY);
		renderTarget->draw(char_);

		ofsX += fontWidth;

		if (ofsX > maxX + fontWidth / 2)
			break;
	}
}
