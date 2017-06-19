#pragma once
#include "Enemies.h"

namespace DX
{
class StepTimer;
}
struct AppContext;
struct AppResources;
struct Entity;

//------------------------------------------------------------------------------
class GameLogic
{
public:
	GameLogic(AppContext& context, AppResources& resources);

	void reset();
	void update(const DX::StepTimer& timer);
	void render();

	void performPhysicsUpdate(const DX::StepTimer& timer);
	void performCollisionTests();

	template <typename Func>
	void collisionTestEntity(
		Entity& entity,
		const size_t rangeStartIdx,
		const size_t rangeOnePastEndIdx,
		Func onCollision);

	void renderEntity(Entity& entity);
	void renderEntityBound(Entity& entity);
	void drawHUD();

private:
	AppContext& m_context;
	AppResources& m_resources;

public:
	Enemies m_enemies;
};

//------------------------------------------------------------------------------
