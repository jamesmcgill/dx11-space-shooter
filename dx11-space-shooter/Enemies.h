#pragma once

#include "Entity.h"					 // NUM_ENEMIES
#include "AppResources.h"		 // ModelResource
#include "json11/json11.hpp"

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

	static Waypoint from_json(const json11::Json& json);
	json11::Json to_json() const;
};

struct Path
{
	std::wstring id;
	std::vector<Waypoint> waypoints;

	void debugRender(
		DX::DebugBatchType* batch,
		size_t selectedPointIdx		= -1,
		size_t selectedControlIdx = -1) const;

	static Path from_json(const json11::Json& json);
	json11::Json to_json() const;
};

struct FormationSection
{
	size_t pathIdx			= 0;
	int numShips				= 0;
	ModelResource model = ModelResource::Enemy1;

	static FormationSection from_json(const json11::Json& json);
	json11::Json to_json() const;
};

struct Formation
{
	std::wstring id;
	std::vector<FormationSection> sections;

	static Formation from_json(const json11::Json& json);
	json11::Json to_json() const;
};

struct Wave
{
	float spawnTimeS		= 0.0f;
	size_t formationIdx = 0;

	static Wave from_json(const json11::Json& json);
	json11::Json to_json() const;
};

struct Level
{
	std::vector<Wave> waves;

	static Level from_json(const json11::Json& json);
	json11::Json to_json() const;
};

using PathPool			= std::vector<Path>;
using FormationPool = std::vector<Formation>;
//------------------------------------------------------------------------------
class Enemies
{
public:
	Enemies(AppContext& context, AppResources& resources);
	void resetLevelData();
	void reset();
	void update(const DX::StepTimer& timer);

	void incrementCurrentTime(const DX::StepTimer& timer);
	void resetCurrentTime();

	void updateLevel();
	void performPhysicsUpdate();

	bool isAnyEnemyAlive() const;
	void jumpToLevel(const size_t levelIdx);
	void jumpToWave(const size_t waveIdx);

	void spawnFormation(const size_t formationIdx, const float birthTimeS);
	void spawnFormationSection(
		const int numShips,
		const size_t pathIdx,
		const ModelResource model,
		const float birthTimeS);

	void emitShot(
		const Entity& emitter,
		const float yPosScale,
		const float speed,
		size_t& shotEntityIdx,
		const size_t minEntityIdx,
		const size_t maxEntityIdxPlusOne);

	void emitPlayerShot();

	void load();
	void save();

	void parseRootJsonArray(const json11::Json& json);
	void parseRootJsonObject(const json11::Json& json);
	void parsePathsJsonObject(const json11::Json& json);
	void parseFormationsJsonObject(const json11::Json& json);
	void parseLevelsJsonObject(const json11::Json& json);

	void debugRender(DX::DebugBatchType* batch);
	void debug_toggleLevel();

	std::vector<Level>& debug_getCurrentLevels();
	FormationPool& debug_getFormations() { return m_formationPool; }
	PathPool& debug_getPaths() { return m_pathPool; }
	size_t nullPathIdx			= 0;
	size_t nullFormationIdx = 0;

private:
	AppContext& m_context;
	AppResources& m_resources;

	float m_currentLevelTimeS = 0.0f;
	size_t m_currentLevelIdx	= 0;
	size_t m_nextEventWaveIdx = 0;
	bool m_isLevelActive			= false;

	static constexpr size_t MAX_NUM_PATHS = 256;
	PathPool m_pathPool;		// Shared pool of all available

	static constexpr size_t MAX_NUM_FORMATIONS = 256;
	FormationPool m_formationPool;		// Shared pool of all available
};

//------------------------------------------------------------------------------
