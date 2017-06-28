//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "ShowScoresState.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float STATE_TIMEOUT_SECONDS = 5.0f;
const XMVECTOR FONT_COLOR							= {1.0f, 1.0f, 0.0f};
constexpr wchar_t* GAMEOVER_TEXT			= L"ScoreBoard goes here";

//------------------------------------------------------------------------------
void
ShowScoresState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	auto& kb		= m_resources.kbTracker;

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
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	m_resources.starField->update(timer);
	m_timeoutS -= elapsedTimeS;
	if (m_timeoutS <= 0.0f) {
		m_states.changeState(m_states.previousState());
	}
}

//------------------------------------------------------------------------------
void
ShowScoresState::render()
{
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	Vector2 pos
		= {m_resources.m_screenWidth / 2.0f, m_resources.m_screenHeight / 2.0f};
	Vector2 origin = m_resources.m_font->MeasureString(GAMEOVER_TEXT) / 2.f;


	m_resources.scoreBoard->render(m_resources.m_font.get(), m_resources.m_spriteBatch.get());

	//m_resources.m_font->DrawString(
	//	m_resources.m_spriteBatch.get(),
	//	GAMEOVER_TEXT,
	//	pos,
	//	FONT_COLOR,
	//	0.f,
	//	origin);

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
ShowScoresState::load()
{
	// TRACE("ShowScoresState::load()");
}

//------------------------------------------------------------------------------
void
ShowScoresState::unload()
{
	// TRACE("ShowScoresState::unload()");
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
	// TRACE("ShowScoresState::enter()");
	m_timeoutS = STATE_TIMEOUT_SECONDS;
}

//------------------------------------------------------------------------------
void
ShowScoresState::exit()
{
	// TRACE("ShowScoresState::exit()");
}

//------------------------------------------------------------------------------
