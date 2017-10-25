#pragma once

#include "Entity.h"					 // NUM_ENEMIES
#include "AppResources.h"		 // ModelResource

namespace DX
{
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

struct EnemyFormationSection
{
	const std::vector<Waypoint> waypoints;
	int numShips;
	ModelResource model;
};

struct EnemyFormation {
	std::wstring id;
	std::vector<EnemyFormationSection> sections;
};

struct EnemyWave
{
	float spawnTimeS;
	size_t formationIdx;
};

struct Level
{
	std::vector<EnemyWave> waves;
};

using EnemyFormationPool = std::vector<EnemyFormation>;
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
	void debug_toggleLevel();

	std::vector<Level>& debug_getCurrentLevels();
	EnemyFormationPool& debug_getFormations() {return m_formationPool;}


private:
	using EntityIdxToWaypointsMap
		= std::array<const std::vector<Waypoint>*, NUM_ENEMIES>;
	EntityIdxToWaypointsMap m_entityIdxToWaypoints;

	AppContext& m_context;
	AppResources& m_resources;
	size_t m_currentLevel;
	float m_nextEventTimeS;
	size_t m_nextEventWaveIdx;
	size_t m_activeWaveIdx;

	static constexpr size_t MAX_WAVES = 256;
	EnemyFormationPool m_formationPool;		// Shared pool of all available
};

//------------------------------------------------------------------------------
