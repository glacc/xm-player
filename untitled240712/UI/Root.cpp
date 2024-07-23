#include "Root.hpp"

using namespace UI;

void Root::UpdateSettings()
{
	if (renderTarget == NULL)
		return;

	rootViewport->width = renderTarget->getSize().x;
	rootViewport->height = renderTarget->getSize().y;
	rootViewport->UpdateSettings();
}

void Root::Update()
{
	rootViewport->MouseHoverCheck();
	rootViewport->Update();
}

void Root::Draw()
{
	if (renderTarget == NULL)
		return;

	rootViewport->Draw();
}

Root::Root(sf::RenderTarget *initRenderTarget, Screen *initScreen)
{
	renderTarget = initRenderTarget;
	currentScreen = initScreen;

	rootViewport = new Elements::Viewport(
		initScreen,
		0, 0, renderTarget->getSize().x, renderTarget->getSize().y
	);
	rootViewport->renderTarget = renderTarget;
}

Root::Root() {}

Root::~Root()
{
	delete rootViewport;
}
