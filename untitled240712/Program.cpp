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
	const std::string winTitle = "XM Player (Glacc)";
	const int winWidth = 1024;
	const int winHeight = 768;
	window = new sf::RenderWindow(sf::VideoMode(winWidth, winHeight), winTitle, sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close);
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
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window->setView(sf::View(visibleArea));
				UI::Globals::root->UpdateSettings();
			}

			UI::Event::UpdateEvent(event);
		}
		UI::Event::UpdateAfterPoll();

		UI::Globals::root->Update();

		window->clear(bgColor);

		UI::Globals::root->Draw();

		window->display();

		if (UI::Globals::exitReq)
			window->close();
	}

	delete window, UI::Globals::font, UI::Globals::root;

	return 0;
}
