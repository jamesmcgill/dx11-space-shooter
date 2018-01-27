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
	enum class GameStatus
	{
		Playing,
		GameOver,
	};

	GameLogic(AppContext& context, AppResources& resources);

	void reset();
	GameStatus update(const DX::StepTimer& timer);
	void render();
	void renderEntityModels();
	void renderShotParticles();
	void renderEntitiesDebug();

	void performPhysicsUpdate(const DX::StepTimer& timer);
	void constrainPlayer(Entity& e);
	void constrainShot(Entity& e);
	void performCollisionTests();

	template <typename Func>
	void collisionTestEntity(
		Entity& entity,
		const size_t rangeStartIdx,
		const size_t rangeOnePastEndIdx,
		Func& onCollision);

	void renderPlayerEntity(Entity& entity);
	void renderEntityModel(Entity& entity, float orientation = 0.0f);
	void renderEntityBound(Entity& entity);
	void renderPlayerBoundary();

	void updateUIScore();
	void updateUILives();

	void drawHUD();
	void updateUIDebugVariables();
	void drawDebugVariables();

private:
	AppContext& m_context;
	AppResources& m_resources;
	bool m_hudDirty = true;

public:
	Enemies m_enemies;
};

//------------------------------------------------------------------------------
