#include "PatternViewer.hpp"

#include "MainScreen.hpp"
#include "../Utils/DrawTextMonospace.hpp"

#include "../Global.hpp"

namespace Screens
{
	namespace PatternViewer
	{
		const std::string noteChar[12] =
		{
			"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
		};

		const std::string volCmdChar[10] =
		{
			"-", "+", "d", "u", "S", "V", "P", "L", "R", "M"
		};

		const std::string hexChar[36] =
		{
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
		};

		PatternViewerScreen patternViewerScreen;
	}
}

using namespace Screens;

void PatternViewer::PatternViewerScreen::Init()
{
	xmPlayer = &Screens::Main::xmPlayer;

	bgColor = sf::Color(0xEF, 0xEF, 0xEF);

	int sizeX = renderTarget->getSize().x;
	int sizeY = renderTarget->getSize().y;
	int centerX = sizeX / 2;
	int centerY = sizeY / 2;

	cursorRect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 192));

	std::string strInstruction = "Press \"Open\" to open module files (.xm)\n\n";
	strInstruction += "Use mouse wheel to scroll horizontally.\n\n";
	strInstruction += "Scroll or drag the order table on the top left to seek.\n\n";
	strInstruction += "Click the scopes to mute/unmute channels.\n\n";
	strInstruction += "Click this window to toggle detailed pattern view.\n\n\n\n";
	strInstruction += "(Work in progress)\nGlacc 2024.10.13 qwq";
	lblInstruction = UI::Elements::Label(strInstruction,
		UI::Globals::font, 24, UI::Element::TopLeft, 64, 64);

	elements.push_back(&lblInstruction);
}

void PatternViewer::PatternViewerScreen::UpdateCustom()
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
			{
				if (chOffset > 0)
					chOffset --;
			}

			if (UI::Event::mouseScrollDn)
			{
				Glacc::XMPlayer::Stat stat;
				xmPlayer->GetCurrentStat(stat);
				if (chOffset < stat.numOfChannels - 1)
					chOffset ++;
			}
		}
	}
	
	if (UI::Event::mousePress && mouseInbound)
		mouseHold = true;

	if (UI::Event::mouseRelease)
	{
		if (mouseHold && mouseInbound && !UI::Event::mouseDragRelease)
			detailedView = !detailedView;

		mouseHold = false;
	}
	
}

