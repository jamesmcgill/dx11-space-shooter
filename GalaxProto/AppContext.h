#pragma once
#include "Entity.h"

//------------------------------------------------------------------------------
struct AppContext
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
	float cameraRotationX = 0.0f;
	float cameraRotationY = 0.0f;
	float cameraDist			= 1.0f;

	size_t nextPlayerShotIdx = PLAYER_SHOTS_IDX;
	size_t nextEnemyShotIdx	= ENEMY_SHOTS_IDX;
	size_t nextEnemyIdx			 = ENEMIES_IDX;
	Entity entities[NUM_ENTITIES];

	DirectX::SimpleMath::Vector3 playerAccel = {};

	DirectX::SimpleMath::Vector2 hudScorePosition = {};
	DirectX::SimpleMath::Vector2 hudLivesPosition = {};
	int playerScore																= 0;
	int playerLives																= 5;
};

//------------------------------------------------------------------------------
