#pragma once

#include <SFML/Graphics.hpp>

namespace UI
{
	namespace Event
	{
		const int mouseDragStartLenThreshold = 10;

		/// <summary>
		/// TODO:
		///		Relative mouse X/Y for different viewports, viewport in viewport etc.
		/// </summary>
		// Is mouse inbound?
		extern bool mouseInbound;
		// Absolute mouse X/Y
		extern int mouseAbsX, mouseAbsY, mouseOldAbsX, mouseOldAbsY;
		// Relative mouse X/Y
		extern int mouseX, mouseY, mouseOldX, mouseOldY;
		// Mouse velocity X/Y
		extern int mouseVelX, mouseVelY;
		
		extern int mousePressAbsX, mousePressAbsY;

		extern bool mouseHold, mousePress, mouseRelease, mouseDragRelease;
		extern bool mouseScrollUp, mouseScrollDn;
		extern bool mouseDragging;

		/// <summary>
		///	Call UpdateBeforePoll() before event polling loop,
		/// </summary>
		void UpdateBeforePoll();

		/// <summary>
		/// Call UpdateEvent() inside event polling loop.
		/// </summary>
		/// <param name="event"> - event from event polling loop</param>
		void UpdateEvent(sf::Event event);

		/// <summary>
		/// Call UpdateAfterPoll() after event polling loop,
		/// </summary>
		void UpdateAfterPoll();
	}
}
