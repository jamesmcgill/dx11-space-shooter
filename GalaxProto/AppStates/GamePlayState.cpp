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
constexpr float CAMERA_SPEED_X = 1.0f;
constexpr float CAMERA_SPEED_Y = 1.0f;
constexpr float UNIT_DIAGONAL_LENGTH = 0.7071067811865475f;

//------------------------------------------------------------------------------
void
GamePlayState::handleInput(const DX::StepTimer& timer)
{
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	auto& kbState = m_resources.kbTracker.lastState;

	if (kbState.Escape) {
		m_states.changeState(m_states.menu.get());
		//ExitGame();
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
		m_gameLogic.gameMaster.emitPlayerShot();
	}
}

//------------------------------------------------------------------------------
void
GamePlayState::update(const DX::StepTimer& timer)
{
	m_resources.starField->update(timer);
	m_gameLogic.tick(timer);
}

//------------------------------------------------------------------------------
void
GamePlayState::render()
{
	auto dc = m_resources.m_deviceResources->GetD3DDeviceContext();

	// STARS
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);
	m_resources.m_spriteBatch->End();

	// GAME ELEMENTS
	for (auto& entity : m_context.entities)
	{
		if (entity.isAlive) {
			m_gameLogic.renderEntity(entity);
		}
	}

	// Debug Drawing
	dc->OMSetBlendState(
		m_resources.m_states->Opaque(), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(m_resources.m_states->DepthNone(), 0);
	dc->RSSetState(m_resources.m_states->CullNone());

	m_resources.m_debugEffect->SetView(m_context.view);
	m_resources.m_debugEffect->SetProjection(m_context.proj);
	m_resources.m_debugEffect->Apply(dc);
	dc->IASetInputLayout(m_resources.m_debugInputLayout.Get());

	m_resources.m_batch->Begin();
	for (auto& entity : m_context.entities)
	{
		if (entity.isAlive) {
			m_gameLogic.renderEntityBound(entity);
		}
	}
	m_gameLogic.gameMaster.debugRender(m_resources.m_batch.get());
	m_resources.m_batch->End();

	// HUD and Menus
	m_resources.m_spriteBatch->Begin();
	m_gameLogic.drawHUD();
	// m_resources.menuManager->render(
	//	m_resources.m_font.get(), m_resources.m_spriteBatch.get());
	m_resources.m_spriteBatch->End();
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
}

//------------------------------------------------------------------------------
void
GamePlayState::exit()
{
//	TRACE("GamePlayState::exit()");
}

//------------------------------------------------------------------------------
