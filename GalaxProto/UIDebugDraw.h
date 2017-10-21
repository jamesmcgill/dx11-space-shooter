#pragma once
struct AppContext;
struct AppResources;

namespace ui
{
//------------------------------------------------------------------------------
struct Layer
{
	static constexpr float L0_Front_Always	 = 0.0f;
	static constexpr float L1_Front_XX			 = 0.1f;
	static constexpr float L2_Front_X				 = 0.2f;
	static constexpr float L3_Front					 = 0.3f;
	static constexpr float L4_Mid_Front			 = 0.4f;
	static constexpr float L5_Default				 = 0.5f;
	static constexpr float L6_Mid_Behind		 = 0.6f;
	static constexpr float L7_Behind				 = 0.7f;
	static constexpr float L8_Behind_X			 = 0.8f;
	static constexpr float L9_Behind_XX			 = 0.9f;
	static constexpr float L10_Behind_Always = 1.0f;
};

//------------------------------------------------------------------------------
// Requires: primitiveBatch already active via Begin() call
//------------------------------------------------------------------------------
void drawBox(
	DX::DebugBatchType& primitiveBatch,
	float x,
	float y,
	float width,
	float height,
	DirectX::FXMVECTOR color,
	float layer = Layer::L5_Default);

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Text
{
	DirectX::SpriteFont* font = nullptr;		// TODO(James): Create/Set via Factory

	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 origin;
	DirectX::SimpleMath::Vector3 color;
	DirectX::SimpleMath::Vector2 scale = DirectX::SimpleMath::Vector2(1.f, 1.f);
	float layer												 = Layer::L5_Default;
	float rotation										 = 0.0f;
	std::wstring text;

	void draw(DirectX::SpriteBatch& spriteBatch)
	{
		ASSERT(font);
		font->DrawString(
			&spriteBatch,
			text.c_str(),
			position,
			color,
			rotation,
			origin,
			scale,
			DirectX::SpriteEffects_None,
			layer);
	}
};

//------------------------------------------------------------------------------
struct Button
{
	enum class Appearance
	{
		Normal,
		Selected,
		HoverOver
	};

	Text uiText;
	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 size;
	DirectX::SimpleMath::Vector2 borderSize
		= DirectX::SimpleMath::Vector2(10.0f, 10.0f);

	DirectX::SimpleMath::Vector3 colorNormal				 = DirectX::Colors::OliveDrab;
	DirectX::SimpleMath::Vector3 borderColorNormal	 = DirectX::Colors::Olive;
	DirectX::SimpleMath::Vector3 colorSelected			 = DirectX::Colors::Green;
	DirectX::SimpleMath::Vector3 borderColorSelected = DirectX::Colors::Olive;
	DirectX::SimpleMath::Vector3 colorHover					 = DirectX::Colors::Gray;
	DirectX::SimpleMath::Vector3 borderColorHover		 = DirectX::Colors::Olive;

	float layer						= Layer::L5_Default;
	Appearance appearance = Appearance::Normal;

	void centerText()
	{
		uiText.position.x = position.x + (size.x * 0.5f);
		uiText.position.y = position.y + (size.y * 0.5f);
		DirectX::XMVECTOR dimensions
			= uiText.font->MeasureString(uiText.text.c_str());
		const float width	= DirectX::XMVectorGetX(dimensions);
		const float height = DirectX::XMVectorGetY(dimensions);
		uiText.origin = DirectX::SimpleMath::Vector2(width * 0.5f, height * 0.5f);
	}

	void
	draw(DX::DebugBatchType& primitiveBatch, DirectX::SpriteBatch& spriteBatch)
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
			position.x + (borderSize.x * 0.5f),
			position.y + (borderSize.y * 0.5f),
			size.x - borderSize.x,
			size.y - borderSize.y,
			currentColor(),
			layer);

		uiText.draw(spriteBatch);
	}

	bool isPointInside(float x, float y)
	{
		return (x >= position.x) && (x <= position.x + size.x) && (y >= position.y)
					 && (y <= position.y + size.y);
	}

	DirectX::SimpleMath::Vector3 currentColor()
	{
		switch (appearance)
		{
			case Appearance::Normal:
				return colorNormal;
				break;
			case Appearance::Selected:
				return colorSelected;
				break;
			case Appearance::HoverOver:
				return colorHover;
				break;
			default:
				return colorNormal;
		}
	}
	DirectX::SimpleMath::Vector3 currentBorderColor()
	{
		switch (appearance)
		{
			case Appearance::Normal:
				return borderColorNormal;
				break;
			case Appearance::Selected:
				return borderColorSelected;
				break;
			case Appearance::HoverOver:
				return borderColorHover;
				break;
			default:
				return borderColorNormal;
		}
	}
};

//------------------------------------------------------------------------------
// Initialise 2D drawing states and open/flush render batches
//------------------------------------------------------------------------------
class DebugDraw
{
public:
	DebugDraw(AppContext& context, AppResources& resources);

	void begin2D();
	void end2D();

private:
	AppContext& m_context;
	AppResources& m_resources;
};

//------------------------------------------------------------------------------
};		// namespace ui
