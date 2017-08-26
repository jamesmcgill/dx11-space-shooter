#pragma once

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
	std::vector<Score> m_scores;

	int m_screenWidth	= 0;
	int m_screenHeight = 0;

	// New Score Entry
	bool m_isEntryModeOn = false;
	char m_ucCurrentChar = 'A';
	char m_ucPlayerName[4];
	unsigned int m_unCurrentCharIdx = 0;

public:
	void render(DirectX::SpriteFont* font, DirectX::SpriteBatch* spriteBatch);
	void setWindowSize(int screenWidth, int screenHeight);

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
