#include "Event.hpp"

namespace UI
{
	namespace Event
	{
		bool mouseInbound;

		int mouseAbsX, mouseAbsY, mouseOldAbsX, mouseOldAbsY;
		int mouseX, mouseY, mouseOldX, mouseOldY;
		int mouseVelX, mouseVelY;

		int mousePressAbsX, mousePressAbsY;

		bool mouseHold, mousePress, mouseRelease, mouseDragRelease;
		bool mouseScrollUp, mouseScrollDn;

		bool mouseDragging;
	}
}

using namespace UI;

void Event::UpdateBeforePoll()
{
	mouseOldX = mouseOldAbsX = mouseAbsX;
	mouseOldY = mouseOldAbsY = mouseAbsY;

	mousePress = false;
	mouseDragRelease = mouseRelease = false;

	mouseScrollUp = false;
	mouseScrollDn = false;

	if (!mouseHold)
		mouseDragging = false;
}

void Event::UpdateEvent(sf::Event event)
{
	if (event.type == sf::Event::MouseMoved)
	{
		mouseX = mouseAbsX = event.mouseMove.x;
		mouseY = mouseAbsY = event.mouseMove.y;
	}

	if (event.type == sf::Event::MouseButtonPressed)
	{
		mouseX = mousePressAbsX = mouseAbsX = event.mouseButton.x;
		mouseY = mousePressAbsY = mouseAbsY = event.mouseButton.y;

		if (!mouseHold)
			mousePress = true;

		mouseHold = true;
	}

	if (event.type == sf::Event::MouseButtonReleased)
	{
		mouseX = mouseAbsX = event.mouseButton.x;
		mouseY = mouseAbsY = event.mouseButton.y;

		if (mouseHold)
		{
			mouseRelease = true;
			if (mouseDragging)
				mouseDragRelease = true;
		}

		mouseDragging = false;
		mouseHold = false;
	}

	if (event.type == sf::Event::MouseWheelScrolled)
	{
		mouseX = mouseAbsX = event.mouseWheelScroll.x;
		mouseY = mouseAbsY = event.mouseWheelScroll.y;

		if (event.mouseWheelScroll.delta > 0)
			mouseScrollUp = true;
		else if (event.mouseWheelScroll.delta < 0)
			mouseScrollDn = true;
	}
}

void Event::UpdateAfterPoll()
{
	mouseVelX = mouseAbsX - mouseOldAbsX;
	mouseVelY = mouseAbsY - mouseOldAbsY;

	if (mouseHold)
	{
		int distX = mouseAbsX - mousePressAbsX;
		int distY = mouseAbsY - mousePressAbsY;
		float distFromPress =
			sqrt(distX * distX + distY * distY);

		if (distFromPress > mouseDragStartLenThreshold)
			mouseDragging = true;
	}
}
