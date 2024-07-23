#include "Button.hpp"
#include "../Event.hpp"

using namespace UI::Elements;

void Button::UpdateOrigin()
{
	switch (origin)
	{
	case Left:
		originX = 0;
		originY = height / 2;
		break;
	case TopLeft:
		originX = 0;
		originY = 0;
		break;
	case Top:
		originX = width / 2;
		originY = 0;
		break;
	case TopRight:
		originX = width;
		originY = 0;
		break;
	case Right:
		originX = width;
		originY = height / 2;
		break;
	case BottomRight:
		originX = width;
		originY = height;
		break;
	case Bottom:
		originX = width / 2;
		originY = height;
		break;
	case BottomLeft:
		originX = 0;
		originY = height;
		break;
	case Middle:
		originX = width / 2;
		originY = height / 2;
		break;
	}
}

void Button::UpdateSettings()
{
	text = sf::Text(label, *font, fontSize);

	UpdateOrigin();

	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Regular);
	text.setOrigin(sf::Vector2f((int)(text.getLocalBounds().left + text.getLocalBounds().width / 2), (int)(text.getLocalBounds().top + text.getLocalBounds().height / 2)));
	text.setPosition(px + width / 2 - (int)originX, py + height / 2 - (int)originY);

	bg = sf::RectangleShape(sf::Vector2f((float)width, (float)height));
	bg.setPosition(sf::Vector2f(px - (int)originX, py - (int)originY));
}

bool Button::MouseHoverCheck()
{
	mouseHover =
		UI::Event::mouseInbound &&
		(UI::Event::mouseX >= px - (int)originX && UI::Event::mouseX < px - (int)originX + width) &&
		(UI::Event::mouseY >= py - (int)originY && UI::Event::mouseY < py - (int)originY + height);

	return mouseHover;
}

void Button::MouseEventHandler()
{
	if (mouseHover && UI::Event::mousePress)
	{
		if (onPress != NULL)
			onPress();

		pressed = true;
	}

	if (UI::Event::mouseRelease)
	{
		if (mouseHover && pressed)
		{
			if (onClick != NULL)
				onClick();
		}

		pressed = false;
	}

	if (!UI::Event::mouseHold)
		pressed = false;
}

void Button::Update()
{
	bg.setFillColor((mouseHover || pressed) ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
}

void Button::Draw(sf::RenderTarget *renderTarget)
{
	renderTarget->draw(bg);
	renderTarget->draw(text);
}

void Button::UpdateAndDraw(sf::RenderTarget *renderTarget)
{
	Update();
	Draw(renderTarget);
}

void Button::InitSettings(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)())
{
	label = newLabel;

	font = newFont;
	fontSize = newFontSize;

	origin = newOrigin;
	px = newPx;
	py = newPy;
	width = newWidth;
	height = newHeight;

	onClick = newOnClick;
}

Button::Button(std::string newLabel, sf::Font *newFont, int newFontSize, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)())
{
	InitSettings(newLabel, newFont, newFontSize, TopLeft,
		newPx, newPy, newWidth, newHeight, newOnClick);

	UpdateSettings();
}

Button::Button(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newPx, int newPy, int newWidth, int newHeight, void (*newOnClick)())
{
	InitSettings(newLabel, newFont, newFontSize, newOrigin,
		newPx, newPy, newWidth, newHeight, newOnClick);

	UpdateSettings();
}

Button::Button(std::string newLabel, sf::Font *newFont, int newFontSize, int newWidth, int newHeight, void (*newOnClick)())
{
	InitSettings(newLabel, newFont, newFontSize, TopLeft,
		0, 0, newWidth, newHeight, newOnClick);

	UpdateSettings();
}

Button::Button(std::string newLabel, sf::Font *newFont, int newFontSize, Origin newOrigin, int newWidth, int newHeight, void (*newOnClick)())
{
	InitSettings(newLabel, newFont, newFontSize, newOrigin,
		0, 0, newWidth, newHeight, newOnClick);

	UpdateSettings();
}

Button::Button() {}
