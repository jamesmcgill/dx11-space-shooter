#include "pch.h"
#include "AppStates.h"
#include "AppContext.h"
#include "ScoreEntryState.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
const XMVECTOR FONT_COLOR = {1.0f, 1.0f, 0.0f};

static const std::wstring FINALSCORE_TEXT = L"Final Score: {}";
static const std::wstring HEADING_TEXT		= L"Congratulations!";
static const std::vector<std::wstring> ENTER_TEXT_LINES
	= {L"You have a new hi-score.", L"Please Enter Your Name:"};

//------------------------------------------------------------------------------
void
ScoreEntryState::handleInput(const DX::StepTimer& timer)
{
	const auto& kb = m_resources.kbTracker;
	if (
		kb.IsKeyPressed(DirectX::Keyboard::Enter)
		&& !m_playerName.getRawText().empty())
	{
		m_resources.scoreBoard->insertScore(
			{m_context.playerScore, m_playerName.getRawText()});

		m_resources.scoreBoard->saveToFile();
		m_states.changeState(&m_states.menu);
		return;
	}

	m_playerName.handleInput(timer);
}

//------------------------------------------------------------------------------
void
ScoreEntryState::update(const DX::StepTimer& timer)
{
	TRACE
	m_resources.starField->update(timer);
}

//------------------------------------------------------------------------------
void
ScoreEntryState::render()
{
	TRACE
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	auto& headingFont			 = m_resources.font32pt;
	auto finalScoreStr		 = fmt::format(FINALSCORE_TEXT, m_context.playerScore);
	Vector2 fontDimensions = headingFont->MeasureString(finalScoreStr.c_str());
	Vector2 origin				 = Vector2(fontDimensions.x / 2.f, 0.0f);

	Vector2 position = {m_context.screenHalfWidth, fontDimensions.y};

	headingFont->DrawString(
		m_resources.m_spriteBatch.get(),
		finalScoreStr.c_str(),
		position,
		FONT_COLOR,
		0.f,
		origin);
	position.y += 2 * fontDimensions.y;

	fontDimensions = headingFont->MeasureString(HEADING_TEXT.c_str());
	origin				 = Vector2(fontDimensions.x / 2.f, 0.0f);
	headingFont->DrawString(
		m_resources.m_spriteBatch.get(),
		HEADING_TEXT.c_str(),
		position,
		FONT_COLOR,
		0.f,
		origin);
	position.y += fontDimensions.y;

	auto& mainFont = m_resources.font16pt;
	fontDimensions = mainFont->MeasureString(L"XXX");
	position.y += fontDimensions.y / 2.0f;

	for (auto& line : ENTER_TEXT_LINES)
	{
		fontDimensions = mainFont->MeasureString(line.c_str());
		origin				 = fontDimensions / 2.f;

		mainFont->DrawString(
			m_resources.m_spriteBatch.get(),
			line.c_str(),
			position,
			FONT_COLOR,
			0.f,
			origin);
		position.y += fontDimensions.y;
	}

	auto& nameFont		= m_resources.font32pt;
	Vector2 centerPos = {m_context.screenHalfWidth, m_context.screenHalfHeight};
	fontDimensions
		= nameFont->MeasureString(m_playerName.getDisplayText().c_str());
	origin = fontDimensions / 2.f;

	nameFont->DrawString(
		m_resources.m_spriteBatch.get(),
		m_playerName.getDisplayText().c_str(),
		centerPos,
		FONT_COLOR,
		0.f,
		origin);

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
ScoreEntryState::load()
{
	TRACE
}

//------------------------------------------------------------------------------
void
ScoreEntryState::unload()
{
	TRACE
}

//------------------------------------------------------------------------------
bool
ScoreEntryState::isLoaded() const
{
	return false;
}

//------------------------------------------------------------------------------
void
ScoreEntryState::enter()
{
	TRACE
	m_playerName.setRawText(L"");
}

//------------------------------------------------------------------------------
void
ScoreEntryState::exit()
{
	TRACE
}

//------------------------------------------------------------------------------
