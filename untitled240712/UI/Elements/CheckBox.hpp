#pragma once

#include "../Element.hpp"

namespace UI
{
	namespace Elements
	{
		class CheckBox : public Element
		{
		private:
			sf::Text text;
			sf::RectangleShape bg;
			sf::RectangleShape chkMark;

			void UpdateOrigin() override;
			
			void InitSettings();

		public:
			std::string label;
			sf::Font *font;
			int fontSize;

			int px, py;
			int width, height;

			int txtX, txtY;
			int chkBoxX, chkBoxY;

			bool checked = false;

			void (*onClick)() = NULL;

			bool pressed = false;

			void UpdateSettings() override;

			bool MouseHoverCheck() override;

			void MouseEventHandler() override;

			void Update() override;

			void Draw(sf::RenderTarget *renderTarget) override;

			void UpdateAndDraw(sf::RenderTarget *renderTarget) override;

			CheckBox();
		};
	}
}