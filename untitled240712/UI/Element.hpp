#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

namespace UI
{
	class Element
	{
	protected:
		float originX, originY;

		virtual void UpdateOrigin();

	public:
		enum Origin
		{
			Left = 0b0001,
			TopLeft = 0b0010,
			Top = 0b0110,
			TopRight = 0b1010,
			Right = 0b1001,
			BottomRight = 0b1000,
			Bottom = 0b0100,
			BottomLeft = 0b0000,
			Middle = 0b1111
		};

		Origin origin;

		bool visible = true;

		bool mouseHover = false;

		/// <summary>
		/// Update settings after changing parameters
		/// </summary>
		virtual void UpdateSettings();

		virtual bool MouseHoverCheck();

		virtual void MouseEventHandler();

		virtual void Update();

		virtual void Draw(sf::RenderTarget *renderTarget);

		virtual void UpdateAndDraw(sf::RenderTarget *renderTarget);

		Element();
	};
}