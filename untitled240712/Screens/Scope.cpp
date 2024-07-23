#include "Scope.hpp"

#include "MainScreen.hpp"

#include "../Global.hpp"

#include <memory>

namespace Screens
{
	namespace Scope
	{
		Scope scope;
	}
}

using namespace Screens::Scope;

void Scope::Init()
{
	xmPlayer = &Main::xmPlayer;

	bgColor = sf::Color(0xEF, 0xEF, 0xEF);

	UpdateSettings();
}

void Scope::UpdateSettings()
{
	if (renderTarget == NULL)
		return;

	if (scopeTexture == NULL)
		scopeTexture = new sf::Texture();

	width = renderTarget->getSize().x;
	height = renderTarget->getSize().y;

	bufferSize = width * height;
	if (pixelBuffer == NULL)
	{
		if (bufferSize > 0)
			pixelBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bufferSize);
	}
	else
	{
		free(pixelBuffer);
		if (bufferSize > 0)
			pixelBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bufferSize);
		else
			pixelBuffer = NULL;
	}

	if (pixelBuffer != NULL)
		scopeTexture->create(width, height);
}

void Scope::UpdateCustom()
{
	if (xmPlayer != NULL)
	{
		xmPlayer->GetCurrentStat(stat);

		int numOfCh = xmPlayer->scopes.numOfCh;
		float scopeWidth = width / (float)numOfCh;
		narrow = scopeWidth < 60;


		int i = 1;
		while (i <= numOfCh)
		{
			float endX = scopeWidth * i;
			borders[i] = endX;

			i ++;
		}

		if (mouseInbound)
		{
			if (UI::Event::mousePress)
				mouseHold = true;

			if (UI::Event::mouseRelease)
			{
				if (mouseInbound && !UI::Event::mouseDragRelease)
				{
					int i = 0;
					while (i < numOfCh)
					{
						if (UI::Event::mouseX >= borders[i] && UI::Event::mouseX < borders[i + 1])
							stat.channels[i].muted = !stat.channels[i].muted;

						i ++;
					}
				}

				mouseHold = false;
			}
		}

		if (!UI::Event::mouseHold)
			mouseHold = false;
	}
}

void Scope::DrawCustom()
{
	if (xmPlayer != NULL)
	{
		if (xmPlayer->IsLoaded())
		{
			int numOfCh = xmPlayer->scopes.numOfCh;
			int bufSize = xmPlayer->scopes.sizePerCh;

			if (pixelBuffer != NULL)
			{
				memset(pixelBuffer, 0, sizeof(uint32_t) * bufferSize);
				int i = 0;
				while (i < numOfCh)
				{
					int startX = borders[i];
					int sizeX = borders[i + 1] - 1 - startX;
					int endX = borders[i + 1] - 1;

					if (i < numOfCh - 1)
					{
						int offset = endX;
						int j = 0;
						while (j < height)
						{
							if (offset > bufferSize || offset < 0)
								offset = 0;
							pixelBuffer[offset] = 0xFF666666;
							offset += width;
							j ++;
						}
					}

					if (stat.channels[i].muted)
					{
						i ++;
						continue;
					}

					int8_t *scopeData = xmPlayer->scopes.scopeData + bufSize * i;

					int oldX = startX;
					int oldY = height / 2;
					int j = 0;
					while (j < bufSize)
					{
						int sd = scopeData[j];
						int ofsX = startX + (sizeX * j / bufSize);
						if (ofsX >= width)
							break;
						while (oldX < ofsX)
						{
							int offset = oldY * width + oldX;
							if (offset > bufferSize || offset < 0)
								offset = 0;
							pixelBuffer[offset] = 0xFF666666;
							oldX ++;
						}
						int ofsY = (height / 2) - (height / 2) * sd / 128;
						oldY = ofsY;
						int offset = ofsY * width + ofsX;
						if (offset > bufferSize || offset < 0)
							offset = 0;

						// Pay attention to the endianness
						pixelBuffer[offset] = 0xFF666666;

						j ++;
					}

					i ++;
				}

				if (scopeTexture != NULL)
				{
					scopeTexture->update((uint8_t *)pixelBuffer);
					sf::Sprite scopeSprite(*scopeTexture);
					renderTarget->draw(scopeSprite);
				}
			}

			int i = 0;
			while (i < xmPlayer->scopes.numOfCh)
			{
				sf::Text chNum(std::to_string(i + 1), *Glacc::Global::monoFont, 18);
				chNum.setFillColor(sf::Color(0x66, 0x66, 0x66));
				chNum.setOrigin(chNum.getLocalBounds().left, chNum.getLocalBounds().top);
				chNum.setPosition(borders[i] + 4, 4);
				renderTarget->draw(chNum);

				if (stat.channels[i].muted)
				{
					sf::Text mutedTxt(narrow ? "M" : "Muted", *Glacc::Global::monoFont, 24);
					mutedTxt.setFillColor(sf::Color(0x66, 0x66, 0x66));
					mutedTxt.setOrigin(
						mutedTxt.getLocalBounds().left + mutedTxt.getLocalBounds().width / 2,
						mutedTxt.getLocalBounds().top + mutedTxt.getLocalBounds().height / 2
					);
					mutedTxt.setPosition((borders[i] + borders[i + 1]) / 2, height / 2);
					renderTarget->draw(mutedTxt);
				}

				i ++;
			}
		}
	}
}

Scope::~Scope()
{
	if (pixelBuffer != NULL)
	{
		free(pixelBuffer);
		pixelBuffer = NULL;
	}
	/*
	if (scopeTexture != NULL)
	{
		delete scopeTexture;
		scopeTexture = NULL;
	}
	*/
}
