//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "GamePlayState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
constexpr float CAMERA_SPEED_X			 = 1.0f;
constexpr float CAMERA_SPEED_Y			 = 1.0f;
constexpr float UNIT_DIAGONAL_LENGTH = 0.7071067811865475f;

//------------------------------------------------------------------------------
void
GamePlayState::handleInput(const DX::StepTimer& timer)
{
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	auto& kbState = m_resources.kbTracker.lastState;

	if (kbState.Escape) {
		m_states.changeState(&m_states.paused);
	}

	if (kbState.W) {
		m_context.cameraRotationX -= elapsedTimeS * CAMERA_SPEED_X;
	}
	else if (kbState.S)
	{
		m_context.cameraRotationX += elapsedTimeS * CAMERA_SPEED_X;
	}

	if (kbState.A) {
		m_context.cameraRotationY -= elapsedTimeS * CAMERA_SPEED_Y;
	}
	else if (kbState.D)
	{
		m_context.cameraRotationY += elapsedTimeS * CAMERA_SPEED_Y;
	}

	m_context.playerAccel = Vector3();
	if (kbState.Up) {
		m_context.playerAccel.y = 1.0f;
	}
	else if (kbState.Down)
	{
		m_context.playerAccel.y = -1.0f;
	}

	if (kbState.Left) {
		m_context.playerAccel.x = -1.0f;
	}
	else if (kbState.Right)
	{
		m_context.playerAccel.x = 1.0f;
	}

	if (m_context.playerAccel.x != 0.0f && m_context.playerAccel.y != 0.0f) {
		m_context.playerAccel *= UNIT_DIAGONAL_LENGTH;
	}

	if (
		m_resources.kbTracker.IsKeyPressed(Keyboard::LeftControl)
		|| m_resources.kbTracker.IsKeyPressed(Keyboard::Space))
	{
		m_gameLogic.m_gameMaster.emitPlayerShot();
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::update(const DX::StepTimer& timer)
{
	m_resources.starField->update(timer);
	m_gameLogic.update(timer);
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

	spriteBatch->Begin();
	m_gameLogic.drawHUD();
	spriteBatch->End();
}

//------------------------------------------------------------------------------
void
GamePlayState::load()
{
	//	TRACE("GamePlayState::load()");
}

//------------------------------------------------------------------------------
void
GamePlayState::unload()
{
	//	TRACE("GamePlayState::unload()");
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
	//	TRACE("GamePlayState::enter()");
	if (m_states.previousState() != &m_states.paused) {
		m_gameLogic.reset();
		m_resources.m_timer.ResetTotalTimer();
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::exit()
{
	//	TRACE("GamePlayState::exit()");
}

//------------------------------------------------------------------------------
