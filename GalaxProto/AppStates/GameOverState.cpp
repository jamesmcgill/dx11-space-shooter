//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "GameOverState.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float STATE_TIMEOUT_SECONDS = 4.0f;
const XMVECTOR FONT_COLOR							= {1.0f, 1.0f, 0.0f};
constexpr wchar_t* GAMEOVER_TEXT			= L"Game Over";

//------------------------------------------------------------------------------
void
GameOverState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);
}

//------------------------------------------------------------------------------
void
GameOverState::update(const DX::StepTimer& timer)
{
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);

	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f) {
		if (m_resources.scoreBoard->isHiScore(m_context.playerScore)) {
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
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	Vector2 pos
		= {m_resources.m_screenWidth / 2.0f, m_resources.m_screenHeight / 2.0f};
	Vector2 origin = m_resources.m_font->MeasureString(GAMEOVER_TEXT) / 2.f;

	m_resources.m_font->DrawString(
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
	// TRACE("GameOverState::load()");
}

//------------------------------------------------------------------------------
void
GameOverState::unload()
{
	// TRACE("GameOverState::unload()");
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
	// TRACE("GameOverState::enter()");
	m_timeoutS = STATE_TIMEOUT_SECONDS;
	m_resources.soundEffects[AudioResource::GameOver]->Play();
}

//------------------------------------------------------------------------------
void
GameOverState::exit()
{
	// TRACE("GameOverState::exit()");
}

//------------------------------------------------------------------------------
