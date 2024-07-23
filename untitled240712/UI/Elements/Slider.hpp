#pragma once

#include "../Element.hpp"
#include "../Event.hpp"

namespace UI
{
	namespace Elements
	{
		template <class T>
		class Slider : public Element
		{
		private:
			sf::Text text;
			sf::RectangleShape bg;
			sf::RectangleShape slider;

			void UpdateOrigin() override { }

			void InitSettings() { }

			void UpdateSliderPosition()
			{
				slider.setPosition(px + sliderX - sliderWidth / 2, py + sliderY - sliderHeight / 2);
			}

			void ClampSliderPosition()
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

			void ClampValue()
			{
				if (value < minValue)
					value = minValue;
				if (value > maxValue)
					value = maxValue;
			}

			void UpdateValue()
			{
				T valueOld = value;

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

				ClampValue();

				if (value != valueOld)
				{
					if (onValueChange != NULL)
						onValueChange();
				}
			}

			void UpdateSliderPosByValue()
			{
				ClampValue();

				T valueRange = maxValue - minValue;
				if (valueRange == 0)
					valueRange = 1;
				switch (direction)
				{
				case Horizontal:
					sliderX = sliderWidth / 2 + ((value - minValue) * (width - sliderWidth) / valueRange);
					sliderY = height / 2;
					break;
				case Vertical:
					sliderX = width / 2;
					sliderY = height - sliderHeight / 2 - ((value - minValue) * (height - sliderHeight) / valueRange);
					break;
				}

				ClampSliderPosition();
				UpdateSliderPosition();
			}

		public:
			T value;
			T maxValue, minValue;
			T valInc;

			std::string label;
			sf::Font *font;
			int fontSize;

			enum Direction
			{
				Horizontal,
				Vertical
			};
			Direction direction = Horizontal;

			int px, py;
			int width, height;
			int bgThickness;

			int sliderX, sliderY;
			int sliderOrigX, sliderOrigY;
			int sliderWidth, sliderHeight;

			int mousePressX, mousePressY;

			bool mouseOnBg = false;
			bool mouseOnSlider = false;
			bool pressed = false;

			void (*onValueChange)() = NULL;

			void UpdateSettings() override
			{
				ClampValue();

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

				slider = sf::RectangleShape(sf::Vector2f(sliderWidth, sliderHeight));
				UpdateSliderPosByValue();
			}

			bool MouseHoverCheck() override
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

				mouseHover = mouseOnBg || mouseOnSlider;
				return mouseHover;
			}

			void MouseEventHandler() override
			{
				if (mouseHover && UI::Event::mouseScrollUp)
				{
					value += valInc;
					UpdateSliderPosByValue();
					if (onValueChange != NULL)
						onValueChange();
				}
				if (mouseHover && UI::Event::mouseScrollDn)
				{
					value -= valInc;
					UpdateSliderPosByValue();
					if (onValueChange != NULL)
						onValueChange();
				}

				if (mouseOnBg && UI::Event::mousePress)
				{
					switch (direction)
					{
					case Horizontal:
						sliderX = UI::Event::mouseX - px;
						break;
					case Vertical:
						sliderY = UI::Event::mouseY - py;
						break;
					}
					ClampSliderPosition();
					UpdateValue();
					UpdateSliderPosByValue();
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
						UpdateSliderPosByValue();

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
					UpdateSliderPosition();
				}
			}

			void Update() override
			{
				bg.setFillColor(mouseOnBg ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
				//slider.setFillColor((mouseOnSlider || pressed) ? sf::Color(0x00, 0x00, 0x00, 96) : sf::Color(0x00, 0x00, 0x00, 64));
				slider.setFillColor((mouseOnSlider || pressed) ? sf::Color(0x66, 0x66, 0x66) : sf::Color(0x88, 0x88, 0x88));
			}

			void Draw(sf::RenderTarget *renderTarget) override
			{
				renderTarget->draw(bg);
				renderTarget->draw(slider);
			}

			void UpdateAndDraw(sf::RenderTarget *renderTarget) override
			{
				Update();
				Draw(renderTarget);
			}

			Slider() {}
		};
	}
}

#include "Slider.cpp"
