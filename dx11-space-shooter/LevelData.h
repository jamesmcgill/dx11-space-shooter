#pragma once

#include "ResourceIDs.h"    // ModelResource
namespace json11
{
class Json;
};

//------------------------------------------------------------------------------
struct Waypoint
{
  DirectX::SimpleMath::Vector3 wayPoint     = {};
  DirectX::SimpleMath::Vector3 controlPoint = {};

  static Waypoint from_json(const json11::Json& json);
  json11::Json to_json() const;
};

//------------------------------------------------------------------------------
struct Path
{
  std::wstring id;
  std::vector<Waypoint> waypoints;

  void debugRender(
    DX::DebugBatchType* batch,
    size_t selectedPointIdx   = -1,
    size_t selectedControlIdx = -1) const;

  static Path from_json(const json11::Json& json);
  json11::Json to_json() const;
};
using PathPool = std::vector<Path>;

//------------------------------------------------------------------------------
struct FormationSection
{
  size_t pathIdx      = 0;
  int numShips        = 0;
  ModelResource model = ModelResource::Enemy1;
  std::wstring pathId;

  static FormationSection from_json(const json11::Json& json);
  json11::Json to_json() const;
};

//------------------------------------------------------------------------------
struct Formation
{
  std::wstring id;
  std::vector<FormationSection> sections;

  static Formation from_json(const json11::Json& json);
  json11::Json to_json() const;
};
using FormationPool = std::vector<Formation>;

//------------------------------------------------------------------------------
struct Wave
{
  float spawnTimeS    = 0.0f;
  size_t formationIdx = 0;
  std::wstring formationId;

  static Wave from_json(const json11::Json& json);
  json11::Json to_json() const;
};

//------------------------------------------------------------------------------
struct Level
{
  std::vector<Wave> waves;

  static Level from_json(const json11::Json& json);
  json11::Json to_json() const;
};
using LevelPool = std::vector<Level>;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct LevelData
{
  // Relational indices will be set during load.
  // Therefore DO NOT modify the order of the lists after calling Load()
  // without adjusting the index references.
  // Inserting Dummy Data before calling will generate the indices correctly.
  static bool
  load(PathPool& paths, FormationPool& formations, LevelPool& levels);

  static bool
  save(PathPool& paths, FormationPool& formations, LevelPool& levels);

  static bool save(
    PathPool::iterator pathsBegin,
    PathPool::iterator pathsEnd,
    FormationPool::iterator formationsBegin,
    FormationPool::iterator formationsEnd,
    LevelPool::iterator levelsBegin,
    LevelPool::iterator levelsEnd);

  // This must be called on the full runtime data (including any Dummy Data)
  // to ensure the id's retrieved from the correct index.
  static void populateIdsPreSave(
    PathPool& paths, FormationPool& formations, LevelPool& levels);

private:
  static void populateIndicesPostLoad(
    PathPool& paths, FormationPool& formations, LevelPool& levels);
};

//------------------------------------------------------------------------------
