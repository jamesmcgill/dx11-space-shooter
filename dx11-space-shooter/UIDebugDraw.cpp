#include "pch.h"
#include "UIDebugDraw.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace ui
{
const DirectX::SimpleMath::Vector2 BUTTON_BORDER_SIZE
	= DirectX::SimpleMath::Vector2(5.0f, 5.0f);

const DirectX::XMVECTOR BUTTON_COLOR_NORMAL		= DirectX::Colors::OliveDrab;
const DirectX::XMVECTOR BUTTON_COLOR_SELECTED = DirectX::Colors::Green;
const DirectX::XMVECTOR BUTTON_COLOR_HOVER		= DirectX::Colors::Gray;

const DirectX::XMVECTOR BUTTON_BORDER_COLOR_NORMAL	 = DirectX::Colors::Olive;
const DirectX::XMVECTOR BUTTON_BORDER_COLOR_SELECTED = DirectX::Colors::Olive;
const DirectX::XMVECTOR BUTTON_BORDER_COLOR_HOVER		 = DirectX::Colors::Olive;

//------------------------------------------------------------------------------
void
drawBox(
	DX::DebugBatchType& primitiveBatch,
	float x,
	float y,
	float width,
	float height,
	FXMVECTOR color,
	float layer)
{
	const XMVECTOR topLeft	= {x, y, layer};
	const XMVECTOR topRight = {x + width, y, layer};
	const XMVECTOR botRight = {x + width, y + height, layer};
	const XMVECTOR botLeft	= {x, y + height, layer};

	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, topLeft);
	XMStoreFloat3(&verts[1].position, topRight);
	XMStoreFloat3(&verts[2].position, botRight);
	XMStoreFloat3(&verts[3].position, botLeft);

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);
	XMStoreFloat4(&verts[3].color, color);

	primitiveBatch.DrawQuad(verts[0], verts[1], verts[2], verts[3]);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void
Button::draw(
	DX::DebugBatchType& primitiveBatch, DirectX::SpriteBatch& spriteBatch)
{
	drawBox(
		primitiveBatch,
		position.x,
		position.y,
		size.x,
		size.y,
		currentBorderColor(),
		layer);

	drawBox(
		primitiveBatch,
		position.x + (BUTTON_BORDER_SIZE.x * 0.5f),
		position.y + (BUTTON_BORDER_SIZE.y * 0.5f),
		size.x - BUTTON_BORDER_SIZE.x,
		size.y - BUTTON_BORDER_SIZE.y,
		currentColor(),
		layer);

	uiText.draw(spriteBatch);
}

//------------------------------------------------------------------------------
void
Button::centerText()
{
	uiText.position.x = position.x + (size.x * 0.5f);
	uiText.position.y = position.y + (size.y * 0.5f);
	DirectX::XMVECTOR dimensions
		= uiText.font->MeasureString(uiText.text.c_str());
	const float width	= DirectX::XMVectorGetX(dimensions);
	const float height = DirectX::XMVectorGetY(dimensions);
	uiText.origin = DirectX::SimpleMath::Vector2(width * 0.5f, height * 0.5f);
}

//------------------------------------------------------------------------------
bool
Button::isPointInside(float x, float y) const
{
	return (x >= position.x) && (x <= position.x + size.x) && (y >= position.y)
				 && (y <= position.y + size.y);
}

//------------------------------------------------------------------------------
DirectX::SimpleMath::Vector3
Button::currentColor() const
{
	switch (appearance)
	{
		case Appearance::Normal:
			return BUTTON_COLOR_NORMAL;
			break;
		case Appearance::Selected:
			return BUTTON_COLOR_SELECTED;
			break;
		case Appearance::HoverOver:
			return BUTTON_COLOR_HOVER;
			break;
		default:
			return BUTTON_COLOR_NORMAL;
	}
}

//------------------------------------------------------------------------------
DirectX::SimpleMath::Vector3
Button::currentBorderColor() const
{
	switch (appearance)
	{
		case Appearance::Normal:
			return BUTTON_BORDER_COLOR_NORMAL;
			break;
		case Appearance::Selected:
			return BUTTON_BORDER_COLOR_SELECTED;
			break;
		case Appearance::HoverOver:
			return BUTTON_BORDER_COLOR_HOVER;
			break;
		default:
			return BUTTON_BORDER_COLOR_NORMAL;
	}
}

//------------------------------------------------------------------------------
};		// namespace ui
