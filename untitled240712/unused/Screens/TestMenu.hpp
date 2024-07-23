#pragma once

#include "../UI/Screen.hpp"
#include "../UI/Elements.hpp"
#include "../UI/Globals.hpp"

namespace Screens
{
	namespace TestMenu
	{
		void ElemOverlayTest();

		class TestMenuScreen : public UI::Screen
		{
		public:
			UI::Elements::Button btnChangeColor;
			UI::Elements::Button btnToggleSound;
			UI::Elements::Button btnOverlayTest;
			UI::Elements::Button btnExit;
			UI::Elements::Label lblTitle;
			UI::Elements::Label lblDebug;

			void (*colorChangeFunc)();

			void (*soundToggleFunc)();

			void (*exitFunc)();

			void Init() override;

			void UpdateCustom() override;

			void DrawCustom() override;

			TestMenuScreen();
		};

		extern TestMenuScreen testMenuScreen;
	}
}
