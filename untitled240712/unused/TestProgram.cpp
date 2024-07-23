#include <iostream>
#include <fstream>
#include <random>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "UI/Globals.hpp"
#include "UI/Event.hpp"
#include "UI/Root.hpp"
#include "UI/Elements/Viewport.hpp"

#include "Screens/TestMenu.hpp"

#include "Test/SoundStream.hpp"

#include "xmPlayer/xmPlayer.h"

sf::RenderWindow *window;
sf::Font *font;

//Screens::TestMenu::TestMenuScreen mainMenu;
//UI::Root root(&mainMenu);
UI::Elements::Viewport *rootViewport;

sf::Color bgColor(0x51, 0x78, 0xB7);

Test::SoundStream *soundStream;

bool exit_ = false;

void Exit()
{
	exit_ = true;
}

void ChangeColor()
{
	int r = 63 + std::rand() % (255 - 64 - 64);
	int g = 63 + std::rand() % (255 - 64 - 64);
	int b = 63 + std::rand() % (255 - 64 - 64);
	bgColor = sf::Color(r, g, b);
}

void ToggleSound()
{
	if (soundStream->getStatus() == soundStream->Stopped)
		soundStream->play();
	else
	{
		switch (soundStream->waveform)
		{
		case Test::SoundStream::Sine:
			soundStream->waveform = Test::SoundStream::Square;
			break;
		case Test::SoundStream::Square:
			soundStream->waveform = Test::SoundStream::Triangle;
			break;
		case Test::SoundStream::Triangle:
			soundStream->waveform = Test::SoundStream::Sawtooth;
			break;
		case Test::SoundStream::Sawtooth:
			soundStream->waveform = Test::SoundStream::Sine;
			soundStream->stop();
			break;
		}
	}
}

void ToggleModule()
{
	if (GXMPlayer::IsLoaded())
	{
		if (GXMPlayer::IsPlaying())
			GXMPlayer::StopModule();
		else
			GXMPlayer::PlayModule();
	}
}

int main()
{
	// Initalize window and font
	const int winWidth = 800;
	const int winHeight = 600;

	window = new sf::RenderWindow(sf::VideoMode(winWidth, winHeight), "Test XwX", sf::Style::Titlebar | sf::Style::Close);

	font = new sf::Font();
	font->loadFromFile("./HarmonyOS_Sans_Regular.ttf");

	UI::Globals::font = font;

	window->setFramerateLimit(60);

	// Initalize

	std::srand(std::time(nullptr));

	//mainMenu.renderTarget = root.renderTarget = root.renderTarget = window;
	rootViewport = new UI::Elements::Viewport(&Screens::TestMenu::testMenuScreen, 100, 75, 600, 450);
	rootViewport->renderTarget = window;
	Screens::TestMenu::testMenuScreen.colorChangeFunc = ChangeColor;
	//Screens::TestMenu::testMenuScreen.soundToggleFunc = ToggleSound;
	Screens::TestMenu::testMenuScreen.soundToggleFunc = ToggleModule;
	Screens::TestMenu::testMenuScreen.exitFunc = Exit;

	soundStream = new Test::SoundStream();

	std::string fileName = "./INTROC14.XM";
	char *fileData;
	std::ifstream inputFile;
	inputFile.open(fileName, std::ios::in | std::ios::binary);
	if (inputFile)
	{
		inputFile.seekg(0, std::ios_base::end);
		int fileSize = inputFile.tellg();

		fileData = (char *)malloc(fileSize);
		inputFile.seekg(0, std::ios_base::beg);
		inputFile.read(fileData, fileSize);

		if (GXMPlayer::LoadModule((uint8_t *)fileData, fileSize, false))
			std::cout << "Loaded module: " << GXMPlayer::GetSongName() << std::endl;
		else
			std::cout << "Failed to load module " << fileName << std::endl;

		inputFile.close();
		free(fileData);
	}
	else
		std::cout << "Failed to open module " << fileName << std::endl;

	//GXMPlayer::PlayModule();

	bool drag = false;
	while (window->isOpen())
	{
		UI::Event::UpdateBeforePoll();
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();

			if (event.type == sf::Event::Resized)
			{
				rootViewport->UpdateSettings();
			}

			UI::Event::UpdateEvent(event);
		}
		UI::Event::UpdateAfterPoll();

		rootViewport->Update();

		if (UI::Event::mousePress && !rootViewport->mouseInbound)
			drag = true;
		if (UI::Event::mouseRelease || !UI::Event::mouseHold)
			drag = false;

		if (drag)
		{
			rootViewport->px += UI::Event::mouseVelX;
			rootViewport->py += UI::Event::mouseVelY;
			rootViewport->UpdateSettings();
		}

		window->clear(bgColor);

		//root.UpdateAndDraw();
		rootViewport->Draw();

		window->display();

		if (exit_)
			window->close();
	}

	GXMPlayer::CleanUp();
	delete window, font, rootViewport, soundStream;

	return 0;
}
