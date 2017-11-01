#include "pch.h"
#include "Enemies.h"
#include "AppContext.h"
#include "AppResources.h"
#include "StepTimer.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const Path
createDebugPath(float xPos)
{
	const float yStart = 20.0f;
	const float yEnd	 = -15.0f;
	const int ySteps	 = 10;
	const float yStep	= (yEnd - yStart) / ySteps;
	float xOscillate	 = -2.0f;

	static int count = 0;
	Path ret{fmt::format(L"debug path {} ", count++)};
	ret.waypoints.reserve(ySteps);
	float yPos = yStart;
	for (int i = 0; i < ySteps; ++i)
	{
		ret.waypoints.emplace_back(Waypoint{
			{xPos, yPos, 0.0f}, {xPos + xOscillate, yPos - (yStep / 2), 0.0f}});
		yPos += yStep;
		xOscillate = -xOscillate;
	}

	return ret;
}

static const Level
createDebugLevel(PathPool& pathPool, FormationPool& formationPool)
{
	const int baseEnemyIdx = static_cast<int>(ModelResource::Enemy1);
	const int numEnemyTypes
		= (static_cast<int>(ModelResource::Enemy9) - baseEnemyIdx) + 1;

	const float xStart = -25.0f;
	const float xRange = xStart * -2;
	const float xStep = (numEnemyTypes > 1) ? xRange / (numEnemyTypes - 1) : 0.0f;

	auto& formation = formationPool.emplace_back(Formation());
	formation.id		= L"DebugFormation";
	for (int i = 0; i < numEnemyTypes; ++i)
	{
		ModelResource res = static_cast<ModelResource>(baseEnemyIdx + i);
		const float xPos	= xStart + (xStep * i);
		pathPool.emplace_back(createDebugPath(xPos));

		formation.sections.emplace_back(
			FormationSection{pathPool.size() - 1, 1, res});
	}

	return Level{{Wave{3.0f, formationPool.size() - 1}}};
}

