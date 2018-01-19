#pragma once
#include "pch.h"
#include "Entity.h"
#include "UIDebugDraw.h"

//------------------------------------------------------------------------------
constexpr int INITIAL_NUM_PLAYER_LIVES = 5;

//------------------------------------------------------------------------------
enum class PlayerState
{
	Normal,
	Dying,
	Reviving,
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
	const float defaultCameraDistance = 80.0f;

	float screenWidth			 = 0.0f;
	float screenHeight		 = 0.0f;
	float screenHalfWidth	= 0.0f;
	float screenHalfHeight = 0.0f;

	DirectX::SimpleMath::Matrix worldToView;
	DirectX::SimpleMath::Matrix viewToProjection;
	DirectX::SimpleMath::Matrix pixelsToProjection;
	DirectX::SimpleMath::Matrix projectionToPixels;
	float cameraRotationX = 0.0f;
	float cameraRotationY = 0.0f;
	float cameraDistance	= defaultCameraDistance;

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
	ui::Text uiScore;
	ui::Text uiLives;

	ProfileViz profileViz		= ProfileViz::Basic;
	bool debugDraw					= false;
	bool isMidiConnected		= false;
	float playerSpeed				= 200.0f;
	float playerFriction		= 60.0f;
	float playerMaxVelocity = 40.0f;
	float playerMinVelocity = 0.3f;

	ui::Text uiDebugVarsTitle;
	ui::Text uiPlayerSpeed;
	ui::Text uiPlayerFriction;
	ui::Text uiPlayerMaxVelocity;
	ui::Text uiPlayerMinVelocity;
	ui::Text uiCameraDist;
	ui::Text uiControlInfo;

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

		worldToView = XMMatrixLookAtRH(eyePos, at, up);
	}
};

//------------------------------------------------------------------------------
