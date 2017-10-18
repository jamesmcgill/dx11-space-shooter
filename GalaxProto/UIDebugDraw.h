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
struct Text
{
	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 origin;
	DirectX::SpriteFont* font = nullptr;
	std::wstring text;

	void draw(
		DirectX::SpriteBatch& spriteBatch,
		DirectX::XMVECTOR color,
		float layer												 = Layer::L5_Default,
		float rotation										 = 0.0f,
		DirectX::SimpleMath::Vector2 scale = DirectX::SimpleMath::Vector2(1.f, 1.f))
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
class DebugDraw
{
public:
	DebugDraw(AppContext& context, AppResources& resources);

	//----------------------------------------------------------------------------
	// Call before and after any 2D draw calls
	//----------------------------------------------------------------------------
	void begin2D();
	void end2D();

	//----------------------------------------------------------------------------
	// Requires: primitiveBatch already active via Begin() call
	//----------------------------------------------------------------------------
	void drawBox(
		DX::DebugBatchType& primitiveBatch,
		float x,
		float y,
		float width,
		float height,
		DirectX::FXMVECTOR color,
		float layer = Layer::L5_Default);

private:
	AppContext& m_context;
	AppResources& m_resources;
};

};		// namespace ui
