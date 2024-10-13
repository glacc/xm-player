#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "UI/Globals.hpp"
#include "UI/Event.hpp"
#include "UI/Root.hpp"

#include "Screens/MainScreen.hpp"

#include "Global.hpp"

sf::RenderWindow *window;

sf::Color bgColor(0xDF, 0xDF, 0xDF);

bool exit_ = false;

void Exit()
{
	exit_ = true;
}

//int main(int argv, char **args)
int main()
{
	// Initalize window and font
	const int winWidth = 1276;
	const int winHeight = 1026;
	window = new sf::RenderWindow(sf::VideoMode(winWidth, winHeight), Glacc::Global::winTitle, sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);
	window->setFramerateLimit(60);

	// Load font
	//std::string fontName = "./HarmonyOS_Sans_Regular.ttf";
	std::string fontName = "./FantasqueSansMono-Regular.ttf";
	UI::Globals::font = new sf::Font();
	UI::Globals::font->loadFromFile(fontName);

	Glacc::Global::monoFont = UI::Globals::font;

	// Initalize
	UI::Globals::root = new UI::Root(window, &Screens::Main::mainScreen);
	Screens::Main::mainScreen.renderTarget = UI::Globals::root->rootViewport->viewportRenderTexture;

	/* Custom */

	Glacc::Global::window = window;
	bool isLoadedOld = Screens::Main::xmPlayer.IsLoaded();

	/* -- End */

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
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window->setView(sf::View(visibleArea));
				UI::Globals::root->UpdateSettings();
			}

			UI::Event::UpdateEvent(event);
		}
		UI::Event::UpdateAfterPoll();

		UI::Globals::root->Update();

		window->clear(bgColor);

		/* Custom */

		/*
		if (Screens::Main::xmPlayer.IsLoaded() != isLoadedOld)
		{
			if (Screens::Main::xmPlayer.IsLoaded())
			{
				Glacc::XMPlayer::Stat stat;
				Screens::Main::xmPlayer.GetCurrentStat(stat);
				window->setTitle(winTitle + " - " + stat.songName);
			}
			else
				window->setTitle(winTitle);
		}
		*/

		/* -- End */

		UI::Globals::root->Draw();

		window->display();

		/* Custom */

		isLoadedOld = Screens::Main::xmPlayer.IsLoaded();

		/* -- End */

		if (UI::Globals::exitReq)
			window->close();
	}

	Screens::Main::xmPlayer.CleanUp();
	delete window, UI::Globals::font, UI::Globals::root;

	return 0;
}
