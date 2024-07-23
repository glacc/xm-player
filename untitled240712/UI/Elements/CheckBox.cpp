#include "CheckBox.hpp"

#include "../Event.hpp"

using namespace UI::Elements;

void CheckBox::UpdateOrigin()
{
	originX = text.getLocalBounds().left;
	originY = text.getLocalBounds().top;
	switch (origin)
	{
	default:
	case TopLeft:
		chkBoxX = 0;
		chkBoxY = 0;

		originY += text.getLocalBounds().height / 2;

		txtX = fontSize * 3 / 2;
		txtY = fontSize / 2;
		break;
	case Left:
		chkBoxX = 0;
		chkBoxY = (height - fontSize) / 2;

		originY += text.getLocalBounds().height / 2;

		txtX = fontSize * 3 / 2;
		txtY = height / 2;
		break;
	case BottomLeft:
		chkBoxX = 0;
		chkBoxY = height - fontSize;

		originY += text.getLocalBounds().height / 2;

		txtX = fontSize * 3 / 2;
		txtY = height - fontSize / 2;
		break;
	case TopRight:
		chkBoxX = width - fontSize;
		chkBoxY = 0;

		originX += text.getLocalBounds().width;
		originY += text.getLocalBounds().height / 2;

		txtX = width - fontSize * 3 / 2;
		txtY = fontSize / 2;
		break;
	case Right:
		chkBoxX = width - fontSize;
		chkBoxY = (height - fontSize) / 2;

		originX += text.getLocalBounds().width;
		originY += text.getLocalBounds().height / 2;

		txtX = width - fontSize * 3 / 2;
		txtY = height / 2;
		break;
	case BottomRight:
		chkBoxX = width - fontSize;
		chkBoxY = height - fontSize;

		originX += text.getLocalBounds().width;
		originY += text.getLocalBounds().height / 2;

		txtX = width - fontSize * 3 / 2;
		txtY = height - fontSize / 2;
		break;
	}
}

void CheckBox::InitSettings()
{
	origin = Left;
}

bool CheckBox::MouseHoverCheck()
{
	mouseHover =
		UI::Event::mouseInbound &&
		(UI::Event::mouseX >= px + chkBoxX && UI::Event::mouseX < px + chkBoxX + fontSize) &&
		(UI::Event::mouseY >= py + chkBoxY && UI::Event::mouseY < py + chkBoxY + fontSize);
	return mouseHover;
}

void CheckBox::MouseEventHandler()
{
	if (mouseHover && UI::Event::mousePress)
		pressed = true;

	if (UI::Event::mouseRelease)
	{
		if (mouseHover && pressed)
		{
			checked = !checked;
			if (onClick != NULL)
				onClick();
		}

		pressed = false;
	}

	if (!UI::Event::mouseHold)
		pressed = false;
}

void CheckBox::UpdateSettings()
{
	text = sf::Text(label, *font, fontSize);

	UpdateOrigin();

	bg.setPosition(px + chkBoxX, py + chkBoxY);
	bg.setSize(sf::Vector2f(fontSize, fontSize));

	int chkMarkSize = fontSize * 2 / 4;
	int chkMarkOffset = (fontSize - chkMarkSize) / 2;
	chkMark.setPosition(px + chkBoxX + chkMarkOffset, py + chkBoxY + chkMarkOffset);
	chkMark.setSize(sf::Vector2f(chkMarkSize, chkMarkSize));

	text.setOrigin((int)originX, (int)originY);
	text.setPosition(px + txtX, py + txtY);
	text.setFillColor(sf::Color(0x66, 0x66, 0x66));
}

void CheckBox::Update()
{
	bg.setFillColor((mouseHover || pressed) ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
}

void CheckBox::Draw(sf::RenderTarget *renderTarget)
{
	renderTarget->draw(bg);
	/*
	*	TODO:
	*		Group of check boxes
	*/
	if (checked)
		renderTarget->draw(chkMark);
	renderTarget->draw(text);
}

void CheckBox::UpdateAndDraw(sf::RenderTarget *renderTarget)
{
	Update();
	Draw(renderTarget);
}

CheckBox::CheckBox()
{
	InitSettings();
}
