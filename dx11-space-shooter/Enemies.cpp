#include "pch.h"
#include "Enemies.h"
#include "AppContext.h"
#include "AppResources.h"
#include "StepTimer.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

static const std::string LEVEL_DATA_FILENAME = "assets/leveldata.json";
static const std::string PATHS_NODE_ID			 = "paths";
static const std::string FORMATIONS_NODE_ID	= "formations";
static const std::string LEVELS_NODE_ID			 = "levels";
static const std::string ID_NODE_KEY				 = "id";
static const std::string WAYPOINTS_KEY			 = "waypoints";
static const std::string SECTIONS_KEY				 = "sections";

static const std::string WAYPOINT_KEY = "waypoint";
static const std::string CONTROL_KEY	= "control";

static const std::string PATH_IDX_KEY	= "pathIdx";
static const std::string NUM_SHIPS_KEY = "numShips";
static const std::string MODEL_KEY		 = "model";

static const std::string SPAWN_TIME_KEY		 = "spawnTimeS";
static const std::string FORMATION_IDX_KEY = "formationIdx";

static const std::string WAVES_KEY = "waves";

//------------------------------------------------------------------------------
constexpr float SHOT_SPEED = 40.0f;
constexpr float ENEMY_SPAWN_OFFSET_TIME_SEC = 0.5f;

//------------------------------------------------------------------------------
Waypoint
Waypoint::from_json(const json11::Json& json)
{
	Waypoint ret{};

	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Waypoint object - not a JSON object type");
		return ret;
	}
	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (!value.is_array() || value.array_items().size() != 3)
		{
			LOG_ERROR(
				"Can't parse Waypoint coordinate - JSON array of size 3 required");
			continue;
		}
		auto& point = (key == CONTROL_KEY) ? ret.controlPoint : ret.wayPoint;
		float pts[3];
		int i = 0;
		for (const auto& coord : value.array_items())
		{
			if (coord.is_number())
			{
				ASSERT(i < 3);
				pts[i++] = static_cast<float>(coord.number_value());
			}
		}
		point = Vector3(pts);
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
Waypoint::to_json() const
{
	return json11::Json::object{
		{WAYPOINT_KEY, json11::Json::array{wayPoint.x, wayPoint.y, wayPoint.z}},
		{CONTROL_KEY,
		 json11::Json::array{controlPoint.x, controlPoint.y, controlPoint.z}}};
}

//------------------------------------------------------------------------------
void
Path::debugRender(
	DX::DebugBatchType* batch,
	size_t selectedPointIdx,
	size_t selectedControlIdx) const
{
	static const float radius		= 0.6f;
	static const XMVECTOR xaxis = g_XMIdentityR0 * radius;
	static const XMVECTOR yaxis = g_XMIdentityR1 * radius;

	static const XMVECTOR SELECTED_COLOR = Colors::OrangeRed;
	static const XMVECTOR POINT_COLOR		 = Colors::White;
	static const XMVECTOR CONTROL_COLOR	= Colors::Yellow;

	ASSERT(!waypoints.empty());
	auto prevPoint = waypoints[0].wayPoint;
	DX::DrawRing(
		batch,
		prevPoint,
		xaxis,
		yaxis,
		(selectedPointIdx == 0) ? SELECTED_COLOR : POINT_COLOR);

	for (size_t i = 1; i < waypoints.size(); ++i)
	{
		const auto& point		= waypoints[i].wayPoint;
		const auto& control = waypoints[i].controlPoint;

		DX::DrawCurve(batch, prevPoint, point, control);
		DX::DrawRing(
			batch,
			point,
			xaxis,
			yaxis,
			(selectedPointIdx == i) ? SELECTED_COLOR : POINT_COLOR);

		DX::DrawLine(batch, point, control, Colors::Yellow);
		DX::DrawRing(
			batch,
			control,
			xaxis,
			yaxis,
			(selectedControlIdx == i) ? SELECTED_COLOR : CONTROL_COLOR);

		prevPoint = point;
	}
}

