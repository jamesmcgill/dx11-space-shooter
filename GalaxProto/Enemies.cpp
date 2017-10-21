#include "pch.h"
#include "Enemies.h"
#include "AppContext.h"
#include "AppResources.h"
#include "StepTimer.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const std::vector<Waypoint>
getDebugPath(float xPos)
{
	const float yStart = 20.0f;
	const float yEnd	 = -15.0f;
	const int ySteps	 = 10;
	const float yStep	= (yEnd - yStart) / ySteps;
	float xOscillate	 = -2.0f;

	std::vector<Waypoint> ret;
	ret.reserve(ySteps);
	float yPos = yStart;
	for (int i = 0; i < ySteps; ++i)
	{
		ret.emplace_back(Waypoint{{xPos, yPos, 0.0f},
															{xPos + xOscillate, yPos - (yStep / 2), 0.0f}});
		yPos += yStep;
		xOscillate = -xOscillate;
	}

	return ret;
}

static const Level
createDebugLevel()
{
	const int baseEnemyIdx = static_cast<int>(ModelResource::Enemy1);
	const int numEnemyTypes
		= (static_cast<int>(ModelResource::Enemy9) - baseEnemyIdx) + 1;

	const float xStart = -25.0f;
	const float xRange = xStart * -2;
	const float xStep = (numEnemyTypes > 1) ? xRange / (numEnemyTypes - 1) : 0.0f;

	std::vector<EnemyWaveSection> sections;
	for (int i = 0; i < numEnemyTypes; ++i)
	{
		ModelResource res = static_cast<ModelResource>(baseEnemyIdx + i);
		const float xPos	= xStart + (xStep * i);
		sections.emplace_back(EnemyWaveSection{getDebugPath(xPos), 1, res});
	}

	return Level{{EnemyWave{sections, 3.0f}}};
}

