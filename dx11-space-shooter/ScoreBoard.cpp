#include "pch.h"
#include "ScoreBoard.h"
#include "AppContext.h"
#include "AppResources.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
static const wchar_t* SCORE_FILENAME  = L"score.dat";
static const size_t NUM_SCORES        = 10;
static const XMVECTOR HIGHLIGHT_COLOR = {1.0f, 1.0f, 0.0f};
static const XMVECTOR NORMAL_COLOR    = {1.0f, 1.0f, 1.0f};
static const int FILE_VERSION         = 100;
static const wchar_t* DEFAULT_NAME    = L"Jim";
static const float X_PADDING          = 30.0f;

//------------------------------------------------------------------------------
void
ScoreBoard::render(DirectX::SpriteBatch& spriteBatch)
{
  TRACE

  // Layout
  //
  //       ####
  // ####  ----
  // ----  ####
  // ####  ---- X
  // ----  ####
  // ####  ----
  //       ####
  auto& font       = m_resources.font32pt;
  float fontHeight = XMVectorGetY(font->MeasureString(L"XXX"));
  float padding    = fontHeight * 0.3f;
  Vector2 pos      = {m_context.screenHalfWidth, 0.0f};

  float numRowsAboveCenter = (m_scores.size() % 2 == 0)
                               ? m_scores.size() / 2.0f
                               : ((m_scores.size() + 1) / 2.0f) - 0.5f;
  float numPaddingRowsAbove = numRowsAboveCenter - 0.5f;

  // Screen center - text rows - padding rows
  pos.y = m_context.screenHalfHeight - (numRowsAboveCenter * fontHeight)
          - (numPaddingRowsAbove * padding);

  // Render Scoreboard
  std::wstring scoreStr;
  for (auto& e : m_scores)
  {
    scoreStr = fmt::format(L"{} ", e.score);

    XMVECTOR dimensions = font->MeasureString(scoreStr.c_str());
    Vector2 scoreOrigin = {(XMVectorGetX(dimensions)), 0.0f};
    Vector2 nameOrigin  = {0.0f, 0.0f};

    font->DrawString(
      &spriteBatch,
      scoreStr.c_str(),
      {pos.x - X_PADDING, pos.y},
      NORMAL_COLOR,
      0.f,
      scoreOrigin);

    font->DrawString(
      &spriteBatch, e.playerName.c_str(), pos, NORMAL_COLOR, 0.f, nameOrigin);

    pos.y += fontHeight + padding;
  }
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
  TRACE
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
bool
ScoreBoard::loadFromFile()
{
  TRACE
  m_scores.clear();
  std::wifstream file(SCORE_FILENAME, std::ios::in);
  if (!file.is_open())
  {
    loadDefaultScores();
    LOG_ERROR("Couldn't open file: %ws", SCORE_FILENAME);
    return false;
  }

  std::wstring versionStr;
  if (!getline(file, versionStr))
  {
    LOG_ERROR("Couldn't read from file: %ws", SCORE_FILENAME);
    return false;
  }
  int version = std::stoi(versionStr);
  if (version != FILE_VERSION)
  {
    LOG_ERROR("Invalid Version in scores file: %ws", SCORE_FILENAME);
    return false;
  }

  std::wstring scoreStr;
  std::wstring nameStr;
  while (getline(file, scoreStr))
  {
    if (!getline(file, nameStr))
    {
      LOG_ERROR("Couldn't read from file: %ws", SCORE_FILENAME);
      return false;
    }

    int value = std::stoi(scoreStr);
    Score score{value, nameStr};
    m_scores.push_back(score);
  }

  return true;
}

//------------------------------------------------------------------------------
bool
ScoreBoard::saveToFile()
{
  TRACE
  std::wofstream file(SCORE_FILENAME, std::ios::out);
  if (!file.is_open())
  {
    LOG_ERROR("Couldn't open %ws for saving", SCORE_FILENAME);
    return false;
  }

  file << FILE_VERSION << '\n';
  for (auto& e : m_scores)
  {
    file << e.score << '\n';
    file << e.playerName << '\n';
  }

  return true;
}

//------------------------------------------------------------------------------
void
ScoreBoard::loadDefaultScores()
{
  TRACE
  m_scores.clear();
  int score = 10000;
  for (size_t i = 0; i < NUM_SCORES; ++i)
  {
    m_scores.emplace_back(Score{score, DEFAULT_NAME});
    score -= 1000;
  }
}

//------------------------------------------------------------------------------
