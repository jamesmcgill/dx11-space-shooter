#include "pch.h"
#include "ScoreBoard.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "fmt/format.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const wchar_t* SCORE_FILENAME = L"score.dat";
static const wchar_t* ENTER_NAME_MESSAGE
	= L"Congratulations!\nYou have a new hi-score.\nPlease Enter Your Name:\n";

static const int PLAYER_NAME_LENGTH		= 4;
static const size_t NUM_SCORES				= 10;
static const XMVECTOR HIGHLIGHT_COLOR = {1.0f, 1.0f, 0.0f};
static const XMVECTOR NORMAL_COLOR		= {1.0f, 1.0f, 1.0f};
static const int FILE_VERSION					= 100;
static const wchar_t* DEFAULT_NAME		= L"Jim";

//------------------------------------------------------------------------------
void
ScoreBoard::PrevMenu()
{
	if (!m_isEntryModeOn)
	{
		return;
	}

	/*if (m_unCurrentCharIdx > 0) m_unCurrentCharIdx--;*/
}

//------------------------------------------------------------------------------
void
ScoreBoard::NextItem()
{
	if (!m_isEntryModeOn)
	{
		return;
	}

	// if (m_ucCurrentChar == 'A')
	//	m_ucCurrentChar = 'Z';
	// else
	//	m_ucCurrentChar--;
}

//------------------------------------------------------------------------------
void
ScoreBoard::PrevItem()
{
	if (!m_isEntryModeOn)
	{
		return;
	}

	// if (m_ucCurrentChar == 'Z')
	//	m_ucCurrentChar = 'A';
	// else
	//	m_ucCurrentChar++;
}

//------------------------------------------------------------------------------
void
ScoreBoard::SelectItem()
{
	if (!m_isEntryModeOn)
	{
		return;
	}

	// if (m_unCurrentCharIdx < PLAYERNAME_MAXLENGTH - 1) {
	//	m_ucPlayerName[m_unCurrentCharIdx] = m_ucCurrentChar;
	//	m_unCurrentCharIdx++;
	//}

	// if (m_unCurrentCharIdx >= PLAYERNAME_MAXLENGTH - 1) {
	//	insertScore(g_pApp->GetCurrentPlayerScore(), m_ucPlayerName);
	//	m_isEntryModeOn = false;

	//	m_ucCurrentChar		 = 'A';
	//	m_unCurrentCharIdx = 0;
	//}
}

//------------------------------------------------------------------------------
bool
ScoreBoard::isHiScore(const int& nScore) const
{
	return (nScore >= m_scores.back().score);
}

//------------------------------------------------------------------------------
void
ScoreBoard::insertScore(Score newScore)
{
	auto it = std::lower_bound(
		m_scores.begin(), m_scores.end(), newScore.score, [](Score& s, int val) {
			return s.score > val;
		});

	if (it != m_scores.end())
	{
		m_scores.insert(it, newScore);
		m_scores.pop_back();
	}
}

//------------------------------------------------------------------------------
void
ScoreBoard::loadFromFile()
{
	m_scores.clear();
	std::wifstream file(SCORE_FILENAME, std::ios::in);
	if (!file.is_open())
	{
		// TODO: Logging and error handling
		loadDefaultScores();
		return;
	}

	std::wstring versionStr;
	if (!getline(file, versionStr))
	{
		// TODO: Logging and error handling
		throw;
	}
	int version = std::stoi(versionStr);
	if (version != FILE_VERSION)
	{
		// TODO: Logging and error handling
		throw;
	}

	std::wstring scoreStr;
	std::wstring nameStr;
	while (getline(file, scoreStr))
	{
		if (!getline(file, nameStr))
		{
			// TODO: Logging and error handling
			throw;
		}

		int value = std::stoi(scoreStr);
		Score score{value, nameStr};
		m_scores.push_back(score);
	}
}