//------------------------------------------------------------------------------
static const std::vector<Waypoint> path1 = {
	{Vector3(-30.0f, -10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(20.0f, 10.0f, 0.0f), Vector3(20.0f, 10.0f, 0.0f)},
	{Vector3(20.0f, 18.0f, 0.0f), Vector3(35.0f, 16.0f, 0.0f)},

	{Vector3(-20.0f, 18.0f, 0.0f), Vector3(-20.0f, 18.0f, 0.0f)},
	{Vector3(-20.0f, 10.0f, 0.0f), Vector3(-35.0f, 16.0f, 0.0f)},
	{Vector3(30.0f, -10.0f, 0.0f), Vector3(30.0f, -10.0f, 0.0f)},
};

static const std::vector<Waypoint> path1Reverse = {
	{Vector3(30.0f, -10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(-20.0f, 10.0f, 0.0f), Vector3(-20.0f, 10.0f, 0.0f)},
	{Vector3(-20.0f, 18.0f, 0.0f), Vector3(-35.0f, 16.0f, 0.0f)},

	{Vector3(20.0f, 18.0f, 0.0f), Vector3(20.0f, 18.0f, 0.0f)},
	{Vector3(20.0f, 10.0f, 0.0f), Vector3(35.0f, 16.0f, 0.0f)},
	{Vector3(-30.0f, -10.0f, 0.0f), Vector3(-30.0f, -10.0f, 0.0f)},
};

// Anti-clockwise circle
static const std::vector<Waypoint>
getPath2a()
{
	Vector3 START	= Vector3(5.0f, 20.0f, 0.0f);
	Vector3 END		 = Vector3(-30.0f, 0.0f, 0.0f);
	float radius	 = 5.0f;
	Vector3 CENTER = Vector3(START.x + radius, END.y - radius, 0.0f);

	return {
		{START, Vector3()},
		{CENTER + Vector3(-radius, 0.f, 0.f), CENTER + Vector3(-radius, 0.f, 0.f)},
		{CENTER + Vector3(0.f, -radius, 0.0f),
		 CENTER + Vector3(-radius, -radius, 0.f)},
		{CENTER + Vector3(radius, 0.f, 0.f),
		 CENTER + Vector3(radius, -radius, 0.f)},
		{CENTER + Vector3(0.f, radius, 0.f), CENTER + Vector3(radius, radius, 0.f)},
		{END, END},
	};
}

// Clock-wise version of Path2A, shifted up
static const std::vector<Waypoint>
getPath2b()
{
	Vector3 START	= Vector3(-5.0f, 20.0f, 0.0f);
	Vector3 END		 = Vector3(30.0f, 5.0f, 0.0f);
	float radius	 = 5.0f;
	Vector3 CENTER = Vector3(START.x - radius, END.y - radius, 0.0f);

	return {
		{START, Vector3()},
		{CENTER + Vector3(radius, 0.f, 0.f), CENTER + Vector3(radius, 0.f, 0.f)},
		{CENTER + Vector3(0.f, -radius, 0.0f),
		 CENTER + Vector3(radius, -radius, 0.f)},
		{CENTER + Vector3(-radius, 0.f, 0.f),
		 CENTER + Vector3(-radius, -radius, 0.f)},
		{CENTER + Vector3(0.f, radius, 0.f),
		 CENTER + Vector3(-radius, radius, 0.f)},
		{END, END},
	};
}

static const std::vector<Waypoint> path91 = {
	{Vector3(10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 10.0f, 0.0f)},
	{Vector3(-10.0f, -10.0f, 0.0f), Vector3(0.0f, -5.0f, 0.0f)},
};

static const std::vector<Waypoint> path92 = {
	{Vector3(-10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
	{Vector3(10.0f, 8.0f, 0.0f), Vector3(10.0f, 10.0f, 0.0f)},
	{Vector3(-10.0f, 6.0f, 0.0f), Vector3(-10.0f, 8.0f, 0.0f)},
	{Vector3(10.0f, 4.0f, 0.0f), Vector3(10.0f, 8.0f, 0.0f)},
	{Vector3(-10.0f, 2.0f, 0.0f), Vector3(-10.0f, 4.0f, 0.0f)},
};

static const EnemyWaveSection section1
	= {getPath2a(), 3, ModelResource::Enemy9};
static const EnemyWaveSection section2
	= {getPath2b(), 3, ModelResource::Enemy2};
static const EnemyWaveSection section3 = {path1, 5, ModelResource::Enemy3};
static const EnemyWaveSection section4 = {path92, 5, ModelResource::Player};

static const Level level1 = {{
	EnemyWave{{section1, section2}, 3.0f},
	EnemyWave{{section2}, 5.0f},
	EnemyWave{{section3}, 10.0f},
	EnemyWave{{section4}, 14.0f},
	// EnemyWave{{section1}, 20.0f},
	// EnemyWave{{section2}, 25.0f},
	// EnemyWave{{section3}, 30.0f},
	// EnemyWave{{section4}, 35.0f},
	// EnemyWave{{section1}, 40.0f},
	// EnemyWave{{section2}, 45.0f},
}};

static const Level level2 = { {
		EnemyWave{ { section1, section2 }, 23.0f },
		EnemyWave{ { section2 }, 25.0f },
		EnemyWave{ { section3 }, 30.0f },
		EnemyWave{ { section4 }, 34.0f },
		// EnemyWave{{section1}, 20.0f},
		// EnemyWave{{section2}, 25.0f},
		// EnemyWave{{section3}, 30.0f},
		// EnemyWave{{section4}, 35.0f},
		// EnemyWave{{section1}, 40.0f},
		// EnemyWave{{section2}, 45.0f},
	} };


static std::vector<Level> s_debugLevels = {{createDebugLevel()}};
static std::vector<Level> s_levels			= {{level1, level2}};

//------------------------------------------------------------------------------
constexpr float SHOT_SPEED									= 40.0f;
constexpr float ENEMY_SPAWN_OFFSET_TIME_SEC = 0.5f;

//------------------------------------------------------------------------------
Enemies::Enemies(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
		, m_currentLevel(0)
		, m_nextEventWaveIdx(0)
		, m_activeWaveIdx(0)
{
	TRACE
	reset();
}

//------------------------------------------------------------------------------
void
Enemies::reset()
{
	TRACE
	m_currentLevel		 = 0;
	m_nextEventWaveIdx = 0;
	m_activeWaveIdx		 = 0;

	ASSERT(!s_levels[m_currentLevel].waves.empty());
	m_nextEventTimeS = s_levels[m_currentLevel].waves[0].instanceTimeS;
	for (auto& e : m_entityIdxToWaypoints)
	{
		e = nullptr;
	}

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_context.entities[i].isAlive			= false;
		m_context.entities[i].isColliding = false;
	}
}

//------------------------------------------------------------------------------
void
Enemies::update(const DX::StepTimer& timer)
{
	TRACE
	float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

	// Spawn enemies
	auto& level = s_levels[m_currentLevel];
	if (m_nextEventTimeS != 0.0f && totalTimeS >= m_nextEventTimeS)
	{
		// Spawn wave
		for (auto& sec : level.waves[m_nextEventWaveIdx].sections)
		{
			auto& numShips = sec.numShips;
			float delayS	 = 0.0f;
			for (int ship = 0; ship < numShips; ++ship)
			{
				// Spawn enemy
				auto& newEnemy = m_context.entities[m_context.nextEnemyIdx];
				ASSERT(
					m_entityIdxToWaypoints.size() > m_context.nextEnemyIdx - ENEMIES_IDX);
				m_entityIdxToWaypoints[m_context.nextEnemyIdx - ENEMIES_IDX]
					= &sec.waypoints;
				newEnemy.isAlive		= true;
				newEnemy.birthTimeS = totalTimeS + delayS;
				newEnemy.model			= &m_resources.modelData[sec.model];
				m_context.nextEnemyIdx++;
				if (m_context.nextEnemyIdx >= ENEMIES_END)
				{
					m_context.nextEnemyIdx = ENEMIES_IDX;
				}
				delayS += ENEMY_SPAWN_OFFSET_TIME_SEC;
			}		 // ship
		}			 // section

		m_nextEventWaveIdx++;
		if (m_nextEventWaveIdx < level.waves.size())
		{
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
	if (fmod(totalTimeS, 2.0f) < elapsedTimeS)
	{
		for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
		{
			if (m_context.entities[i].isAlive)
			{
				emitShot(
					m_context.entities[i],
					-1.0f,
					-SHOT_SPEED,
					m_context.nextEnemyShotIdx,
					ENEMY_SHOTS_IDX,
					ENEMY_SHOTS_END);
				m_resources.soundEffects[AudioResource::EnemyShot]->Play();
				break;
			}
		}
	}

	int countAlive = 0;
	for (size_t i = 0; i < NUM_ENTITIES; ++i)
	{
		if (m_context.entities[i].isAlive)
		{
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
	TRACE
	// float elapsedTimeS = float(timer.GetElapsedSeconds());
	float totalTimeS = static_cast<float>(timer.GetTotalSeconds());

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
		ASSERT(m_entityIdxToWaypoints.size() > i - ENEMIES_IDX);
		ASSERT(m_entityIdxToWaypoints[i - ENEMIES_IDX] != nullptr);
		auto& waypoints = *m_entityIdxToWaypoints[i - ENEMIES_IDX];

		const float aliveS = (totalTimeS - e.birthTimeS);
		if (aliveS < 0.0f)
		{
			e.position = waypoints[0].wayPoint;
			continue;
		}

		// Enemy finished it's route
		const size_t currentSegment
			= static_cast<size_t>(floor(aliveS / SEGMENT_DURATION_S));
		if (currentSegment >= waypoints.size() - 1)
		{
			e.isAlive = false;
			continue;
		}

		const float t = fmod(aliveS, SEGMENT_DURATION_S) / SEGMENT_DURATION_S;
		e.position		= bezier(
			 t,
			 waypoints[currentSegment].wayPoint,
			 waypoints[currentSegment + 1].wayPoint,
			 waypoints[currentSegment + 1].controlPoint);
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
Enemies::debugRender(DX::DebugBatchType* batch)
{
	TRACE
	std::set<const std::vector<Waypoint>*> waypointsToRender;
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		ASSERT(m_entityIdxToWaypoints.size() > i - ENEMIES_IDX);
		if (m_entityIdxToWaypoints[i - ENEMIES_IDX] != nullptr)
		{
			waypointsToRender.insert(m_entityIdxToWaypoints[i - ENEMIES_IDX]);
		}
	}

	static const float radius		= 0.1f;
	static const XMVECTOR xaxis = g_XMIdentityR0 * radius;
	static const XMVECTOR yaxis = g_XMIdentityR1 * radius;

	for (auto w : waypointsToRender)
	{
		const auto& waypoints = *w;
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
void
Enemies::debug_toggleLevel()
{
	s_levels.swap(s_debugLevels);
}

//------------------------------------------------------------------------------
std::vector<Level>&
Enemies::debug_getCurrentLevels()
{
	return s_levels;
}

//------------------------------------------------------------------------------
