#pragma once

#include "Entity.h"	// NUM_ENEMIES

namespace DX {
	class StepTimer;
};
struct AppContext;
struct AppResources;

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
	Enemies(AppContext& context, AppResources& resources);
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
	AppResources& m_resources;
	size_t m_currentLevel;
	float m_nextEventTimeS;
	size_t m_nextEventWaveIdx;
	size_t m_activeWaveIdx;
};

//------------------------------------------------------------------------------
