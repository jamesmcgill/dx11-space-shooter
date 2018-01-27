#pragma once

#include "LevelData.h"

namespace DX
{
class StepTimer;
};
struct AppContext;
struct AppResources;
struct Entity;

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
	float currentLevelTimeS() const { return m_currentLevelTimeS; }

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
	void debugRender(DX::DebugBatchType* batch);

public:
	static constexpr size_t MAX_NUM_PATHS = 256;
	PathPool m_pathPool;		// Shared pool of all available

	static constexpr size_t MAX_NUM_FORMATIONS = 256;
	FormationPool m_formationPool;		// Shared pool of all available

	LevelPool m_levels;

	// dummy(null) data always exists in first position (index[0])
	// This means there will always be a valid path and formation to reference
	// and we don't have to worry about empty lists. However, it does mean
	// hiding this in the editor and save file.
	static constexpr size_t DUMMY_PATH_IDX			= 0;
	static constexpr size_t DUMMY_FORMATION_IDX = 0;
	void addDummyData();

private:
	AppContext& m_context;
	AppResources& m_resources;

	float m_currentLevelTimeS = 0.0f;
	size_t m_currentLevelIdx	= 0;
	size_t m_nextEventWaveIdx = 0;
	bool m_isLevelActive			= false;
};

//------------------------------------------------------------------------------