//------------------------------------------------------------------------------
static const Path path1 = {
	L"path1",
	{
		{Vector3(-30.0f, -10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		{Vector3(20.0f, 10.0f, 0.0f), Vector3(20.0f, 10.0f, 0.0f)},
		{Vector3(20.0f, 18.0f, 0.0f), Vector3(35.0f, 16.0f, 0.0f)},

		{Vector3(-20.0f, 18.0f, 0.0f), Vector3(-20.0f, 18.0f, 0.0f)},
		{Vector3(-20.0f, 10.0f, 0.0f), Vector3(-35.0f, 16.0f, 0.0f)},
		{Vector3(30.0f, -10.0f, 0.0f), Vector3(30.0f, -10.0f, 0.0f)},
	},
};

static const Path path1Reverse = {
	L"path1Reverse",
	{
		{Vector3(30.0f, -10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		{Vector3(-20.0f, 10.0f, 0.0f), Vector3(-20.0f, 10.0f, 0.0f)},
		{Vector3(-20.0f, 18.0f, 0.0f), Vector3(-35.0f, 16.0f, 0.0f)},

		{Vector3(20.0f, 18.0f, 0.0f), Vector3(20.0f, 18.0f, 0.0f)},
		{Vector3(20.0f, 10.0f, 0.0f), Vector3(35.0f, 16.0f, 0.0f)},
		{Vector3(-30.0f, -10.0f, 0.0f), Vector3(-30.0f, -10.0f, 0.0f)},
	},
};

// Anti-clockwise circle
static const Path
getPath2a()
{
	Vector3 START	= Vector3(5.0f, 20.0f, 0.0f);
	Vector3 END		 = Vector3(-30.0f, 0.0f, 0.0f);
	float radius	 = 5.0f;
	Vector3 CENTER = Vector3(START.x + radius, END.y - radius, 0.0f);

	return {
		L"path2a",
		{
			{START, Vector3()},
			{CENTER + Vector3(-radius, 0.f, 0.f),
			 CENTER + Vector3(-radius, 0.f, 0.f)},
			{CENTER + Vector3(0.f, -radius, 0.0f),
			 CENTER + Vector3(-radius, -radius, 0.f)},
			{CENTER + Vector3(radius, 0.f, 0.f),
			 CENTER + Vector3(radius, -radius, 0.f)},
			{CENTER + Vector3(0.f, radius, 0.f),
			 CENTER + Vector3(radius, radius, 0.f)},
			{END, END},
		},
	};
}

// Clock-wise version of Path2A, shifted up
static const Path
getPath2b()
{
	Vector3 START	= Vector3(-5.0f, 20.0f, 0.0f);
	Vector3 END		 = Vector3(30.0f, 5.0f, 0.0f);
	float radius	 = 5.0f;
	Vector3 CENTER = Vector3(START.x - radius, END.y - radius, 0.0f);

	return {
		L"path2b",
		{
			{START, Vector3()},
			{CENTER + Vector3(radius, 0.f, 0.f), CENTER + Vector3(radius, 0.f, 0.f)},
			{CENTER + Vector3(0.f, -radius, 0.0f),
			 CENTER + Vector3(radius, -radius, 0.f)},
			{CENTER + Vector3(-radius, 0.f, 0.f),
			 CENTER + Vector3(-radius, -radius, 0.f)},
			{CENTER + Vector3(0.f, radius, 0.f),
			 CENTER + Vector3(-radius, radius, 0.f)},
			{END, END},
		},
	};
}

static const Path path91 = {
	L"path91",
	{
		{Vector3(10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 10.0f, 0.0f)},
		{Vector3(-10.0f, -10.0f, 0.0f), Vector3(0.0f, -5.0f, 0.0f)},
	},
};

static const Path path92 = {
	L"path92",
	{
		{Vector3(-10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		{Vector3(10.0f, 8.0f, 0.0f), Vector3(10.0f, 10.0f, 0.0f)},
		{Vector3(-10.0f, 6.0f, 0.0f), Vector3(-10.0f, 8.0f, 0.0f)},
		{Vector3(10.0f, 4.0f, 0.0f), Vector3(10.0f, 8.0f, 0.0f)},
		{Vector3(-10.0f, 2.0f, 0.0f), Vector3(-10.0f, 4.0f, 0.0f)},
	},
};

//------------------------------------------------------------------------------
static const std::vector<Level>
createTestLevels(PathPool& pathPool, FormationPool& formationPool)
{
	// Paths
	size_t pathOffset = pathPool.size();
	PathPool paths		= {
		 path1,
		 path1Reverse,
		 getPath2a(),
		 getPath2b(),
		 path91,
		 path92,
	};
	pathPool.insert(pathPool.end(), paths.begin(), paths.end());

	const size_t path1Idx				 = pathOffset + 0;
	const size_t path1ReverseIdx = pathOffset + 1;
	const size_t path2aIdx			 = pathOffset + 2;
	const size_t path2bIdx			 = pathOffset + 3;
	const size_t path91Idx			 = pathOffset + 4;
	const size_t path92Idx			 = pathOffset + 5;

	// Formations
	FormationSection section1 = {path2aIdx, 3, ModelResource::Enemy9};
	FormationSection section2 = {path2bIdx, 3, ModelResource::Enemy2};
	FormationSection section3 = {path1Idx, 5, ModelResource::Enemy3};
	FormationSection section4 = {path92Idx, 5, ModelResource::Player};

	size_t formOffset								= formationPool.size();
	static FormationPool formations = {
		{L"simple", {section1}},
		{L"multi", {section1, section2}},
		{L"many", {section3}},
		{L"players", {section4}},
	};

	formationPool.insert(
		formationPool.end(), formations.begin(), formations.end());
	static Level level1 = {{
		Wave{3.0f, formOffset + 0},
		Wave{5.0f, formOffset + 1},
		Wave{10.0f, formOffset + 2},
		Wave{14.0f, formOffset + 3},
		Wave{18.0f, formOffset + 0},
	}};

	static Level level2 = {{
		Wave{23.0f, formOffset + 0},
		Wave{25.0f, formOffset + 1},
		Wave{30.0f, formOffset + 2},
		Wave{34.0f, formOffset + 3},
	}};

	return {level1, level2};
}
//------------------------------------------------------------------------------

static std::vector<Level> s_debugLevels;
static std::vector<Level> s_levels;

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
	m_formationPool.reserve(MAX_NUM_FORMATIONS);
	m_pathPool.reserve(MAX_NUM_PATHS);

	s_debugLevels = {{createDebugLevel(m_pathPool, m_formationPool)}};
	s_levels			= {{createTestLevels(m_pathPool, m_formationPool)}};

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

	for (auto& e : m_entityIdxToPath)
	{
		e = nullptr;
	}

	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_context.entities[i].isAlive			= false;
		m_context.entities[i].isColliding = false;
	}

	m_nextEventTimeS = 0.0f;
	if (
		(m_currentLevel < s_levels.size())
		&& (!s_levels[m_currentLevel].waves.empty()))
	{
		m_nextEventTimeS = s_levels[m_currentLevel].waves[0].spawnTimeS;
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
	if (m_currentLevel >= s_levels.size())
	{
		return;
	}
	auto& level = s_levels[m_currentLevel];
	if (m_nextEventTimeS != 0.0f && totalTimeS >= m_nextEventTimeS)
	{
		// Spawn wave
		ASSERT(m_nextEventWaveIdx < level.waves.size());
		const auto& formationIdx = level.waves[m_nextEventWaveIdx].formationIdx;
		ASSERT(formationIdx < m_formationPool.size());
		auto& formation = m_formationPool[formationIdx];
		for (auto& sec : formation.sections)
		{
			auto& numShips = sec.numShips;
			float delayS	 = 0.0f;
			for (int ship = 0; ship < numShips; ++ship)
			{
				// Spawn enemy
				auto& newEnemy = m_context.entities[m_context.nextEnemyIdx];
				ASSERT(m_entityIdxToPath.size() > m_context.nextEnemyIdx - ENEMIES_IDX);
				m_entityIdxToPath[m_context.nextEnemyIdx - ENEMIES_IDX]
					= &m_pathPool[sec.pathIdx];
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
			m_nextEventTimeS = level.waves[m_nextEventWaveIdx].spawnTimeS;
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
		ASSERT(m_entityIdxToPath.size() > i - ENEMIES_IDX);
		ASSERT(m_entityIdxToPath[i - ENEMIES_IDX] != nullptr);
		auto& path = *m_entityIdxToPath[i - ENEMIES_IDX];

		const float aliveS = (totalTimeS - e.birthTimeS);
		if (aliveS < 0.0f)
		{
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
	std::set<const Path*> pathsToRender;
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		ASSERT(m_entityIdxToPath.size() > i - ENEMIES_IDX);
		if (m_entityIdxToPath[i - ENEMIES_IDX] != nullptr)
		{
			pathsToRender.insert(m_entityIdxToPath[i - ENEMIES_IDX]);
		}
	}

	static const float radius		= 0.1f;
	static const XMVECTOR xaxis = g_XMIdentityR0 * radius;
	static const XMVECTOR yaxis = g_XMIdentityR1 * radius;

	for (const auto& p : pathsToRender)
	{
		const auto& waypoints = (*p).waypoints;
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
