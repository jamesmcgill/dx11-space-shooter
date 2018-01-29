#pragma once

#include "UIDebugDraw.h"

namespace DX
{
class StepTimer;
};
struct AppContext;
struct AppResources;

//------------------------------------------------------------------------------
class ScoreBoard
{
public:
	struct Score
	{
		int score = 0;
		std::wstring playerName;
	};

public:
	ScoreBoard(AppContext& context, AppResources& resources)
			: m_context(context)
			, m_resources(resources)
	{
	}

	void render(DirectX::SpriteBatch& spriteBatch);

	bool isHiScore(const int& nScore) const;
	void insertScore(Score newScore);

	bool loadFromFile();
	bool saveToFile();
	void loadDefaultScores();

private:
	AppContext& m_context;
	AppResources& m_resources;

	std::vector<Score> m_scores;
};
