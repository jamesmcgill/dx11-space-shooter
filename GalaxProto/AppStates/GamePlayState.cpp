#include "pch.h"
#include "AppStates.h"
#include "GamePlayState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

#define ENABLE_TRACE
#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
constexpr float CAMERA_SPEED_X							 = 1.0f;
constexpr float CAMERA_SPEED_Y							 = 1.0f;
constexpr float CAMERA_MIN_DIST							 = 30.0f;
constexpr float UNIT_DIAGONAL_LENGTH				 = 0.7071067811865475f;

constexpr size_t CAMERA_DIST_CONTROL				 = 16;
constexpr size_t PLAYER_SPEED_CONTROL				 = 17;
constexpr size_t PLAYER_FRICTION_CONTROL		 = 18;
constexpr size_t PLAYER_MAX_VELOCITY_CONTROL = 19;
constexpr size_t PLAYER_MIN_VELOCITY_CONTROL = 20;

//------------------------------------------------------------------------------
void
GamePlayState::handleInput(const DX::StepTimer& timer)
{
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	auto& kbState = m_resources.kbTracker.lastState;

	if (kbState.Escape)
	{
		m_states.changeState(&m_states.paused);
	}

	// Debug options
	if (m_resources.kbTracker.IsKeyPressed(Keyboard::E))
	{
		auto pos = m_context.entities[PLAYERS_IDX].position
							 + m_context.entities[PLAYERS_IDX].model->bound.Center;
		m_resources.explosions->emit(pos, Vector3());
	}

	if (m_resources.kbTracker.IsKeyPressed(Keyboard::F2))
	{
		m_gameLogic.m_enemies.debugLevel();
		m_gameLogic.reset();
	}

	const auto& midiMask	= m_resources.midiTracker.dirtyMask;
	const auto& midiState = m_resources.midiTracker.currentState;
	if (midiMask.test(CAMERA_DIST_CONTROL))
	{
		m_context.cameraDistance
			= static_cast<float>(midiState[CAMERA_DIST_CONTROL]) + CAMERA_MIN_DIST;
		m_context.updateViewMatrix();
		m_gameLogic.updateUIDebugVariables();
	}
	if (midiMask.test(PLAYER_SPEED_CONTROL))
	{
		m_context.playerSpeed
			= static_cast<float>(midiState[PLAYER_SPEED_CONTROL] * 2);
		m_gameLogic.updateUIDebugVariables();
	}
	if (midiMask.test(PLAYER_FRICTION_CONTROL))
	{
		m_context.playerFriction
			= static_cast<float>(midiState[PLAYER_FRICTION_CONTROL] * 2);
		m_gameLogic.updateUIDebugVariables();
	}
	if (midiMask.test(PLAYER_MAX_VELOCITY_CONTROL))
	{
		m_context.playerMaxVelocity
			= static_cast<float>(midiState[PLAYER_MAX_VELOCITY_CONTROL] * 2);
		m_gameLogic.updateUIDebugVariables();
	}
	if (midiMask.test(PLAYER_MIN_VELOCITY_CONTROL))
	{
		m_context.playerMinVelocity
			= static_cast<float>(midiState[PLAYER_MIN_VELOCITY_CONTROL]) / 10.f;
		m_gameLogic.updateUIDebugVariables();
	}
	m_resources.midiTracker.flush();

	if (kbState.W)
	{
		m_context.cameraRotationX -= elapsedTimeS * CAMERA_SPEED_X;
		m_context.updateViewMatrix();
	}
	else if (kbState.S)
	{
		m_context.cameraRotationX += elapsedTimeS * CAMERA_SPEED_X;
		m_context.updateViewMatrix();
	}

	if (kbState.A)
	{
		m_context.cameraRotationY -= elapsedTimeS * CAMERA_SPEED_Y;
		m_context.updateViewMatrix();
	}
	else if (kbState.D)
	{
		m_context.cameraRotationY += elapsedTimeS * CAMERA_SPEED_Y;
		m_context.updateViewMatrix();
	}

	// Player Movement
	m_context.playerAccel = Vector3();		// NB. Must be reset, even while dead.
	if (m_context.playerState == PlayerState::Dying)
	{
		return;
	}

	if (kbState.Up)
	{
		m_context.playerAccel.y = 1.0f;
	}
	else if (kbState.Down)
	{
		m_context.playerAccel.y = -1.0f;
	}

	if (kbState.Left)
	{
		m_context.playerAccel.x = -1.0f;
	}
	else if (kbState.Right)
	{
		m_context.playerAccel.x = 1.0f;
	}

	if (m_context.playerAccel.x != 0.0f && m_context.playerAccel.y != 0.0f)
	{
		m_context.playerAccel *= UNIT_DIAGONAL_LENGTH;
	}

	if (
		m_resources.kbTracker.IsKeyPressed(Keyboard::LeftControl)
		|| m_resources.kbTracker.IsKeyPressed(Keyboard::Space))
	{
		m_gameLogic.m_enemies.emitPlayerShot();
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::update(const DX::StepTimer& timer)
{
	m_resources.starField->update(timer);
	m_resources.explosions->update(timer);
	if (GameLogic::GameStatus::GameOver == m_gameLogic.update(timer))
	{
		m_states.changeState(&m_states.gameOver);
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::render()
{
	auto& spriteBatch = m_resources.m_spriteBatch;

	spriteBatch->Begin();
	m_resources.starField->render(*spriteBatch);
	spriteBatch->End();

	m_gameLogic.render();
}

//------------------------------------------------------------------------------
void
GamePlayState::load()
{
	TRACE();
}

//------------------------------------------------------------------------------
void
GamePlayState::unload()
{
	TRACE();
}

//------------------------------------------------------------------------------
bool
GamePlayState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
GamePlayState::enter()
{
	TRACE();
	if (m_states.previousState() != &m_states.paused)
	{
		m_gameLogic.reset();
		m_resources.m_timer.ResetTotalTimer();
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::exit()
{
	TRACE();
}

//------------------------------------------------------------------------------
