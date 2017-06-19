#include "pch.h"
#include "Enemies.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const std::vector<Waypoint> path1 = {
	{Vector3(10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 10.0f, 0.0f)},
	{Vector3(-10.0f, -10.0f, 0.0f), Vector3(0.0f, -5.0f, 0.0f)},
};

static const std::vector<Waypoint> path2 = {
	{Vector3(-10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(10.0f, 8.0f, 0.0f), Vector3(10.0f, 10.0f, 0.0f)},
	{Vector3(-10.0f, 6.0f, 0.0f), Vector3(-10.0f, 8.0f, 0.0f)},
	{Vector3(10.0f, 4.0f, 0.0f), Vector3(10.0f, 8.0f, 0.0f)},
	{Vector3(-10.0f, 2.0f, 0.0f), Vector3(-10.0f, 4.0f, 0.0f)},
};

static const EnemyWave wave1 = {path1, 3, 0};
static const EnemyWave wave2 = {path2, 3, 0};
static const EnemyWave wave3 = {path1, 5, 0};
static const EnemyWave wave4 = {path2, 5, 0};

static const Level level1 = {{
	EnemyWaveInstance{wave1, 3.0f},
	EnemyWaveInstance{wave2, 5.0f},
	EnemyWaveInstance{wave3, 10.0f},
	EnemyWaveInstance{wave4, 14.0f},
	EnemyWaveInstance{wave1, 20.0f},
	EnemyWaveInstance{wave2, 25.0f},
	EnemyWaveInstance{wave3, 30.0f},
	EnemyWaveInstance{wave4, 35.0f},
	EnemyWaveInstance{wave1, 40.0f},
	EnemyWaveInstance{wave2, 45.0f},
}};

static const std::vector<Level> s_levels = {{level1}};

//------------------------------------------------------------------------------
constexpr float SHOT_SPEED									= 20.0f;
constexpr float ENEMY_SPAWN_OFFSET_TIME_SEC = 0.5f;

//------------------------------------------------------------------------------
Enemies::Enemies(AppContext& context)
		: m_context(context)
		, m_currentLevel(0)
		, m_nextEventWaveIdx(0)
		, m_activeWaveIdx(0)
{
	reset();
}

//------------------------------------------------------------------------------
void
Enemies::reset()
{
	m_currentLevel		 = 0;
	m_nextEventWaveIdx = 0;
	m_activeWaveIdx		 = 0;

	assert(!s_levels[m_currentLevel].waves.empty());
	m_nextEventTimeS = s_levels[m_currentLevel].waves[0].instanceTimeS;
	for (auto& e : m_enemyToWaveMap)
	{
		e = nullptr;
	}
}

//------------------------------------------------------------------------------
void
Enemies::update(const DX::StepTimer& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

	// Spawn enemies
	auto& level = s_levels[m_currentLevel];
	if (m_nextEventTimeS != 0.0f && totalTimeS >= m_nextEventTimeS) {
		// Spawn wave
		auto& numShips = level.waves[m_nextEventWaveIdx].wave.numShips;
		float delayS	 = 0.0f;
		for (int i = 0; i < numShips; ++i)
		{
			// Spawn enemy
			auto& newEnemy = m_context.entities[m_context.nextEnemyIdx];
			assert(m_enemyToWaveMap.size() > m_context.nextEnemyIdx - ENEMIES_IDX);
			m_enemyToWaveMap[m_context.nextEnemyIdx - ENEMIES_IDX]
				= &level.waves[m_nextEventWaveIdx];
			newEnemy.isAlive		= true;
			newEnemy.birthTimeS = totalTimeS + delayS;
			m_context.nextEnemyIdx++;
			if (m_context.nextEnemyIdx >= ENEMIES_END) {
				m_context.nextEnemyIdx = ENEMIES_IDX;
			}
			delayS += ENEMY_SPAWN_OFFSET_TIME_SEC;
		}

		m_nextEventWaveIdx++;
		if (m_nextEventWaveIdx < level.waves.size()) {
			m_nextEventTimeS = level.waves[m_nextEventWaveIdx].instanceTimeS;
			m_activeWaveIdx	= m_nextEventWaveIdx - 1;
		}
		else
		{
			m_activeWaveIdx		 = 0;
			m_nextEventWaveIdx = 0;
			m_nextEventTimeS	 = 0.0f;
		}
	}

	// Spawn enemy shots
	if (fmod(totalTimeS, 2.0f) < elapsedTimeS) {
		for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
		{
			if (m_context.entities[i].isAlive) {
				emitShot(
					m_context.entities[i],
					-1.0f,
					-SHOT_SPEED,
					m_context.nextEnemyShotIdx,
					ENEMY_SHOTS_IDX,
					ENEMY_SHOTS_END);
				break;
			}
		}
	}

	int countAlive = 0;
	for (size_t i = 0; i < NUM_ENTITIES; ++i)
	{
		if (m_context.entities[i].isAlive) {
			++countAlive;
		}
	}

	performPhysicsUpdate(timer);
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
Enemies::performPhysicsUpdate(const DX::StepTimer& timer)
{
	// float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS = static_cast<float>(timer.GetTotalSeconds());

	// TODO(James): make the ships move at constant speed.
	// At the moment it looks bad that the speed changes suddenly
	// when moving between curves
	static const float SEGMENT_DURATION_S = 1.2f;

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		auto& e = m_context.entities[i];
		if (!e.isAlive) {
			continue;
		}
		assert(m_enemyToWaveMap.size() > i - ENEMIES_IDX);
		assert(m_enemyToWaveMap[i - ENEMIES_IDX] != nullptr);
		const EnemyWaveInstance& instance = *m_enemyToWaveMap[i - ENEMIES_IDX];

		const float aliveS = (totalTimeS - e.birthTimeS);
		if (aliveS < 0.0f) {
			e.position = instance.wave.waypoints[0].wayPoint;
			continue;
		}

		// Enemy finished it's route
		const size_t currentSegment
			= static_cast<size_t>(floor(aliveS / SEGMENT_DURATION_S));
		if (currentSegment >= instance.wave.waypoints.size() - 1) {
			e.isAlive = false;
			continue;
		}

		const float t = fmod(aliveS, SEGMENT_DURATION_S) / SEGMENT_DURATION_S;
		e.position		= bezier(
			 t,
			 instance.wave.waypoints[currentSegment].wayPoint,
			 instance.wave.waypoints[currentSegment + 1].wayPoint,
			 instance.wave.waypoints[currentSegment + 1].controlPoint);
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
	auto& newShot		= m_context.entities[shotEntityIdx];
	newShot.isAlive = true;
	newShot.position
		= emitter.position + emitter.model->bound.Center
			+ Vector3(0.0f, (yPosScale * emitter.model->bound.Radius), 0.0f);
	newShot.velocity = Vector3(0.0f, speed, 0.0f);

	shotEntityIdx++;
	if (shotEntityIdx >= maxEntityIdxPlusOne) {
		shotEntityIdx = minEntityIdx;
	}
}

//------------------------------------------------------------------------------
void
Enemies::emitPlayerShot()
{
	emitShot(
		m_context.entities[PLAYERS_IDX],
		1.0f,
		SHOT_SPEED,
		m_context.nextPlayerShotIdx,
		PLAYER_SHOTS_IDX,
		PLAYER_SHOTS_END);
}

//------------------------------------------------------------------------------
void
Enemies::debugRender(DX::DebugBatchType* batch)
{
	std::set<const EnemyWaveInstance*> wavesToRender;
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		assert(m_enemyToWaveMap.size() > i - ENEMIES_IDX);
		if (m_enemyToWaveMap[i - ENEMIES_IDX] != nullptr) {
			wavesToRender.insert(m_enemyToWaveMap[i - ENEMIES_IDX]);
		}
	}

	static const float radius		= 0.1f;
	static const XMVECTOR xaxis = g_XMIdentityR0 * radius;
	static const XMVECTOR yaxis = g_XMIdentityR1 * radius;

	for (const auto& w : wavesToRender)
	{
		const auto& waypoints = w->wave.waypoints;
		auto prevPoint				= waypoints[0].wayPoint;
		for (size_t i = 1; i < waypoints.size(); ++i)
		{
			const auto& point		= waypoints[i].wayPoint;
			const auto& control = waypoints[i].controlPoint;

			DX::DrawCurve(batch, prevPoint, point, control);
			DX::DrawRing(batch, prevPoint, xaxis, yaxis);
			DX::DrawRing(batch, point, xaxis, yaxis);

			DX::DrawLine(batch, point, control, Colors::Yellow);
			DX::DrawRing(batch, control, xaxis, yaxis, Colors::Yellow);

			prevPoint = point;
		}
	}
}
//------------------------------------------------------------------------------
