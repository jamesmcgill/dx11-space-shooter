#pragma once

#include "ResourceIDs.h"		// ModelResource
namespace json11
{
class Json;
};

//------------------------------------------------------------------------------
struct Waypoint
{
	DirectX::SimpleMath::Vector3 wayPoint			= {};
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
		size_t selectedPointIdx		= -1,
		size_t selectedControlIdx = -1) const;

	static Path from_json(const json11::Json& json);
	json11::Json to_json() const;
};

//------------------------------------------------------------------------------
struct FormationSection
{
	size_t pathIdx			= 0;
	int numShips				= 0;
	ModelResource model = ModelResource::Enemy1;

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

//------------------------------------------------------------------------------
struct Wave
{
	float spawnTimeS		= 0.0f;
	size_t formationIdx = 0;

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

//------------------------------------------------------------------------------
using PathPool			= std::vector<Path>;
using FormationPool = std::vector<Formation>;
using LevelPool			= std::vector<Level>;

//------------------------------------------------------------------------------
struct LevelData
{
	static bool
	load(PathPool& paths, FormationPool& formations, LevelPool& levels);

	static bool save(
		const PathPool& paths,
		const FormationPool& formations,
		const LevelPool& levels);

	static bool save(
		PathPool::const_iterator pathsBegin,
		PathPool::const_iterator pathsEnd,
		FormationPool::const_iterator formationsBegin,
		FormationPool::const_iterator formationsEnd,
		LevelPool::const_iterator levelsBegin,
		LevelPool::const_iterator levelsEnd);
};

//------------------------------------------------------------------------------
