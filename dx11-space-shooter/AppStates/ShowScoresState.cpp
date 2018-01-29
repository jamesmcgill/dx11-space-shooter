#include "pch.h"
#include "AppStates.h"
#include "ShowScoresState.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float STATE_TIMEOUT_SECONDS = 5.0f;

//------------------------------------------------------------------------------
void
ShowScoresState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	TRACE
	auto& kb = m_resources.kbTracker;

	if (
		kb.IsKeyPressed(Keyboard::Escape) || kb.IsKeyPressed(Keyboard::LeftControl)
		|| kb.IsKeyPressed(Keyboard::Space) || kb.IsKeyPressed(Keyboard::Enter))
	{
		m_states.changeState(m_states.previousState());
	}
}

//------------------------------------------------------------------------------
void
ShowScoresState::update(const DX::StepTimer& timer)
{
	TRACE
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);
	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f)
	{
		m_states.changeState(m_states.previousState());
	}
}

//------------------------------------------------------------------------------
void
ShowScoresState::render()
{
	TRACE
	m_resources.m_spriteBatch->Begin();

	m_resources.starField->render(*m_resources.m_spriteBatch);
	m_resources.scoreBoard->render(*m_resources.m_spriteBatch);

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
ShowScoresState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
ShowScoresState::unload()
{
	TRACE
}

//------------------------------------------------------------------------------
bool
ShowScoresState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
ShowScoresState::enter()
{
	TRACE
	m_timeoutS = STATE_TIMEOUT_SECONDS;
}

//------------------------------------------------------------------------------
void
ShowScoresState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
