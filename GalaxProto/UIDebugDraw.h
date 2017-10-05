#pragma once
struct AppContext;
struct AppResources;

class UIDebugDraw
{
public:
	UIDebugDraw(AppContext& context, AppResources& resources);

	void drawBox(
		float x, float y, float width, float height, DirectX::FXMVECTOR color);

private:
	AppContext& m_context;
	AppResources& m_resources;
};
