#include "Viewport.hpp"

using namespace UI::Elements;

void Viewport::UpdateOrigin()
{
	switch (origin)
	{
	case Left:
		originX = 0;
		originY = height / 2;
		break;
	case TopLeft:
		originX = 0;
		originY = 0;
		break;
	case Top:
		originX = width / 2;
		originY = 0;
		break;
	case TopRight:
		originX = width;
		originY = 0;
		break;
	case Right:
		originX = width;
		originY = height / 2;
		break;
	case BottomRight:
		originX = width;
		originY = height;
		break;
	case Bottom:
		originX = width / 2;
		originY = height;
		break;
	case BottomLeft:
		originX = 0;
		originY = height;
		break;
	case Middle:
		originX = width / 2;
		originY = height / 2;
		break;
	}

	originX = (int)originX;
	originY = (int)originY;
}

void Viewport::CheckRenderTexture()
{
	if (viewportRenderTexture == NULL)
	{
		viewportRenderTexture = new sf::RenderTexture();
		viewportRenderTexture->create(width, height);

		if (screen != NULL)
			screen->renderTarget = viewportRenderTexture;
	}
}

void Viewport::UpdateSettings()
{
	CheckRenderTexture();
	if (width != viewportRenderTexture->getSize().x || height != viewportRenderTexture->getSize().y)
	{
		viewportRenderTexture->create(width, height);
		//viewportRenderTexture->setV
	}

	UpdateOrigin();

	if (screen != NULL)
		screen->UpdateSettings();
}

bool Viewport::MouseHoverCheck()
{
	if (!screen->init)
		return false;

	bool prevMouseInbound = true;
	if (parentViewport != NULL)
		prevMouseInbound = parentViewport->mouseInbound;

	int absPx = px - originX;
	int absPy = py - originY;
	mouseInbound =
		prevMouseInbound &&
		(UI::Event::mouseX >= absPx && UI::Event::mouseX < absPx + width) &&
		(UI::Event::mouseY >= absPy && UI::Event::mouseY < absPy + height);

	UI::Event::mouseX -= absPx;
	UI::Event::mouseY -= absPy;
	UI::Event::mouseOldX -= absPx;
	UI::Event::mouseOldY -= absPy;
	UI::Event::mouseInbound = mouseInbound;

	if (screen != NULL)
	{
		screen->mouseInbound = mouseInbound;
		screen->MouseHoverCheck();
	}

	UI::Event::mouseX += absPx;
	UI::Event::mouseY += absPy;
	UI::Event::mouseOldX -= absPx;
	UI::Event::mouseOldY -= absPy;

	return mouseInbound;
}

void Viewport::Update()
{
	screen->InitCheck();

	int absPx = px - originX;
	int absPy = py - originY;
	UI::Event::mouseX -= absPx;
	UI::Event::mouseY -= absPy;
	UI::Event::mouseOldX -= absPx;
	UI::Event::mouseOldY -= absPy;

	if (screen != NULL)
		screen->Update();

	UI::Event::mouseX += absPx;
	UI::Event::mouseY += absPy;
	UI::Event::mouseOldX -= absPx;
	UI::Event::mouseOldY -= absPy;
}

void Viewport::Draw(sf::RenderTarget *rt)
{
	if (renderTarget == NULL || screen == NULL)
		return;

	viewportRenderTexture->clear(screen->bgColor);

	screen->Draw();

	viewportRenderTexture->display();

	sf::Sprite viewportSprite(viewportRenderTexture->getTexture());
	viewportSprite.setPosition(px - originX, py - originY);
	renderTarget->draw(viewportSprite);
}

void Viewport::UpdateAndDraw(sf::RenderTarget *rt)
{
	Update();
	Draw();
}

void Viewport::InitSettings(UI::Screen *initScreen, int newPx, int newPy, int newWidth, int newHeight, Viewport *newParentViewport)
{
	px = newPx;
	py = newPy;
	width = newWidth;
	height = newHeight;
	origin = TopLeft;

	parentViewport = newParentViewport;
	if (newParentViewport != NULL)
		renderTarget = newParentViewport->renderTarget;

	screen = initScreen;

	/*
	viewportRenderTexture = new sf::RenderTexture();
	viewportRenderTexture->create(width, height);

	screen->renderTarget = viewportRenderTexture;
	*/
	/*
	CheckRenderTexture();
	UpdateOrigin();
	*/
	UpdateSettings();
}

Viewport::Viewport(UI::Screen *initScreen, int newPx, int newPy, int newWidth, int newHeight, Viewport *newParentViewport)
{
	InitSettings(initScreen, newPx, newPy, newWidth, newHeight, newParentViewport);
}

Viewport::Viewport(UI::Screen *initScreen, Viewport *newParentViewport)
{
	InitSettings(initScreen, 0, 0, 0, 0, newParentViewport);
}

Viewport::Viewport() {}

Viewport::~Viewport()
{
	/* (Some crap that causes crash)
	if (viewportRenderTexture != NULL)
		delete viewportRenderTexture;
	*/
}
