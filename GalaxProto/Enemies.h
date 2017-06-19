#pragma once
#include <pch.h>
#include "StepTimer.h"
#include "Entity.h"
#include "AppContext.h"
#include <vector>
#include <array>

//------------------------------------------------------------------------------
struct Waypoint
{
	DirectX::SimpleMath::Vector3 wayPoint			= {};
	DirectX::SimpleMath::Vector3 controlPoint = {};
};

struct EnemyWave
{
	const std::vector<Waypoint> waypoints;
	const int numShips;
	const int shipType;
};

struct EnemyWaveInstance
{
	const EnemyWave wave;
	const float instanceTimeS;
};

struct Level
{
	const std::vector<EnemyWaveInstance> waves;
};

//------------------------------------------------------------------------------
class Enemies
{
public:
	Enemies(AppContext& context);
	void reset();
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

	void debugRender(DX::DebugBatchType* batch);

private:
	std::array<const EnemyWaveInstance*, NUM_ENEMIES> m_enemyToWaveMap;

	AppContext& m_context;
	size_t m_currentLevel;
	float m_nextEventTimeS;
	size_t m_nextEventWaveIdx;
	size_t m_activeWaveIdx;
};

//------------------------------------------------------------------------------
