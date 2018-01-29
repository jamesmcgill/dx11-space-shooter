#include "pch.h"
#include "AppStates.h"
#include "GetReadyState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float STATE_TIMEOUT_SECONDS = 3.0f;
const XMVECTOR FONT_COLOR							= {1.0f, 1.0f, 0.0f};
constexpr wchar_t READY_TEXT[]				= L"Get Ready";

//------------------------------------------------------------------------------
void
GetReadyState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	const auto& kb = m_resources.kbTracker;
	if (kb.IsKeyPressed(DirectX::Keyboard::Escape))
	{
		m_states.changeState(&m_states.playing);
	}
}

//------------------------------------------------------------------------------
void
GetReadyState::update(const DX::StepTimer& timer)
{
	TRACE
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);
	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f)
	{
		m_states.changeState(&m_states.playing);
	}
}

//------------------------------------------------------------------------------
void
GetReadyState::render()
{
	TRACE
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	Vector2 pos		 = {m_context.screenHalfWidth, m_context.screenHalfHeight};
	Vector2 origin = m_resources.font32pt->MeasureString(READY_TEXT) / 2.f;

	m_resources.font32pt->DrawString(
		m_resources.m_spriteBatch.get(), READY_TEXT, pos, FONT_COLOR, 0.f, origin);

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
GetReadyState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
GetReadyState::unload()
{
	TRACE
}

//------------------------------------------------------------------------------
bool
GetReadyState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
GetReadyState::enter()
{
	TRACE
	m_timeoutS = STATE_TIMEOUT_SECONDS;
	m_resources.soundEffects[AudioResource::GameStart]->Play();
}

//------------------------------------------------------------------------------
void
GetReadyState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
