#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "UI/Globals.hpp"
#include "UI/Event.hpp"
#include "UI/Root.hpp"

#include "Screens/MainScreen.hpp"

UI::Root *root;
sf::RenderWindow *window;
sf::Font *font;

sf::Color bgColor(0xDF, 0xDF, 0xDF);

bool exit_ = false;

void Exit()
{
	exit_ = true;
}

int main()
{
	// Initalize window and font
	const std::string winTitle = "Template Program";
	const int winWidth = 800;
	const int winHeight = 600;
	window = new sf::RenderWindow(sf::VideoMode(winWidth, winHeight), winTitle, sf::Style::Titlebar | sf::Style::Close);
	window->setFramerateLimit(60);

	// Load font
	font = new sf::Font();
	font->loadFromFile("./HarmonyOS_Sans_Regular.ttf");
	UI::Globals::font = font;

	// Initalize
	root = new UI::Root(window, &Screens::Main::mainScreen);

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
				root->UpdateSettings();

			UI::Event::UpdateEvent(event);
		}
		UI::Event::UpdateAfterPoll();

		root->Update();

		window->clear(bgColor);

		root->Draw();

		window->display();

		if (UI::Globals::exitReq)
			window->close();
	}

	// Cleanup
	delete window, font, root;

	return 0;
}
