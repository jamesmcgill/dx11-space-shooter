#pragma once
#include <pch.h>
#include "StepTimer.h"
#include "Entity.h"
#include <vector>

//------------------------------------------------------------------------------
struct Waypoint
{
	DirectX::SimpleMath::Vector3 wayPoint			= {};
	DirectX::SimpleMath::Vector3 controlPoint = {};
};

//------------------------------------------------------------------------------
class GameMaster
{
public:
	GameMaster(GameState& gameState);
	void update(const DX::StepTimer& timer);
	void performPhysicsUpdate(const DX::StepTimer& timer);

	void emitShot(
		const Entity& emitter,
		const float yPosScale,
		const float speed,
		size_t& shotEntityIdx,
		const size_t minEntityIdx,
		const size_t maxEntityIdxPlusOne);

	void emitPlayerShot();

	void loadWaveData();
	void debugRender(DX::DebugBatchType* batch);

private:
	GameState& m_state;
	float m_waveSpawnTime;
	bool m_isWaveSpawned = false;
	std::vector<Waypoint> m_waypoints;
};

//------------------------------------------------------------------------------
