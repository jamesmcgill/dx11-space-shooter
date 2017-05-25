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
}

//------------------------------------------------------------------------------
void
GameMaster::Update(DX::StepTimer const& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

	// Spawn enemies
	if (fmod(totalTimeS, 2.0f) < elapsedTimeS) {
		for (auto i = 0; i < 5; ++i)
		{
			// Create Enemy
			auto enemyIdx		 = (m_state.nextEnemyIdx - ENEMIES_IDX);
			auto row				 = floor(enemyIdx / 5.0f);
			auto col				 = static_cast<float>(fmod(enemyIdx, 5.0f));
			auto& newEnemy	 = m_state.entities[m_state.nextEnemyIdx];
			newEnemy.isAlive = true;
			newEnemy.position
				= ENEMY_START_POS + Vector3(col * 5.0f, row * 5.0f, 0.0f);

			m_state.nextEnemyIdx++;
			if (m_state.nextEnemyIdx >= ENEMIES_END) {
				m_state.nextEnemyIdx = ENEMIES_IDX;
			}
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
GameMaster::LoadWaveData()
{
	m_waypoints.clear();

	m_waypoints.emplace_back(
		Waypoint({Vector3(5.0f, 5.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)}));

	m_waypoints.emplace_back(
		Waypoint({Vector3(0.0f, 2.5f, 0.0f), Vector3(0.0f, 5.0f, 0.0f)}));

	m_waypoints.emplace_back(
		Waypoint({Vector3(-5.0f, 0.0f, 0.0f), Vector3(0.0f, -5.0f, 0.0f)}));
}

//------------------------------------------------------------------------------
void
GameMaster::RenderWaveData()
{
}
//------------------------------------------------------------------------------