//------------------------------------------------------------------------------
Path
Path::from_json(const json11::Json& json)
{
	Path ret{};
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Path object - not a JSON object type");
		return ret;
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (value.is_string() && key == ID_NODE_KEY)
		{
			ret.id = strUtils::utf8ToWstring(value.string_value());
		}
		else if (value.is_array() && key == WAYPOINTS_KEY)
		{
			for (const auto& waypoint : value.array_items())
			{
				ret.waypoints.emplace_back(Waypoint::from_json(waypoint));
			}
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
Path::to_json() const
{
	return json11::Json::object{{ID_NODE_KEY, strUtils::wstringToUtf8(id)},
															{WAYPOINTS_KEY, waypoints}};
}

//------------------------------------------------------------------------------
FormationSection
FormationSection::from_json(const json11::Json& json)
{
	FormationSection ret{};
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse FormationSection object - not a JSON object type");
		return ret;
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (value.is_number() && key == PATH_IDX_KEY)
		{
			ret.pathIdx = static_cast<size_t>(value.int_value());
		}
		else if (value.is_number() && key == NUM_SHIPS_KEY)
		{
			ret.numShips = value.int_value();
		}
		else if (value.is_number() && key == MODEL_KEY)
		{
			ret.model = static_cast<ModelResource>(value.int_value());
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
FormationSection::to_json() const
{
	return json11::Json::object{{PATH_IDX_KEY, static_cast<int>(pathIdx)},
															{NUM_SHIPS_KEY, numShips},
															{MODEL_KEY, static_cast<int>(model)}};
}

//------------------------------------------------------------------------------
Formation
Formation::from_json(const json11::Json& json)
{
	Formation ret{};
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Formation object - not a JSON object type");
		return ret;
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (value.is_string() && key == ID_NODE_KEY)
		{
			ret.id = strUtils::utf8ToWstring(value.string_value());
		}
		else if (value.is_array() && key == SECTIONS_KEY)
		{
			for (const auto& section : value.array_items())
			{
				ret.sections.emplace_back(FormationSection::from_json(section));
			}
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
Formation::to_json() const
{
	return json11::Json::object{{ID_NODE_KEY, strUtils::wstringToUtf8(id)},
															{SECTIONS_KEY, sections}};
}

//------------------------------------------------------------------------------
Wave
Wave::from_json(const json11::Json& json)
{
	Wave ret{};
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Wave object - not a JSON object type");
		return ret;
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (value.is_number() && key == FORMATION_IDX_KEY)
		{
			ret.formationIdx = value.int_value();
		}
		else if (value.is_number() && key == SPAWN_TIME_KEY)
		{
			ret.spawnTimeS = static_cast<float>(value.number_value());
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
Wave::to_json() const
{
	return json11::Json::object{
		{SPAWN_TIME_KEY, spawnTimeS},
		{FORMATION_IDX_KEY, static_cast<int>(formationIdx)}};
}

//------------------------------------------------------------------------------
Level
Level::from_json(const json11::Json& json)
{
	Level ret{};
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Level object - not a JSON object type");
		return ret;
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		const auto& key		= child.first;
		const auto& value = child.second;

		if (value.is_array() && key == WAVES_KEY)
		{
			for (const auto& wave : value.array_items())
			{
				ret.waves.emplace_back(Wave::from_json(wave));
			}
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
json11::Json
Level::to_json() const
{
	return json11::Json::object{{WAVES_KEY, waves}};
}

//------------------------------------------------------------------------------
static void
addNullData(PathPool& pathPool, FormationPool& formationPool)
{
	static const Path nullPath = {
		L"nullPath",
		{
			{Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
		},
	};
	pathPool.emplace_back(nullPath);
	size_t nullPathIdx = pathPool.size() - 1;

	const int shipCount = 1;
	auto& formation			= formationPool.emplace_back(Formation());
	formation.id				= L"nullFormation";
	formation.sections.emplace_back(
		FormationSection{nullPathIdx, shipCount, ModelResource::Enemy1});
}

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

	// Prevent data ever becoming truely empty with 'null' versions at the front
	addNullData(m_pathPool, m_formationPool);
	nullPathIdx			 = m_pathPool.size() - 1;
	nullFormationIdx = m_formationPool.size() - 1;
}

//------------------------------------------------------------------------------
void
Enemies::reset()
{
	TRACE
	resetCurrentTime();
	m_currentLevelIdx	= 0;
	m_nextEventWaveIdx = 0;

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
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());
	incrementCurrentTime(timer);

	updateLevel();

	// Spawn enemy shots
	if (fmod(m_currentLevelTimeS, 2.0f) < elapsedTimeS)
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
		// Activate next level
		if (m_currentLevelIdx < m_levels.size())
		{
			if (!isAnyEnemyAlive())
			{
				m_isLevelActive = true;
				resetCurrentTime();
			}
		}
		return;
	}

	auto& level = m_levels[m_currentLevelIdx];
	ASSERT(m_nextEventWaveIdx < level.waves.size());
	auto& nextWave = level.waves[m_nextEventWaveIdx];

	// Spawn next wave
	if (m_currentLevelTimeS >= nextWave.spawnTimeS)
	{
		spawnFormation(nextWave.formationIdx, m_currentLevelTimeS);

		m_nextEventWaveIdx++;
		if (m_nextEventWaveIdx >= level.waves.size())
		{
			m_isLevelActive		 = false;
			m_nextEventWaveIdx = 0;
			m_currentLevelIdx++;
		}
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

	std::ifstream fileIn(LEVEL_DATA_FILENAME);
	if (!fileIn.is_open())
	{
		LOG_ERROR("Level file could not be found\n");
		return;
	}

	std::stringstream ss;
	ss << fileIn.rdbuf();
	fileIn.close();

	std::string err;
	const auto& str = ss.str();

	parseRootJsonArray(json11::Json::parse(str, err));
}

//------------------------------------------------------------------------------
void
Enemies::parseRootJsonArray(const json11::Json& json)
{
	TRACE
	if (!json.is_array())
	{
		LOG_ERROR("Can't parse Root Array - not a JSON array type");
	}

	const auto& children = json.array_items();
	for (const auto& child : children)
	{
		parseRootJsonObject(child);
	}
}

//------------------------------------------------------------------------------
void
Enemies::parseRootJsonObject(const json11::Json& json)
{
	TRACE
	if (!json.is_object())
	{
		LOG_ERROR("Can't parse Root Object - not a JSON object type");
	}

	const auto& children = json.object_items();
	for (const auto& child : children)
	{
		if (child.first == PATHS_NODE_ID)
		{
			parsePathsJsonObject(child.second);
		}
		else if (child.first == FORMATIONS_NODE_ID)
		{
			parseFormationsJsonObject(child.second);
		}
		else if (child.first == LEVELS_NODE_ID)
		{
			parseLevelsJsonObject(child.second);
		}
	}
}

//------------------------------------------------------------------------------
void
Enemies::parsePathsJsonObject(const json11::Json& json)
{
	TRACE
	if (!json.is_array())
	{
		LOG_ERROR("Can't parse Paths - not a JSON array type");
		return;
	}

	for (const auto& path : json.array_items())
	{
		m_pathPool.emplace_back(Path::from_json(path));
	}
}

//------------------------------------------------------------------------------
void
Enemies::parseFormationsJsonObject(const json11::Json& json)
{
	TRACE
	if (!json.is_array())
	{
		LOG_ERROR("Can't parse Formations - not a JSON array type");
		return;
	}

	for (const auto& formation : json.array_items())
	{
		m_formationPool.emplace_back(Formation::from_json(formation));
	}
}
//------------------------------------------------------------------------------
void
Enemies::parseLevelsJsonObject(const json11::Json& json)
{
	TRACE
	if (!json.is_array())
	{
		LOG_ERROR("Can't parse Levels - not a JSON array type");
		return;
	}

	for (const auto& level : json.array_items())
	{
		m_levels.emplace_back(Level::from_json(level));
	}
}

//------------------------------------------------------------------------------
void
Enemies::save()
{
	TRACE
	std::ofstream fileOut(LEVEL_DATA_FILENAME);
	if (fileOut.is_open())
	{
		// NB: begin()+1 skips the null items artificially injected at index[0]
		// We don't need to save those
		auto paths = json11::Json(m_pathPool.begin() + 1, m_pathPool.end());
		auto formations
			= json11::Json(m_formationPool.begin() + 1, m_formationPool.end());

		fileOut << json11::Json(
								 json11::Json::array{
									 json11::Json::object{{PATHS_NODE_ID, paths}},
									 json11::Json::object{{FORMATIONS_NODE_ID, formations}},
									 json11::Json::object{{LEVELS_NODE_ID, m_levels}}})
								 .dump();
	}
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
