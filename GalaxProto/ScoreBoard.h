#pragma once

struct AppContext;

//------------------------------------------------------------------------------
class ScoreBoard
{
public:
	struct Score
	{
		int score;
		std::wstring playerName;
	};

private:
	AppContext& m_context;
	std::vector<Score> m_scores;

	// New Score Entry
	bool m_isEntryModeOn = false;
	char m_ucCurrentChar = 'A';
	char m_ucPlayerName[4];
	unsigned int m_unCurrentCharIdx = 0;

public:
	ScoreBoard(AppContext& context);
	void render(DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch);

	// Input
	void PrevMenu();
	void NextItem();
	void PrevItem();
	void SelectItem();

	void loadFromFile();
	void saveToFile();

	void loadDefaultScores();
	bool isHiScore(const int& nScore) const;
	void insertScore(Score newScore);
};
