#include "Slider.hpp"

#include "../Event.hpp"

/*
template <class T>
void UI::Elements::Slider<T>::UpdateOrigin()
{
	
	//	TODO:
	//		Origin settings?
	
}

template <class T>
void UI::Elements::Slider<T>::InitSettings()
{

}

template <class T>
bool UI::Elements::Slider<T>::MouseHoverCheck()
{
	switch (direction)
	{
	case Horizontal:
		mouseOnBg =
			UI::Event::mouseInbound &&
			(UI::Event::mouseX >= px + sliderWidth / 2 && UI::Event::mouseX < px + width - sliderWidth / 2) &&
			(UI::Event::mouseY >= py + (height - bgThickness) / 2 && UI::Event::mouseY < py + (height + bgThickness) / 2);
		break;
	case Vertical:
		mouseOnBg =
			UI::Event::mouseInbound &&
			(UI::Event::mouseX >= px + (width - bgThickness) / 2 && UI::Event::mouseX < px + (width + bgThickness) / 2) &&
			(UI::Event::mouseY >= py + sliderHeight / 2 && UI::Event::mouseY < py + height - sliderHeight / 2);
		break;
	}

	mouseOnSlider =
		UI::Event::mouseInbound &&
		(UI::Event::mouseX >= px + sliderX - sliderWidth / 2 && UI::Event::mouseX < px + sliderX + sliderWidth / 2) &&
		(UI::Event::mouseY >= py + sliderY - sliderHeight / 2 && UI::Event::mouseY < py + sliderY + sliderHeight / 2);
	if (mouseOnSlider)
		mouseOnBg = false;

	return mouseOnBg || mouseOnSlider;
}

template <class T>
void UI::Elements::Slider<T>::ClampSliderPosition()
{
	int minX, maxX;
	int minY, maxY;
	switch (direction)
	{
	case Horizontal:
		minX = sliderWidth / 2;
		maxX = width - sliderWidth / 2;
		if (sliderX < minX)
			sliderX = minX;
		if (sliderX > maxX)
			sliderX = maxX;
		sliderY = height / 2;
		break;
	case Vertical:
		minY = sliderHeight / 2;
		maxY = height - sliderHeight / 2;
		if (sliderY < minY)
			sliderY = minY;
		if (sliderY > maxY)
			sliderY = maxY;
		sliderX = width / 2;
		break;
	}
}

template <class T>
void UI::Elements::Slider<T>::UpdateValue()
{
	int sliderLen;
	switch (direction)
	{
	case Horizontal:
		sliderLen = width - sliderWidth;
		value = minValue + (maxValue - minValue) * (sliderX - sliderWidth / 2) / sliderLen;
		break;
	case Vertical:
		sliderLen = height - sliderHeight;
		value = minValue + (maxValue - minValue) * ((height - sliderHeight / 2) - sliderY) / sliderLen;
		break;
	}
}

template <class T>
void UI::Elements::Slider<T>::UpdateSliderPosByValue()
{
	if (value < minValue)
		value = minValue;
	if (value > maxValue)
		value = maxValue;

	switch (direction)
	{
	case Horizontal:
		sliderX = sliderWidth / 2 + ((value - minValue) * (width - sliderWidth) / (maxValue - minValue));
		sliderY = height / 2;
		break;
	case Vertical:
		sliderX = width / 2;
		sliderY = height - sliderHeight / 2 - ((value - minValue) * (height - sliderHeight) / (maxValue - minValue));
		break;
	}
	ClampSliderPosition();
}

template <class T>
void UI::Elements::Slider<T>::UpdateSettings()
{
	if (value < minValue)
		value = minValue;
	if (value > maxValue)
		value = maxValue;

	sf::Vector2f bgSize;
	switch (direction)
	{
	case Horizontal:
		bgSize = sf::Vector2f(width - sliderWidth, bgThickness);
		bg = sf::RectangleShape(bgSize);
		bg.setPosition(px + sliderWidth / 2, py + (height - bgThickness) / 2);
		break;
	case Vertical:
		bgSize = sf::Vector2f(bgThickness, height - sliderHeight);
		bg = sf::RectangleShape(bgSize);
		bg.setPosition(px + (width - bgThickness) / 2, py + sliderHeight / 2);
		break;
	}

	UpdateSliderPosByValue();
	slider = sf::RectangleShape(sf::Vector2f(sliderWidth, sliderHeight));
	slider.setPosition(px + sliderX, py + sliderY);
}

template <class T>
void UI::Elements::Slider<T>::MouseEventHandler()
{
	if (mouseOnBg && UI::Event::mousePress)
	{
		switch (direction)
		{
		case Horizontal:
			sliderX = UI::Event::mouseX - px - sliderWidth / 2;
			break;
		case Vertical:
			sliderY = UI::Event::mouseY - py - sliderHeight / 2;
			break;
		}
		ClampSliderPosition();
		UpdateValue();
		UpdateSliderPosByValue();
		slider.setPosition(px + sliderX, py + sliderY);
	}

	if (mouseOnSlider && UI::Event::mousePress)
	{
		mousePressX = UI::Event::mouseX;
		mousePressY = UI::Event::mouseY;
		sliderOrigX = sliderX;
		sliderOrigY = sliderY;
		pressed = true;
	}

	if (UI::Event::mouseRelease || !UI::Event::mouseHold)
	{
		if (pressed)
		{
			UpdateSliderPosByValue();
			slider.setPosition(px + sliderX, py + sliderY);
		}

		pressed = false;
	}

	if (pressed)
	{
		switch (direction)
		{
		case Horizontal:
			sliderX = sliderOrigX + UI::Event::mouseX - mousePressX;
			break;
		case Vertical:
			sliderY = sliderOrigY + UI::Event::mouseY - mousePressY;
			break;
		}
		ClampSliderPosition();
		UpdateValue();
		slider.setPosition(px + sliderX, py + sliderY);
	}
}

template <class T>
void UI::Elements::Slider<T>::Update()
{
	bg.setFillColor(mouseOnBg ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
	slider.setFillColor((mouseOnSlider || pressed) ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
}

template <class T>
void UI::Elements::Slider<T>::Draw(sf::RenderTarget *renderTarget)
{
	renderTarget->draw(bg);
	renderTarget->draw(slider);
}

template <class T>
void UI::Elements::Slider<T>::UpdateAndDraw(sf::RenderTarget *renderTarget)
{
	Update();
	Draw(renderTarget);
}

template <class T>
UI::Elements::Slider<T>::Slider()
{

}
*/
