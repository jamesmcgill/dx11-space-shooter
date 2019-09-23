#include "pch.h"
#include "LevelData.h"
#include "json11/json11.hpp"

#include "utils/Log.h"

//------------------------------------------------------------------------------
static const std::string LEVEL_DATA_FILENAME = "assets/leveldata.json";
static const std::string PATHS_NODE_ID       = "paths";
static const std::string FORMATIONS_NODE_ID  = "formations";
static const std::string LEVELS_NODE_ID      = "levels";
static const std::string ID_NODE_KEY         = "id";
static const std::string WAYPOINTS_KEY       = "waypoints";
static const std::string SECTIONS_KEY        = "sections";

static const std::string WAYPOINT_KEY = "waypoint";
static const std::string CONTROL_KEY  = "control";

static const std::string PATH_ID_KEY   = "pathId";
static const std::string NUM_SHIPS_KEY = "numShips";
static const std::string MODEL_KEY     = "model";

static const std::string SPAWN_TIME_KEY   = "spawnTimeS";
static const std::string FORMATION_ID_KEY = "formationId";

static const std::string WAVES_KEY = "waves";
//------------------------------------------------------------------------------

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
    const auto& key   = child.first;
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
    point = DirectX::SimpleMath::Vector3(pts);
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
  using DirectX::XMVECTOR;

  static const float radius   = 0.6f;
  static const XMVECTOR xaxis = DirectX::g_XMIdentityR0 * radius;
  static const XMVECTOR yaxis = DirectX::g_XMIdentityR1 * radius;

  static const XMVECTOR SELECTED_COLOR = DirectX::Colors::OrangeRed;
  static const XMVECTOR POINT_COLOR    = DirectX::Colors::White;
  static const XMVECTOR CONTROL_COLOR  = DirectX::Colors::Yellow;

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
    const auto& point   = waypoints[i].wayPoint;
    const auto& control = waypoints[i].controlPoint;

    DX::DrawCurve(batch, prevPoint, point, control);
    DX::DrawRing(
      batch,
      point,
      xaxis,
      yaxis,
      (selectedPointIdx == i) ? SELECTED_COLOR : POINT_COLOR);

    DX::DrawLine(batch, point, control, DirectX::Colors::Yellow);
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
    const auto& key   = child.first;
    const auto& value = child.second;

    if (value.is_string() && key == ID_NODE_KEY)
    {
      ret.id = strUtils::utf8ToWstring(value.string_value().c_str());
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
    const auto& key   = child.first;
    const auto& value = child.second;

    if (value.is_string() && key == PATH_ID_KEY)
    {
      ret.pathId = strUtils::utf8ToWstring(value.string_value().c_str());
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
  return json11::Json::object{{PATH_ID_KEY, strUtils::wstringToUtf8(pathId)},
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
    const auto& key   = child.first;
    const auto& value = child.second;

    if (value.is_string() && key == ID_NODE_KEY)
    {
      ret.id = strUtils::utf8ToWstring(value.string_value().c_str());
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
    const auto& key   = child.first;
    const auto& value = child.second;

    if (value.is_string() && key == FORMATION_ID_KEY)
    {
      ret.formationId = strUtils::utf8ToWstring(value.string_value().c_str());
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
    {FORMATION_ID_KEY, strUtils::wstringToUtf8(formationId)}};
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
    const auto& key   = child.first;
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

//------------------------------------------------------------------------------
struct Parser
{
  PathPool& paths;
  FormationPool& formations;
  LevelPool& levels;

  Parser(PathPool& paths, FormationPool& formations, LevelPool& levels)
      : paths(paths)
      , formations(formations)
      , levels(levels)
  {
  }

  void parse(const json11::Json& json);
  bool isParseError = false;

private:
  void parseRootJsonObject(const json11::Json& json);
  void parsePathsJsonObject(const json11::Json& json);
  void parseFormationsJsonObject(const json11::Json& json);
  void parseLevelsJsonObject(const json11::Json& json);
};

//------------------------------------------------------------------------------
void
Parser::parse(const json11::Json& json)
{
  TRACE
  if (!json.is_array())
  {
    LOG_ERROR("Can't parse Root Array - not a JSON array type");
    isParseError = true;
  }

  const auto& children = json.array_items();
  for (const auto& child : children)
  {
    parseRootJsonObject(child);
  }
}

//------------------------------------------------------------------------------
void
Parser::parseRootJsonObject(const json11::Json& json)
{
  TRACE
  if (!json.is_object())
  {
    LOG_ERROR("Can't parse Root Object - not a JSON object type");
    isParseError = true;
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
Parser::parsePathsJsonObject(const json11::Json& json)
{
  TRACE
  if (!json.is_array())
  {
    LOG_ERROR("Can't parse Paths - not a JSON array type");
    isParseError = true;
    return;
  }

  for (const auto& path : json.array_items())
  {
    paths.emplace_back(Path::from_json(path));
  }
}

//------------------------------------------------------------------------------
void
Parser::parseFormationsJsonObject(const json11::Json& json)
{
  TRACE
  if (!json.is_array())
  {
    LOG_ERROR("Can't parse Formations - not a JSON array type");
    isParseError = true;
    return;
  }

  for (const auto& formation : json.array_items())
  {
    formations.emplace_back(Formation::from_json(formation));
  }
}
//------------------------------------------------------------------------------
void
Parser::parseLevelsJsonObject(const json11::Json& json)
{
  TRACE
  if (!json.is_array())
  {
    LOG_ERROR("Can't parse Levels - not a JSON array type");
    isParseError = true;
    return;
  }

  for (const auto& level : json.array_items())
  {
    levels.emplace_back(Level::from_json(level));
  }
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool
LevelData::load(PathPool& paths, FormationPool& formations, LevelPool& levels)
{
  TRACE
  std::ifstream fileIn(LEVEL_DATA_FILENAME);
  if (!fileIn.is_open())
  {
    LOG_ERROR("Level file could not be found to load");
    return false;
  }

  std::stringstream ss;
  ss << fileIn.rdbuf();
  fileIn.close();

  std::string err;
  const auto& str   = ss.str();
  json11::Json json = json11::Json::parse(str, err);
  if (json.is_null())
  {
    LOG_ERROR(str.c_str());
    return false;
  }

  Parser parser(paths, formations, levels);
  parser.parse(json);

  populateIndicesPostLoad(paths, formations, levels);

  return (parser.isParseError == false);
}

//------------------------------------------------------------------------------
bool
LevelData::save(PathPool& paths, FormationPool& formations, LevelPool& levels)
{
  return save(
    paths.begin(),
    paths.end(),
    formations.begin(),
    formations.end(),
    levels.begin(),
    levels.end());
}

//------------------------------------------------------------------------------
bool
LevelData::save(
  PathPool::iterator pathsBegin,
  PathPool::iterator pathsEnd,
  FormationPool::iterator formationsBegin,
  FormationPool::iterator formationsEnd,
  LevelPool::iterator levelsBegin,
  LevelPool::iterator levelsEnd)
{
  TRACE
  std::ofstream fileOut(LEVEL_DATA_FILENAME);
  if (!fileOut.is_open())
  {
    LOG_ERROR("Level file could opened on save");
    return false;
  }
  auto paths      = json11::Json(pathsBegin, pathsEnd);
  auto formations = json11::Json(formationsBegin, formationsEnd);
  auto levels     = json11::Json(levelsBegin, levelsEnd);

  fileOut << json11::Json(
               json11::Json::array{
                 json11::Json::object{{PATHS_NODE_ID, paths}},
                 json11::Json::object{{FORMATIONS_NODE_ID, formations}},
                 json11::Json::object{{LEVELS_NODE_ID, levels}}})
               .dump();

  return true;
}

//------------------------------------------------------------------------------
void
LevelData::populateIdsPreSave(
  PathPool& paths, FormationPool& formations, LevelPool& levels)
{
  for (auto& formation : formations)
  {
    for (auto& sec : formation.sections)
    {
      ASSERT(sec.pathIdx < paths.size());
      sec.pathId = paths[sec.pathIdx].id;
    }
  }

  for (auto& level : levels)
  {
    for (auto& wave : level.waves)
    {
      ASSERT(wave.formationIdx < formations.size());
      wave.formationId = formations[wave.formationIdx].id;
    }
  }
}

//------------------------------------------------------------------------------
void
LevelData::populateIndicesPostLoad(
  PathPool& paths, FormationPool& formations, LevelPool& levels)
{
  for (auto& formation : formations)
  {
    for (auto& sec : formation.sections)
    {
      sec.pathIdx = 0;
      for (size_t i = 0; i < paths.size(); ++i)
      {
        if (paths[i].id == sec.pathId)
        {
          sec.pathIdx = i;
          continue;
        }
      }
    }
  }

  for (auto& level : levels)
  {
    for (auto& wave : level.waves)
    {
      wave.formationIdx = 0;
      for (size_t i = 0; i < formations.size(); ++i)
      {
        if (formations[i].id == wave.formationId)
        {
          wave.formationIdx = i;
          continue;
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
