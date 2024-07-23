#include "Label.hpp"

using namespace UI::Elements;

void Label::UpdateOrigin()
{
	originX = text.getLocalBounds().left;
	originY = text.getLocalBounds().top;
	switch (origin)
	{
	case Left:
		originY += text.getLocalBounds().height / 2;
		break;
	case TopLeft:
		break;
	case Top:
		originX += text.getLocalBounds().width / 2;
		break;
	case TopRight:
		originX += text.getLocalBounds().width;
		break;
	case Right:
		originX += text.getLocalBounds().width;
		originY += text.getLocalBounds().height / 2;
		break;
	case BottomRight:
		originX += text.getLocalBounds().width;
		originY += text.getLocalBounds().height;
		break;
	case Bottom:
		originX += text.getLocalBounds().width / 2;
		originY += text.getLocalBounds().height;
		break;
	case BottomLeft:
		originY += text.getLocalBounds().height;
		break;
	case Middle:
		originX += text.getLocalBounds().width / 2;
		originY += text.getLocalBounds().height / 2;
		break;
	}
}

void Label::UpdateSettings()
{
	text = sf::Text(label, *font, fontSize);
	text.setFillColor(color);
	text.setStyle(sf::Text::Regular);

	UpdateOrigin();
	text.setOrigin(sf::Vector2f((int)originX, (int)originY));

	text.setPosition(px, py);
}

void Label::Draw(sf::RenderTarget *renderTarget)
{
	renderTarget->draw(text);
}

void Label::UpdateAndDraw(sf::RenderTarget *renderTarget)
{
	renderTarget->draw(text);
}

void Label::InitSettings(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, sf::Color newColor)
{
	label = newLabel;

	font = newFont;
	fontSize = newFontSize;

	origin = newOrigin;

	px = newPx;
	py = newPy;

	color = newColor;
}

Label::Label(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, sf::Color newColor)
{
	InitSettings(newLabel, newFont, newFontSize, newOrigin, newPx, newPy, newColor);

	UpdateSettings();
}

Label::Label(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, sf::Color newColor)
{
	InitSettings(newLabel, newFont, newFontSize, newOrigin, 0, 0, newColor);

	UpdateSettings();
}

Label::Label() {}
