//#define ENABLE_TRACE
//#define ENABLE_LOCAL

#include "pch.h"
#include "AppStates.h"
#include "AppContext.h"
#include "ScoreEntryState.h"
#include "AppResources.h"
#include "GameLogic.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
const XMVECTOR FONT_COLOR																= {1.0f, 1.0f, 0.0f};
static const std::vector<std::wstring> ENTER_TEXT_LINES = {
	L"Congratulations!", L"You have a new hi-score.", L"Please Enter Your Name:"};
static const std::vector<std::wstring> INSTRUCTION_LINES
	= {L"Maximum 3 Initials",
		 L"Up/Down Arrows: Change letter",
		 L"Backspace: Remove last letter",
		 L"Enter: Select letter"};

constexpr size_t MAX_NAME_LENGTH = 3;

//------------------------------------------------------------------------------
void
ScoreEntryState::handleInput(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	auto& kb = m_resources.kbTracker;

	if (kb.IsKeyPressed(Keyboard::Enter)) {
		m_playerName.push_back(m_currentChar);

		if (m_playerName.size() >= MAX_NAME_LENGTH) {
			m_resources.scoreBoard->insertScore(
				{m_context.playerScore, m_playerName});
			m_resources.scoreBoard->saveToFile();
			m_states.changeState(&m_states.menu);
		}
	}

	if (kb.IsKeyPressed(Keyboard::Up)) {
		if (m_currentChar == 'A') {
			m_currentChar = 'Z';
		}
		else
		{
			m_currentChar--;
		}
	}

	if (kb.IsKeyPressed(Keyboard::Down)) {
		if (m_currentChar == 'Z') {
			m_currentChar = 'A';
		}
		else
		{
			m_currentChar++;
		}
	}

	if (kb.IsKeyPressed(Keyboard::Back) || kb.IsKeyPressed(Keyboard::Escape)) {
		if (!m_playerName.empty()) {
			m_currentChar = m_playerName.back();
			m_playerName.pop_back();
		}
	}
}

//------------------------------------------------------------------------------
void
ScoreEntryState::update(const DX::StepTimer& timer)
{
	m_resources.starField->update(timer);
}

//------------------------------------------------------------------------------
void
ScoreEntryState::render()
{
	m_resources.m_spriteBatch->Begin();
	m_resources.starField->render(*m_resources.m_spriteBatch);

	Vector2 fontDimensions = m_resources.font32pt->MeasureString(L"XXX");
	Vector2 centerPos
		= {m_resources.m_screenWidth / 2.0f, (m_resources.m_screenHeight / 2.0f)};

	Vector2 linePos
		= centerPos - Vector2(0.0f, (ENTER_TEXT_LINES.size() * fontDimensions.y));
	for (auto& line : ENTER_TEXT_LINES)
	{
		Vector2 origin = m_resources.font32pt->MeasureString(line.c_str()) / 2.f;
		m_resources.font32pt->DrawString(
			m_resources.m_spriteBatch.get(),
			line.c_str(),
			linePos,
			FONT_COLOR,
			0.f,
			origin);
		linePos.y += fontDimensions.y;
	}

	{
		Vector2 origin		 = fontDimensions / 2.f;
		auto displayedName = (m_playerName.size() >= MAX_NAME_LENGTH)
													 ? m_playerName
													 : m_playerName + m_currentChar;

		m_resources.font32pt->DrawString(
			m_resources.m_spriteBatch.get(),
			displayedName.c_str(),
			linePos,
			FONT_COLOR,
			0.f,
			origin);
	}

	linePos = Vector2(
		0.0f,
		m_resources.m_screenHeight
			- ((INSTRUCTION_LINES.size() - 1) * fontDimensions.y));
	for (auto& line : INSTRUCTION_LINES)
	{
		Vector2 origin = {0.0f, fontDimensions.y};
		m_resources.font32pt->DrawString(
			m_resources.m_spriteBatch.get(),
			line.c_str(),
			linePos,
			FONT_COLOR,
			0.f,
			origin);
		linePos.y += fontDimensions.y;
	}

	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
ScoreEntryState::load()
{
	// TRACE("ScoreEntryState::load()");
}

//------------------------------------------------------------------------------
void
ScoreEntryState::unload()
{
	// TRACE("ScoreEntryState::unload()");
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
	// TRACE("ScoreEntryState::enter()");
	m_playerName.clear();
	m_currentChar = L'A';
}

//------------------------------------------------------------------------------
void
ScoreEntryState::exit()
{
	// TRACE("ScoreEntryState::exit()");
}

//------------------------------------------------------------------------------
