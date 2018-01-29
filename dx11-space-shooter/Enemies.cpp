#include "pch.h"
#include "Enemies.h"
#include "AppContext.h"
#include "AppResources.h"
#include "StepTimer.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

constexpr float SHOT_SPEED									= 40.0f;
constexpr float ENEMY_SPAWN_OFFSET_TIME_SEC = 0.5f;

using UniRandFloat = std::uniform_real_distribution<float>;
using UniRandIdx	 = std::uniform_int_distribution<size_t>;
static UniRandFloat shotTimeRand(0.1f, 0.8f);
static UniRandIdx enemyIdxRand(ENEMIES_IDX, ENEMIES_END - 1);

//------------------------------------------------------------------------------
Enemies::Enemies(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
		, m_currentLevelIdx(0)
		, m_nextEventWaveIdx(0)
{
	TRACE
	m_formationPool.reserve(MAX_NUM_FORMATIONS);
	m_pathPool.reserve(MAX_NUM_PATHS);
	resetLevelData();
	load();

	reset();
}

//------------------------------------------------------------------------------
void
Enemies::resetLevelData()
{
	TRACE
	m_pathPool.clear();
	m_formationPool.clear();
	m_levels.clear();

	addDummyData();
}

//------------------------------------------------------------------------------
void
Enemies::addDummyData()
{
	ASSERT(m_pathPool.size() == 0);
	ASSERT(m_formationPool.size() == 0);
	ASSERT(m_pathPool.size() == DUMMY_PATH_IDX);

	static const Path nullPath = {
		L"nullPath",
		{
			{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		},
	};
	m_pathPool.emplace_back(nullPath);

	const int shipCount = 1;
	auto& formation			= m_formationPool.emplace_back(Formation());
	formation.id				= L"nullFormation";
	formation.sections.emplace_back(
		FormationSection{DUMMY_PATH_IDX, shipCount, ModelResource::Enemy1});
}

//------------------------------------------------------------------------------
void
Enemies::reset()
{
	TRACE
	resetCurrentTime();
	m_currentLevelIdx	= 0;
	m_nextEventWaveIdx = 0;

	float currentTimeS
		= static_cast<float>(m_resources.m_timer.GetTotalSeconds());
	m_nextShotTimeS = currentTimeS + shotTimeRand(m_resources.randEngine);

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_context.entities[i].isAlive			= false;
		m_context.entities[i].isColliding = false;
	}

	m_isLevelActive = false;
	if (
		(m_currentLevelIdx < m_levels.size())
		&& (!m_levels[m_currentLevelIdx].waves.empty()))
	{
		m_isLevelActive = true;
	}
}

//------------------------------------------------------------------------------
void
Enemies::update(const DX::StepTimer& timer)
{
	TRACE
	float currentTimeS = static_cast<float>(timer.GetTotalSeconds());
	incrementCurrentTime(timer);

	updateLevel();

	// Spawn enemy shots
	if (currentTimeS >= m_nextShotTimeS)
	{
		m_nextShotTimeS = currentTimeS + shotTimeRand(m_resources.randEngine);

		size_t attempts = 0;
		size_t enemyIdx = 0;
		do
		{
			enemyIdx = enemyIdxRand(m_resources.randEngine);
			ASSERT(enemyIdx < ENEMIES_END && enemyIdx >= ENEMIES_IDX);
		} while (!m_context.entities[enemyIdx].isAlive && ++attempts < 10);

		if (m_context.entities[enemyIdx].isAlive)
		{
			emitShot(
				m_context.entities[enemyIdx],
				-1.0f,
				-SHOT_SPEED,
				m_context.nextEnemyShotIdx,
				ENEMY_SHOTS_IDX,
				ENEMY_SHOTS_END);
			m_resources.soundEffects[AudioResource::EnemyShot]->Play();
		}
	}

	performPhysicsUpdate();
}

//------------------------------------------------------------------------------
void
Enemies::incrementCurrentTime(const DX::StepTimer& timer)
{
	m_currentLevelTimeS += static_cast<float>(timer.GetElapsedSeconds());
}

//------------------------------------------------------------------------------
void
Enemies::resetCurrentTime()
{
	m_currentLevelTimeS = 0.0f;
}

//------------------------------------------------------------------------------
void
Enemies::updateLevel()
{
	if (!m_isLevelActive)
	{
		// Activate next level once enemies are cleared
		if (!isAnyEnemyAlive())
		{
			m_isLevelActive = true;
			resetCurrentTime();
		}
		return;
	}
	auto& level = m_levels[m_currentLevelIdx];

	// End of level
	if (m_nextEventWaveIdx >= level.waves.size())
	{
		m_isLevelActive		 = false;
		m_nextEventWaveIdx = 0;
		m_currentLevelIdx++;
		if (m_currentLevelIdx >= m_levels.size())
		{
			// Wrap around to level 1 after finishing the last level
			m_currentLevelIdx = 0;
		}

		return;
	}

	// Spawn next wave
	auto& nextWave = level.waves[m_nextEventWaveIdx];
	if (m_currentLevelTimeS >= nextWave.spawnTimeS)
	{
		spawnFormation(nextWave.formationIdx, m_currentLevelTimeS);
		m_nextEventWaveIdx++;
	}
}

//------------------------------------------------------------------------------
bool
Enemies::isAnyEnemyAlive() const
{
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		if (m_context.entities[i].isAlive)
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void
Enemies::jumpToLevel(const size_t levelIdx)
{
	if (levelIdx < m_levels.size())
	{
		m_currentLevelIdx = levelIdx;
	}
}

//------------------------------------------------------------------------------
void
Enemies::jumpToWave(const size_t waveIdx)
{
	ASSERT(m_currentLevelIdx < m_levels.size());
	auto& level = m_levels[m_currentLevelIdx];

	if (waveIdx < level.waves.size())
	{
		m_nextEventWaveIdx	= waveIdx;
		m_isLevelActive			= true;
		m_currentLevelTimeS = level.waves[waveIdx].spawnTimeS;
	}
}

//------------------------------------------------------------------------------
void
Enemies::spawnFormation(const size_t formationIdx, const float birthTimeS)
{
	ASSERT(formationIdx < m_formationPool.size());
	auto& formation = m_formationPool[formationIdx];
	for (auto& sec : formation.sections)
	{
		spawnFormationSection(sec.numShips, sec.pathIdx, sec.model, birthTimeS);
	}
}

//------------------------------------------------------------------------------
void
Enemies::spawnFormationSection(
	const int numShips,
	const size_t pathIdx,
	const ModelResource model,
	const float birthTimeS)
{
	ASSERT(pathIdx < m_pathPool.size());

	float delayS = 0.0f;
	for (int ship = 0; ship < numShips; ++ship)
	{
		// Spawn enemy
		auto& newEnemy			= m_context.entities[m_context.nextEnemyIdx];
		newEnemy.pathIdx		= pathIdx;
		newEnemy.isAlive		= true;
		newEnemy.birthTimeS = birthTimeS + delayS;
		newEnemy.model			= &m_resources.modelData[model];
		m_context.nextEnemyIdx++;
		if (m_context.nextEnemyIdx >= ENEMIES_END)
		{
			m_context.nextEnemyIdx = ENEMIES_IDX;
		}
		delayS += ENEMY_SPAWN_OFFSET_TIME_SEC;
	}
}

//------------------------------------------------------------------------------
static FXMVECTOR
bezier(FLOAT t, FXMVECTOR startPos, FXMVECTOR endPos, FXMVECTOR control)
{
	// https://pomax.github.io/bezierinfo/
	float t2	= t * t;
	float mt	= 1 - t;
	float mt2 = mt * mt;
	return (startPos * mt2) + (control * (2 * mt * t)) + (endPos * t2);
}

//------------------------------------------------------------------------------
void
Enemies::performPhysicsUpdate()
{
	TRACE

	// TODO(James): make the ships move at constant speed.
	// At the moment it looks bad that the speed changes suddenly
	// when moving between curves
	static const float SEGMENT_DURATION_S = 1.2f;

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		auto& e = m_context.entities[i];
		if (!e.isAlive)
		{
			continue;
		}
		ASSERT(e.pathIdx < m_pathPool.size());
		const auto& path	 = m_pathPool[e.pathIdx];
		const float aliveS = (m_currentLevelTimeS - e.birthTimeS);
		if (aliveS < 0.0f)
		{
			ASSERT(!path.waypoints.empty());
			e.position = path.waypoints[0].wayPoint;
			continue;
		}

		// Enemy finished it's route
		const size_t currentSegment
			= static_cast<size_t>(floor(aliveS / SEGMENT_DURATION_S));
		if (currentSegment >= path.waypoints.size() - 1)
		{
			e.isAlive = false;
			continue;
		}

		const float t = fmod(aliveS, SEGMENT_DURATION_S) / SEGMENT_DURATION_S;
		e.position		= bezier(
			 t,
			 path.waypoints[currentSegment].wayPoint,
			 path.waypoints[currentSegment + 1].wayPoint,
			 path.waypoints[currentSegment + 1].controlPoint);
	}
}

//------------------------------------------------------------------------------
void
Enemies::emitShot(
	const Entity& emitter,
	const float yPosScale,
	const float speed,
	size_t& shotEntityIdx,
	const size_t minEntityIdx,
	const size_t maxEntityIdxPlusOne)
{
	TRACE
	auto& newShot		= m_context.entities[shotEntityIdx];
	newShot.isAlive = true;
	newShot.position
		= emitter.position + emitter.model->bound.Center
			+ Vector3(0.0f, (yPosScale * emitter.model->bound.Radius), 0.0f);
	newShot.velocity = Vector3(0.0f, speed, 0.0f);
	newShot.birthTimeS
		= static_cast<float>(m_resources.m_timer.GetTotalSeconds());

	shotEntityIdx++;
	if (shotEntityIdx >= maxEntityIdxPlusOne)
	{
		shotEntityIdx = minEntityIdx;
	}
}

//------------------------------------------------------------------------------
void
Enemies::emitPlayerShot()
{
	TRACE
	emitShot(
		m_context.entities[PLAYERS_IDX],
		1.0f,
		SHOT_SPEED,
		m_context.nextPlayerShotIdx,
		PLAYER_SHOTS_IDX,
		PLAYER_SHOTS_END);

	m_resources.soundEffects[AudioResource::PlayerShot]->Play();
}

//------------------------------------------------------------------------------
void
Enemies::load()
{
	TRACE
	resetLevelData();
	LevelData::load(m_pathPool, m_formationPool, m_levels);
}

//------------------------------------------------------------------------------
void
Enemies::save()
{
	TRACE
	// NB: begin()+1 skips the items injected to index[0] by addNullData()
	// We don't need to save those
	LevelData::save(
		m_pathPool.begin() + 1,
		m_pathPool.end(),
		m_formationPool.begin() + 1,
		m_formationPool.end(),
		m_levels.begin(),
		m_levels.end());
}

//------------------------------------------------------------------------------
void
Enemies::debugRender(DX::DebugBatchType* batch)
{
	TRACE
	std::set<size_t> pathsToRender;
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		const auto& e = m_context.entities[i];
		if (e.isAlive && e.pathIdx < m_pathPool.size())
		{
			pathsToRender.insert(e.pathIdx);
		}
	}

	for (const auto& pathIdx : pathsToRender)
	{
		ASSERT(pathIdx < m_pathPool.size());
		m_pathPool[pathIdx].debugRender(batch);
	}
}

//------------------------------------------------------------------------------
