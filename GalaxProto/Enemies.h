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

struct Path
{
	std::wstring id;
	std::vector<Waypoint> waypoints;
};

struct FormationSection
{
	size_t pathIdx;
	int numShips;
	ModelResource model;
};

struct Formation
{
	std::wstring id;
	std::vector<FormationSection> sections;
};

struct Wave
{
	float spawnTimeS;
	size_t formationIdx;
};

struct Level
{
	std::vector<Wave> waves;
};

using PathPool			= std::vector<Path>;
using FormationPool = std::vector<Formation>;
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
	FormationPool& debug_getFormations() { return m_formationPool; }
	PathPool& debug_getPaths() { return m_pathPool; }

private:
	using EntityIdxToPathMap = std::array<const Path*, NUM_ENEMIES>;
	EntityIdxToPathMap m_entityIdxToPath;

	AppContext& m_context;
	AppResources& m_resources;
	size_t m_currentLevel;
	float m_nextEventTimeS;
	size_t m_nextEventWaveIdx;
	size_t m_activeWaveIdx;

	static constexpr size_t MAX_NUM_PATHS = 256;
	PathPool m_pathPool;		// Shared pool of all available

	static constexpr size_t MAX_NUM_FORMATIONS = 256;
	FormationPool m_formationPool;		// Shared pool of all available
};

//------------------------------------------------------------------------------
