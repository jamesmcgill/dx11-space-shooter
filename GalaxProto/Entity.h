#pragma once
#include "pch.h"

//------------------------------------------------------------------------------
struct ModelData
{
	std::unique_ptr<DirectX::Model> model;
	DirectX::BoundingSphere bound;
};

//------------------------------------------------------------------------------
// Entities Partitioned by type
static const size_t NUM_PLAYERS = 1;
static const size_t PLAYERS_IDX = 0;
static const size_t PLAYERS_END = PLAYERS_IDX + NUM_PLAYERS;

static const size_t NUM_PLAYER_SHOTS = 10;
static const size_t PLAYER_SHOTS_IDX = PLAYERS_END;
static const size_t PLAYER_SHOTS_END = PLAYER_SHOTS_IDX + NUM_PLAYER_SHOTS;

static const size_t NUM_ENEMY_SHOTS = 10;
static const size_t ENEMY_SHOTS_IDX = PLAYER_SHOTS_END;
static const size_t ENEMY_SHOTS_END = ENEMY_SHOTS_IDX + NUM_ENEMY_SHOTS;

static const size_t NUM_ENEMIES = 60;
static const size_t ENEMIES_IDX = ENEMY_SHOTS_END;
static const size_t ENEMIES_END = ENEMIES_IDX + NUM_ENEMIES;

static const size_t NUM_ENTITIES = ENEMIES_END;

static const size_t BALLISTIC_IDX = 0;
static const size_t BALLISTIC_END = ENEMIES_IDX;

//------------------------------------------------------------------------------
struct Entity
{
	DirectX::SimpleMath::Vector3 position = {};
	DirectX::SimpleMath::Vector3 velocity = {};
	float birthTimeS											= 0.0f;
	ModelData* model											= nullptr;
	bool isColliding											= false;
	bool isAlive													= false;
};

//------------------------------------------------------------------------------
