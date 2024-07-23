#include "Screen.hpp"

using namespace UI;

void Screen::InitCheck()
{
	if (init)
		return;

	Init();

	init = true;
}

void Screen::Init() {}

void Screen::MouseHoverCheck()
{
	mouseHoverOn = NULL;

	Element *prevElem = NULL;
	for (int i = 0; i < elements.size(); i ++)
	{
		Element *currElem = elements[i];

		if (currElem->visible)
		{
			if (currElem->MouseHoverCheck())
			{
				if (prevElem != NULL)
					prevElem->mouseHover = false;

				prevElem = currElem;
				mouseHoverOn = currElem;
			}
		}
	}
}

void Screen::UpdateElements()
{
	for (int i = 0; i < elements.size(); i ++)
	{
		Element *currElem = elements[i];

		if (currElem->visible)
		{
			currElem->MouseEventHandler();
			currElem->Update();
		}
	}
}

void Screen::DrawElements()
{
	for (int i = 0; i < elements.size(); i ++)
	{
		Element *currElem = elements[i];

		if (currElem->visible)
			currElem->Draw(renderTarget);
	}
}

void Screen::UpdateSettings() {}

void Screen::Update()
{
	UpdateCustom();
	UpdateElements();
}

void Screen::UpdateCustom() {}

void Screen::Draw()
{
	DrawCustom();
	DrawElements();
}

void Screen::DrawCustom() {}

void Screen::UpdateAndDraw()
{
	if (renderTarget == NULL)
		return;

	Update();

	DrawCustom();
	DrawElements();
}

Screen::Screen() {}
