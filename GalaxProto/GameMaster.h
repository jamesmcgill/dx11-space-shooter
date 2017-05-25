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
	void Update(DX::StepTimer const& timer);

	void emitShot(
		const Entity& emitter,
		const float yPosScale,
		const float speed,
		size_t& shotEntityIdx,
		const size_t minEntityIdx,
		const size_t maxEntityIdxPlusOne);

	void emitPlayerShot();

	void LoadWaveData();
	void RenderWaveData();

private:
	GameState& m_state;
	std::vector<Waypoint> m_waypoints;
};

//------------------------------------------------------------------------------
