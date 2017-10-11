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
struct UIText
{
	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 origin;
	DirectX::SpriteFont* font = nullptr;
	std::wstring text;

	void draw(DirectX::XMVECTOR color, DirectX::SpriteBatch& spriteBatch)
	{
		assert(font);

		font->DrawString(&spriteBatch, text.c_str(), position, color, 0.f, origin);
	}
};

//------------------------------------------------------------------------------
enum class ProfileViz
{
	Disabled,
	Basic,
	List,
	FlameGraph
};

//------------------------------------------------------------------------------
struct AppContext
{
	float screenWidth			 = 0.0f;
	float screenHeight		 = 0.0f;
	float screenHalfWidth	= 0.0f;
	float screenHalfHeight = 0.0f;

	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
	DirectX::SimpleMath::Matrix orthoProj;
	float cameraRotationX = 0.0f;
	float cameraRotationY = 0.0f;
	float cameraDistance	= 80.0f;

	size_t nextPlayerShotIdx = PLAYER_SHOTS_IDX;
	size_t nextEnemyShotIdx	= ENEMY_SHOTS_IDX;
	size_t nextEnemyIdx			 = ENEMIES_IDX;
	Entity entities[NUM_ENTITIES];

	DirectX::SimpleMath::Vector3 playerAccel = {};

	int playerScore					 = 0;
	int playerLives					 = INITIAL_NUM_PLAYER_LIVES;
	PlayerState playerState	= PlayerState::Normal;
	float playerDeathTimerS	= 0.0f;
	float playerReviveTimerS = 0.0f;
	UIText uiScore;
	UIText uiLives;

	ProfileViz profileViz		= ProfileViz::Basic;
	bool debugDraw					= false;
	bool isMidiConnected		= false;
	float playerSpeed				= 200.0f;
	float playerFriction		= 60.0f;
	float playerMaxVelocity = 40.0f;
	float playerMinVelocity = 0.3f;

	UIText uiDebugVarsTitle;
	UIText uiPlayerSpeed;
	UIText uiPlayerFriction;
	UIText uiPlayerMaxVelocity;
	UIText uiPlayerMinVelocity;
	UIText uiCameraDist;
	UIText uiControlInfo;

	void updateViewMatrix()
	{
		using namespace DirectX;

		const auto& atP = DirectX::SimpleMath::Vector3{0.0f, 0.0f, 0.0f};
		const DirectX::XMVECTORF32 eye
			= {atP.x, atP.y, atP.z + cameraDistance, 0.0f};
		const DirectX::XMVECTORF32 at = {atP.x, atP.y, atP.z, 0.0f};
		const DirectX::XMVECTORF32 up = {0.0f, 1.0f, 0.0f, 0.0f};
		// view																 = XMMatrixLookAtRH(eye, at, up);

		// Camera controls
		XMVECTOR eyePos = XMVectorSubtract(eye, at);

		const float radiansX = static_cast<float>(fmod(cameraRotationX, XM_2PI));
		eyePos							 = XMVector3Rotate(
			eyePos, XMQuaternionRotationMatrix(XMMatrixRotationX(radiansX)));

		const float radiansY = static_cast<float>(fmod(cameraRotationY, XM_2PI));
		eyePos							 = XMVector3Rotate(
			eyePos, XMQuaternionRotationMatrix(XMMatrixRotationY(radiansY)));

		eyePos = XMVectorAdd(eyePos, at);

		view = XMMatrixLookAtRH(eyePos, at, up);
	}
};

//------------------------------------------------------------------------------