//------------------------------------------------------------------------------
void
ScoreBoard::saveToFile()
{
	std::wofstream file(SCORE_FILENAME, std::ios::out);
	if (!file.is_open())
	{
		// TODO: Logging and error handling
		throw;
	}

	file << FILE_VERSION << '\n';
	for (auto& e : m_scores)
	{
		file << e.score << '\n';
		file << e.playerName << '\n';
	}
}

//------------------------------------------------------------------------------
void
ScoreBoard::loadDefaultScores()
{
	m_scores.clear();
	int score = 10000;
	for (size_t i = 0; i < NUM_SCORES; ++i)
	{
		m_scores.emplace_back(Score{score, DEFAULT_NAME});
		score -= 1000;
	}
}

//------------------------------------------------------------------------------
void
ScoreBoard::render(DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch)
{
	// Layout
	//
	//       ####
	// ####  ----
	// ----  ####
	// ####  ---- X
	// ----  ####
	// ####  ----
	//       ####
	float fontHeight = XMVectorGetY(font->MeasureString(L"XXX"));
	float padding		 = fontHeight * 0.3f;
	Vector2 pos			 = {m_screenWidth / 2.0f, 0.0f};

	float numRowsAboveCenter = (m_scores.size() % 2 == 0)
															 ? m_scores.size() / 2.0f
															 : ((m_scores.size() + 1) / 2.0f) - 0.5f;
	float numPaddingRowsAbove = numRowsAboveCenter - 0.5f;

	// Screen center - text rows - padding rows
	pos.y = (m_screenHeight / 2) - (numRowsAboveCenter * fontHeight)
					- (numPaddingRowsAbove * padding);

	// Render Scoreboard
	for (auto& e : m_scores)
	{
		XMVECTOR dimensions = font->MeasureString(e.playerName.c_str());
		Vector2 nameOrigin	= {(XMVectorGetX(dimensions)), 0.0f};
		Vector2 scoreOrigin = {0.0f, nameOrigin.y};

		font->DrawString(
			spriteBatch, e.playerName.c_str(), pos, NORMAL_COLOR, 0.f, nameOrigin);

		fmt::WMemoryWriter w;
		w << e.score;

		font->DrawString(
			spriteBatch, w.c_str(), pos, NORMAL_COLOR, 0.f, scoreOrigin);

		pos.y += fontHeight + padding;
	}
}

//------------------------------------------------------------------------------
// HRESULT
// ScoreBoard::DrawNewScoreEntryScreen(LPDIRECT3DDEVICE9 pd3dDevice)
//{
//	RECT main_rect;
//	RECT entry_rect;
//
//	SetRect(&main_rect, 0, 0, m_unScreenWidth, m_unScreenHeight * 0.75f);
//	SetRect(&entry_rect, 0, 0, m_unScreenWidth, m_unScreenHeight);
//
//	m_pLargeFont->DrawText(
//		NULL,																	 // pSprite
//		ENTER_NAME_MESSAGE,										 // pString
//		-1,																		 // Count
//		&main_rect,														 // pRect
//		DT_CENTER | DT_VCENTER | DT_NOCLIP,		 // Format,
//		0xFFFFFFFF);													 // Color
//
//	stringstream p, q;
//	string s;
//
//	for (unsigned int i = 0; i < m_unCurrentCharIdx; i++)
//	{
//		q << m_ucPlayerName[i];
//	}
//	q << m_ucCurrentChar;
//	s = q.str();
//	m_pLargeFont->DrawText(
//		NULL,																	 // pSprite
//		s.c_str(),														 // pString
//		-1,																		 // Count
//		&entry_rect,													 // pRect
//		DT_CENTER | DT_VCENTER | DT_NOCLIP,		 // Format,
//		0xFFFFFFFF);													 // Color
//
//	return S_OK;
//}

//------------------------------------------------------------------------------
void
ScoreBoard::setWindowSize(int screenWidth, int screenHeight)
{
	m_screenWidth	= screenWidth;
	m_screenHeight = screenHeight;
}

//------------------------------------------------------------------------------