void PatternViewer::PatternViewerScreen::DrawCustom()
{
	if (xmPlayer == NULL)
		return;

	int sizeX = renderTarget->getSize().x;
	int sizeY = renderTarget->getSize().y;
	int centerX = sizeX / 2;
	int centerY = sizeY / 2;

	const int fontSize = 18;
	const int fontWidth = 8;
	const int spacing = 2;

	// Performance issue here.
	// Would become obvious in Debug.
	if (xmPlayer->IsLoaded() && Glacc::Global::monoFont != NULL)
	{
		cursorRect.setSize(sf::Vector2f(sizeX, fontSize));
		cursorRect.setPosition(0, centerY - fontSize / 2);
		renderTarget->draw(cursorRect);

		int ofsIncY = (fontSize + spacing);
		int relOffset = (centerY / ofsIncY) + 1;
		int relRowMin = -relOffset + 1;
		int relRowMax = relOffset + 1;
		int relRow = relRowMin;
		while (relRow <= relRowMax)
		{
			int ofsX = spacing;
			int ofsY = centerY + ofsIncY * (relRow - 1);

			std::string tempStr = "  ";

			std::string rowStr = "  ";

			bool inCurrentPattern = false;

			int absRow = stat.row + relRow - 1;
			if (absRow >= 0 && absRow < stat.patternLen)
			{
				rowStr = hexChar[(absRow >> 4) & 0x0F];
				rowStr += hexChar[absRow & 0x0F];;

				if (chOffset == 0)
					Utils::DrawTextMonospace(renderTarget, rowStr, ofsX, ofsY, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont);

				inCurrentPattern = true;
			}

			if (chOffset == 0)
				ofsX += fontWidth * 3;
			
			int i = chOffset;
			while (i < stat.numOfChannels)
			{
				if (ofsX > sizeX)
					break;

				Glacc::XMPlayer::Note note = xmPlayer->GetNoteExt(stat.pos, absRow, i);

				if (note.note != 255)
				{
					tempStr = "...";

					if (detailedView)
					{
						if (note.note == 0)
						{
							tempStr = "...";
						}
						else if (note.note == 97)
						{
							// Note off
							tempStr = "===";
						}
						else
						{
							int noteNum = note.note % 12;
							int noteOct = note.note / 12;
							tempStr = noteChar[noteNum] + std::to_string(noteOct);
						}
						tempStr += " ";

						if (note.instrument)
						{
							tempStr += hexChar[(note.instrument >> 4) & 0x0F];
							tempStr += hexChar[note.instrument & 0x0F];
						}
						else
							tempStr += "..";
						tempStr += " ";

						if (note.volCmd)
						{
							if (note.volCmd >= 0x10)
							{
								if (note.volCmd <= 0x50)
								{
									if ((note.volCmd - 0x10) < 10)
										tempStr += " ";

									tempStr += std::to_string(note.volCmd - 0x10);
								}
								else
								{
									int volCmdTyp = (note.volCmd >> 4) & 0x0F;
									int volCmdPara = note.volCmd & 0x0F;
									tempStr += volCmdChar[volCmdTyp - 6];
									tempStr += hexChar[volCmdPara];
								}

								tempStr += " ";
							}
							else
								tempStr += ".. ";
						}
						else
							tempStr += ".. ";

						if (note.effect < 10)
						{
							if (note.effect || note.parameter)
								tempStr += '0' + note.effect;
							else
								tempStr += '.';
						}
						else
							tempStr += ('A' - 10 + note.effect);

						if (note.parameter || note.effect)
						{
							tempStr += hexChar[(note.parameter >> 4) & 0x0F];
							tempStr += hexChar[note.parameter & 0x0F];
						}
						else
							tempStr += "..";
					}
					else
					{
						if (note.note != 0)
						{
							if (note.note == 97)
							{
								// Note off
								tempStr = "===";
							}
							else
							{
								int noteNum = note.note % 12;
								int noteOct = note.note / 12;
								tempStr = noteChar[noteNum] + std::to_string(noteOct);
							}
						}
						else if (note.instrument)
						{
							tempStr = "i";
							tempStr += hexChar[(note.instrument >> 4) & 0x0F];
							tempStr += hexChar[note.instrument & 0x0F];
						}
						else if (note.volCmd)
						{

							if (note.volCmd >= 0x10)
							{
								if (note.volCmd <= 0x50)
								{
									tempStr = "v";
									if ((note.volCmd - 0x10) < 10)
										tempStr += " ";

									tempStr += std::to_string(note.volCmd - 0x10);
								}
								else
								{
									tempStr = " ";
									int volCmdTyp = (note.volCmd >> 4) & 0x0F;
									int volCmdPara = note.volCmd & 0x0F;
									tempStr += volCmdChar[volCmdTyp - 6];
									tempStr += hexChar[volCmdPara];
								}
							}
							else
								tempStr += ".. ";
						}
						else if (note.effect || note.parameter)
						{
							if (note.effect < 10)
							{
								if (note.effect || note.parameter)
									tempStr = '0' + note.effect;
								else
									tempStr = '.';
							}
							else
								tempStr = ('A' - 10 + note.effect);

							if (note.parameter || note.effect)
							{
								tempStr += hexChar[(note.parameter >> 4) & 0x0F];
								tempStr += hexChar[note.parameter & 0x0F];
							}
							else
								tempStr += "..";
						}
						else
							tempStr = "...";
					}
				}
				else
					tempStr = detailedView ? "             " : "   ";

				if (i < stat.numOfChannels - 1)
					tempStr += "|";

				sf::Color txtColor(0x66, 0x66, 0x66, inCurrentPattern ? 255 : 127);
				Utils::DrawTextMonospace(renderTarget, tempStr, ofsX, ofsY, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont, txtColor);
				ofsX += fontWidth * (tempStr.length());

				/*
				sf::Text txt(tempStr, *Glacc::Global::monoFont, fontSize);
				txt.setOrigin(txt.getLocalBounds().left, txt.getLocalBounds().height / 2);
				txt.setFillColor(sf::Color(0x66, 0x66, 0x66));
				txt.setPosition((int)ofsX, (int)ofsY);
				renderTarget->draw(txt);

				ofsX += txt.getLocalBounds().fontWidth - txt.getLocalBounds().left + spacing;
				*/

				i ++;
			}

			ofsX += fontWidth;
			Utils::DrawTextMonospace(renderTarget, rowStr, ofsX, ofsY, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont);

			relRow ++;
		}

		// Channel Number
		//sf::Vector2f bgSize(fontWidth * 2 + spacing * 2, fontSize);
		int i = 0;
		while (i < stat.numOfChannels - chOffset)
		{
			std::string chNumStr = std::to_string(i + 1 + chOffset);

			int chWidth = (detailedView ? 14 : 4) * fontWidth;
			int px = i * chWidth + (chOffset == 0 ? (fontWidth * 3) : 0);
			int py = fontSize / 2;
			if (px > (int)renderTarget->getSize().x)
				break;

			sf::Vector2f bgSize(fontWidth * chNumStr.length() + spacing * 2, fontSize);
			sf::RectangleShape chNumBg(bgSize);
			chNumBg.setFillColor(sf::Color(0x66, 0x66, 0x66));
			chNumBg.setPosition(px, 0);

			/*
			sf::Text chNumTxt(chNumStr, *Glacc::Global::monoFont, fontSize);
			chNumTxt.setFillColor(sf::Color(0xEF, 0xEF, 0xEF));
			chNumTxt.setOrigin(
				chNumTxt.getLocalBounds().left,
				(int)(chNumTxt.getLocalBounds().top + chNumTxt.getLocalBounds().height / 2)
			);
			chNumTxt.setPosition(px + spacing, py);
			*/

			renderTarget->draw(chNumBg);
			//renderTarget->draw(chNumTxt);

			Utils::DrawTextMonospace(renderTarget, chNumStr, px + spacing, py, fontSize, fontWidth, sizeX, sizeY, Glacc::Global::monoFont, sf::Color(0xEF, 0xEF, 0xEF));

			i ++;
		}
	}
}

PatternViewer::PatternViewerScreen::PatternViewerScreen() {}
