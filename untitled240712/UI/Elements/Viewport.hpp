#pragma once

#include "../Element.hpp"
#include "../Screen.hpp"
#include "../Event.hpp"
#include <SFML/Graphics.hpp>

namespace UI
{
	namespace Elements
	{
		class Viewport : public Element
		{
		private:
			void CheckRenderTexture();

			void UpdateOrigin() override;

			void InitSettings(UI::Screen *initScreen, int newPx, int newPy, int newWidth, int newHeight, Viewport *newParentViewport);

		public:
			int px, py, width, height;
			bool mouseInbound = false;

			Viewport *parentViewport = NULL;
			sf::RenderTarget *renderTarget = NULL;
			UI::Screen *screen = NULL;
			sf::RenderTexture *viewportRenderTexture = NULL;

			sf::Color bgColor = sf::Color(0xDF, 0xDF, 0xDF);

			void UpdateSettings() override;

			bool MouseHoverCheck() override;

			void Update() override;

			void Draw(sf::RenderTarget *rt = NULL) override;

			void UpdateAndDraw(sf::RenderTarget *rt = NULL) override;

			Viewport(UI::Screen *initScreen, int newPx = 0, int newPy = 0, int newWidth = 0, int newHeight = 0, Viewport *newParentViewport = NULL);

			Viewport(UI::Screen *initScreen, Viewport *newParentViewport = NULL);

			Viewport();

			~Viewport();
		};
	}
}
