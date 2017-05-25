#include "pch.h"
#include "GameMaster.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
const Vector3 ENEMY_START_POS(-10.0f, 3.0f, 0.0f);
constexpr float SHOT_SPEED = 20.0f;

//------------------------------------------------------------------------------
GameMaster::GameMaster(GameState& gameState)
		: m_state(gameState)
{
	// TODO(James)
	loadWaveData();
}

//------------------------------------------------------------------------------
void
GameMaster::update(const DX::StepTimer& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

// Spawn enemies
#if 0
	if (fmod(totalTimeS, 2.0f) < elapsedTimeS) {
		//	for (auto i = 0; i < 5; ++i)
		//	{
		//		// Create Enemy
		// auto enemyIdx			= (m_state.nextEnemyIdx - ENEMIES_IDX);
		// auto row					= floor(enemyIdx / 5.0f);
		// auto col					= static_cast<float>(fmod(enemyIdx, 5.0f));
		auto& newEnemy	 = m_state.entities[m_state.nextEnemyIdx];
		newEnemy.isAlive = true;
		// newEnemy.position = ENEMY_START_POS + Vector3(col * 5.0f, row * 5.0f,
		// 0.0f);
		newEnemy.position = m_waypoints[0].wayPoint;

		m_state.nextEnemyIdx++;
		if (m_state.nextEnemyIdx >= ENEMIES_END) {
			m_state.nextEnemyIdx = ENEMIES_IDX;
		}
		//	}
	}
#else
	if (!m_isWaveSpawned && fmod(totalTimeS, 3.0f) < elapsedTimeS) {
		auto& newEnemy	 = m_state.entities[m_state.nextEnemyIdx];
		newEnemy.isAlive = true;
		m_waveSpawnTime	= totalTimeS;
		m_isWaveSpawned	= true;
	}

#endif

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

	const float segmentDurationS = 1.2f;
	const float aliveS					 = (totalTimeS - m_waveSpawnTime);
	const size_t currentSegment
		= static_cast<size_t>(floor(aliveS / segmentDurationS));

	if (currentSegment >= m_waypoints.size() - 1) {
		m_isWaveSpawned																 = false;
		m_state.entities[m_state.nextEnemyIdx].isAlive = false;
		return;
	}

	const float t = fmod(aliveS, segmentDurationS) / segmentDurationS;

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		auto& e = m_state.entities[i];
		if (!e.isAlive) {
			continue;
		}
		e.position = bezier(
			t,
			m_waypoints[currentSegment].wayPoint,
			m_waypoints[currentSegment + 1].wayPoint,
			m_waypoints[currentSegment + 1].controlPoint);
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
GameMaster::loadWaveData()
{
	m_waypoints.clear();

	m_waypoints.emplace_back(
		Waypoint({Vector3(10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)}));

	m_waypoints.emplace_back(
		Waypoint({Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 10.0f, 0.0f)}));

	m_waypoints.emplace_back(
		Waypoint({Vector3(-10.0f, -10.0f, 0.0f), Vector3(0.0f, -5.0f, 0.0f)}));
}

//------------------------------------------------------------------------------
void
GameMaster::debugRender(DX::DebugBatchType* batch)
{
	if (m_waypoints.empty()) {
		return;
	}

	const float radius = 0.1f;
	XMVECTOR xaxis		 = g_XMIdentityR0 * radius;
	XMVECTOR yaxis		 = g_XMIdentityR1 * radius;

	auto prevPoint = m_waypoints[0].wayPoint;
	for (size_t i = 1; i < m_waypoints.size(); ++i)
	{
		const auto& point		= m_waypoints[i].wayPoint;
		const auto& control = m_waypoints[i].controlPoint;

		DX::DrawCurve(batch, prevPoint, point, control);
		DX::DrawRing(batch, prevPoint, xaxis, yaxis);
		DX::DrawRing(batch, point, xaxis, yaxis);

		DX::DrawLine(batch, point, control, Colors::Yellow);
		DX::DrawRing(batch, control, xaxis, yaxis, Colors::Yellow);

		prevPoint = point;
	}
}
//------------------------------------------------------------------------------