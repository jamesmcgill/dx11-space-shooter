#pragma once
#include "pch.h"
#include "Entity.h"

//------------------------------------------------------------------------------
enum class PlayerState
{
	Normal,
	Dying,
	Reviving,
};

constexpr int INITIAL_NUM_PLAYER_LIVES = 5;

//------------------------------------------------------------------------------
struct UIText {
	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 dimensions;
	DirectX::SimpleMath::Vector2 origin;
	DirectX::SpriteFont* font;
	std::wstring text;

	void draw(
		DirectX::XMVECTOR color,
		DirectX::SpriteBatch& spriteBatch)
	{
		assert(font);

		font->DrawString(
			&spriteBatch,
			text.c_str(),
			position,
			color,
			0.f,
			origin);
	}
};

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
	
	int playerScore																= 0;
	int playerLives																= INITIAL_NUM_PLAYER_LIVES;
	PlayerState playerState = PlayerState::Normal;
	float playerDeathTimerS = 0.0f;
	float playerReviveTimerS = 0.0f;
	UIText uiScore;
	UIText uiLives;

	bool debugDraw = false;
	float playerSpeed = 200.0f;
	float playerFriction = 60.0f;
	float playerMaxVelocity = 40.0f;
	float playerMinVelocity = 0.3f;
	float cameraDistance = 80.0f;
	UIText uiPlayerSpeed;
	UIText uiPlayerFriction;
	UIText uiPlayerMaxVelocity;
	UIText uiPlayerMinVelocity;
	UIText uiCameraDist;
};

//------------------------------------------------------------------------------
