#include "pch.h"
#include "AppStates.h"
#include "GameOverState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float STATE_TIMEOUT_SECONDS = 4.0f;
const XMVECTOR FONT_COLOR							= {1.0f, 1.0f, 0.0f};
constexpr wchar_t GAMEOVER_TEXT[] = L"Game Over";
constexpr wchar_t FINALSCORE_TEXT[] = L"Final Score: {}";

//------------------------------------------------------------------------------
void
GameOverState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	const auto& kb = m_resources.kbTracker;
	if (kb.IsKeyPressed(DirectX::Keyboard::Escape))
	{
		m_states.changeState(&m_states.menu);
	}
}

//------------------------------------------------------------------------------
void
GameOverState::update(const DX::StepTimer& timer)
{
	TRACE
	const float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);

	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f)
	{
		if (m_resources.scoreBoard->isHiScore(m_context.playerScore))
		{
			m_states.changeState(&m_states.enteringScore);
		}
		else
		{
			m_states.changeState(&m_states.menu);
		}
	}
}

//------------------------------------------------------------------------------
void
GameOverState::render()
{
	TRACE
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	{
		auto& headingFont = m_resources.font32pt;
		auto finalScoreStr = fmt::format(FINALSCORE_TEXT, m_context.playerScore);
		Vector2 fontDimensions = headingFont->MeasureString(finalScoreStr.c_str());
		Vector2 origin = Vector2(fontDimensions.x / 2.f, 0.0f);
		Vector2 position = { m_context.screenHalfWidth, fontDimensions.y };

		headingFont->DrawString(
			m_resources.m_spriteBatch.get(),
			finalScoreStr.c_str(),
			position,
			FONT_COLOR,
			0.f,
			origin);
	}

	Vector2 pos		 = {m_context.screenHalfWidth, m_context.screenHalfHeight};
	Vector2 origin = m_resources.font32pt->MeasureString(GAMEOVER_TEXT) / 2.f;

	m_resources.font32pt->DrawString(
		m_resources.m_spriteBatch.get(),
		GAMEOVER_TEXT,
		pos,
		FONT_COLOR,
		0.f,
		origin);

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
GameOverState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
GameOverState::unload()
{
	TRACE
}

//------------------------------------------------------------------------------
bool
GameOverState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
GameOverState::enter()
{
	TRACE
	m_timeoutS = STATE_TIMEOUT_SECONDS;
	m_resources.soundEffects[AudioResource::GameOver]->Play();
}

//------------------------------------------------------------------------------
void
GameOverState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
