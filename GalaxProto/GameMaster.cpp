#include "pch.h"
#include "GameMaster.h"

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

static const EnemyWaveInstance waveInstance1{wave1, 5.0f};
static const EnemyWaveInstance waveInstance2{wave2, 10.0f};
static const EnemyWaveInstance waveInstance3{wave3, 20.0f};
static const EnemyWaveInstance waveInstance4{wave4, 30.0f};
static const EnemyWaveInstance waveInstance5{wave1, 40.0f};
static const EnemyWaveInstance waveInstance6{wave2, 50.0f};

static const Level level1 = {{waveInstance1,
															waveInstance2,
															waveInstance3,
															waveInstance4,
															waveInstance5,
															waveInstance6}};

static const std::vector<Level> s_levels = {{level1}};

//------------------------------------------------------------------------------
constexpr float SHOT_SPEED = 20.0f;

//------------------------------------------------------------------------------
GameMaster::GameMaster(GameState& gameState)
		: m_state(gameState)
		, m_currentLevel(0)
		, m_nextEventWaveIdx(0)
		, m_activeWaveIdx(0)
{
	assert(!s_levels[m_currentLevel].waves.empty());
	m_nextEventTimeS = s_levels[m_currentLevel].waves[0].instanceTimeS;
	for (auto& e : m_enemyToWaveMap)
	{
		e = nullptr;
	}
}

//------------------------------------------------------------------------------
void
GameMaster::update(const DX::StepTimer& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

	// Spawn enemies
	auto& level = s_levels[m_currentLevel];
	if (m_nextEventTimeS != 0.0f && totalTimeS >= m_nextEventTimeS) {
		// Spawn wave
		auto& numShips = level.waves[m_nextEventWaveIdx].wave.numShips;
		for (int i = 0; i < numShips; ++i)
		{
			// Spawn enemy
			auto& newEnemy = m_state.entities[m_state.nextEnemyIdx];
			m_enemyToWaveMap[m_state.nextEnemyIdx - ENEMIES_IDX]
				= &level.waves[m_nextEventWaveIdx];
			newEnemy.isAlive = true;
			m_state.nextEnemyIdx++;
			if (m_state.nextEnemyIdx >= ENEMIES_END) {
				m_state.nextEnemyIdx = ENEMIES_IDX;
			}
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
			if (m_state.entities[i].isAlive) {
				emitShot(
					m_state.entities[i],
					-1.0f,
					-SHOT_SPEED,
					m_state.nextEnemyShotIdx,
					ENEMY_SHOTS_IDX,
					ENEMY_SHOTS_END);
				break;
			}
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
GameMaster::performPhysicsUpdate(const DX::StepTimer& timer)
{
	// float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS = static_cast<float>(timer.GetTotalSeconds());

	// TODO(James): make the ships move at constant speed.
	// At the moment it looks bad that the speed changes suddenly
	// when moving between curves
	const float segmentDurationS = 1.2f;

	// TODO(James):
	// 2) multiple waves at once
	// 3) multiple enemies per wave (still using t)

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		auto& e = m_state.entities[i];
		if (!e.isAlive) {
			continue;
		}
		assert(m_enemyToWaveMap[i - ENEMIES_IDX] != nullptr);
		const EnemyWaveInstance& instance = *m_enemyToWaveMap[i - ENEMIES_IDX];

		const float aliveS = (totalTimeS - instance.instanceTimeS);
		const size_t currentSegment
			= static_cast<size_t>(floor(aliveS / segmentDurationS));
		if (currentSegment >= instance.wave.waypoints.size() - 1) {
			e.isAlive = false;
			continue;
		}

		const float t = fmod(aliveS, segmentDurationS) / segmentDurationS;
		e.position		= bezier(
			 t,
			 instance.wave.waypoints[currentSegment].wayPoint,
			 instance.wave.waypoints[currentSegment + 1].wayPoint,
			 instance.wave.waypoints[currentSegment + 1].controlPoint);
	}
}

//------------------------------------------------------------------------------
void
GameMaster::emitShot(
	const Entity& emitter,
	const float yPosScale,
	const float speed,
	size_t& shotEntityIdx,
	const size_t minEntityIdx,
	const size_t maxEntityIdxPlusOne)
{
	auto& newShot		= m_state.entities[shotEntityIdx];
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
GameMaster::emitPlayerShot()
{
	emitShot(
		m_state.entities[PLAYERS_IDX],
		1.0f,
		SHOT_SPEED,
		m_state.nextPlayerShotIdx,
		PLAYER_SHOTS_IDX,
		PLAYER_SHOTS_END);
}

//------------------------------------------------------------------------------
void
GameMaster::debugRender(DX::DebugBatchType* batch)
{
	std::set<const EnemyWaveInstance*> wavesToRender;
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		if (m_enemyToWaveMap[i - ENEMIES_IDX] != nullptr) {
			wavesToRender.insert(m_enemyToWaveMap[i - ENEMIES_IDX]);
		}
	}

	const float radius = 0.1f;
	XMVECTOR xaxis		 = g_XMIdentityR0 * radius;
	XMVECTOR yaxis		 = g_XMIdentityR1 * radius;

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