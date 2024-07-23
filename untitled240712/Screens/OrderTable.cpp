#include "OrderTable.hpp"

#include "../Global.hpp"

#include "../Utils/DrawTextMonospace.hpp"

#include "MainScreen.hpp"

namespace Screens
{
	namespace OrderTable
	{
		const std::string hexChar[36] =
		{
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
		};

		OrderTable orderTable;
	}
}

using namespace Screens;

void OrderTable::OrderTable::Init()
{
	xmPlayer = &Screens::Main::xmPlayer;

	bgColor = sf::Color(0xEF, 0xEF, 0xEF);

	int sizeX = renderTarget->getSize().x;
	int sizeY = renderTarget->getSize().y;
	int centerX = sizeX / 2;
	int centerY = sizeY / 2;

	cursorRect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 192));
}

void OrderTable::OrderTable::UpdateCustom()
{
	if (xmPlayer == NULL)
		return;

	if (xmPlayer->IsLoaded())
		xmPlayer->GetCurrentStat(stat);

	if (mouseInbound)
	{
		if (xmPlayer->IsLoaded())
		{
			if (UI::Event::mouseScrollUp)
				xmPlayer->SetPos(stat.pos - 1);

			if (UI::Event::mouseScrollDn)
				xmPlayer->SetPos(stat.pos + 1);
		}

		if (UI::Event::mousePress)
		{
			movY = (fontSize + spacing) / 2;
			drag = true;
		}
	}
	
	if (!UI::Event::mouseHold)
		drag = false;

	if (drag)
	{
		if (xmPlayer->IsLoaded())
		{
			bool setPos = false;

			movY += UI::Event::mouseVelY;
			int lineHeight = fontSize + spacing;
			int pos = stat.pos;

			while (movY > lineHeight)
			{
				pos --;
				setPos = true;

				movY -= lineHeight;
			}
			while (movY < 0)
			{
				pos ++;
				setPos = true;

				movY += lineHeight;
			}

			if (setPos)
				xmPlayer->SetPos(pos);
		}
	}
}

void OrderTable::OrderTable::DrawCustom()
{
	if (xmPlayer == NULL)
		return;

	int sizeX = renderTarget->getSize().x;
	int sizeY = renderTarget->getSize().y;
	int centerX = sizeX / 2;
	int centerY = sizeY / 2;

	if (Glacc::Global::monoFont != NULL)
	{
		cursorRect.setSize(sf::Vector2f(sizeX, fontSize));
		cursorRect.setPosition(0, centerY - fontSize / 2);
		renderTarget->draw(cursorRect);

		int ofsIncY = (fontSize + spacing);
		int relOffset = (centerY / ofsIncY) + 1;
		int relPosMin = -relOffset + 1;
		int relPosMax = relOffset + 1;
		int relPos = relPosMin;
		while (relPos <= relPosMax)
		{
			int ofsX = centerX - totalWidth / 2;
			int ofsY = centerY + ofsIncY * (relPos - 1);

			int absPos = stat.pos + relPos - 1;
			if (absPos >= 0 && absPos <= 0xFF)
			{
				std::string posStr;
				posStr = hexChar[(absPos >> 4) & 0x0F];
				posStr += hexChar[absPos & 0x0F];;
				Utils::DrawTextMonospace(renderTarget, posStr, ofsX, ofsY, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont);
				ofsX += fontWidth * 3;
				
				if (absPos < stat.songLength && xmPlayer->IsLoaded())
				{
					uint8_t patNum = stat.patternOrder[absPos];

					std::string patStr;
					posStr = hexChar[(patNum >> 4) & 0x0F];
					posStr += hexChar[patNum & 0x0F];
					Utils::DrawTextMonospace(renderTarget, posStr, ofsX, ofsY, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont);
				}
			}

			relPos ++;
		}
	}
}
