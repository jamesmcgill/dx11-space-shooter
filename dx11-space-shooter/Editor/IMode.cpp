#include "pch.h"
#include "IMode.h"
#include "GameLogic.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
namespace
{
const float MODE_BUTTON_HEIGHT		 = 40.0f;
const float MODE_BUTTON_POSITION_Y = 20.0f;

const float MAIN_AREA_START_X = 20.0f;
const float MAIN_AREA_START_Y
	= (1.5f * MODE_BUTTON_POSITION_Y) + MODE_BUTTON_HEIGHT;

const float LIST_START_Y = MAIN_AREA_START_Y + 40.0f;

const DirectX::XMVECTORF32 SELECTED_ITEM_COLOR = DirectX::Colors::White;
const DirectX::XMVECTORF32 NORMAL_ITEM_COLOR = DirectX::Colors::MediumVioletRed;
};
//------------------------------------------------------------------------------
void
IMode::onEnterMode(bool isNavigatingForward)
{
	if (isNavigatingForward)
	{
		m_selectedIdx = firstMenuIdx();
	}
	updateIndices();
	onItemSelected();
}

//------------------------------------------------------------------------------
void
IMode::onExitMode()
{
	m_gameLogic.m_enemies.save();
};

//------------------------------------------------------------------------------
std::wstring
IMode::itemNameToDisplay(size_t itemIdx) const
{
	return itemName(itemIdx);
};

//------------------------------------------------------------------------------
void
IMode::onDeleteItem(size_t itemIdx)
{
	UNREFERENCED_PARAMETER(itemIdx);
}

//------------------------------------------------------------------------------
void
IMode::update(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);
}

//------------------------------------------------------------------------------
void
IMode::updateIndices()
{
	m_firstIdx = firstMenuIdx();
	m_lastIdx	= lastItemIdx() + 1;		 // +1 for CREATE button
}

//------------------------------------------------------------------------------
void
IMode::jumpToLevelWave(const size_t levelIdx, const size_t waveIdx)
{
	m_gameLogic.reset();
	m_gameLogic.m_enemies.jumpToLevel(levelIdx);
	m_gameLogic.m_enemies.jumpToWave(waveIdx);
}

//------------------------------------------------------------------------------
void
IMode::spawnFormation(size_t formationIdx)
{
	m_gameLogic.m_enemies.reset();
	m_gameLogic.m_enemies.spawnFormation(formationIdx, 0.0f);
}

//------------------------------------------------------------------------------
void
IMode::spawnPath(size_t pathIdx)
{
	m_gameLogic.m_enemies.reset();
	m_gameLogic.m_enemies.spawnFormationSection(
		5, pathIdx, ModelResource::Enemy9, 0.0f);
}

//------------------------------------------------------------------------------
LevelPool&
IMode::levelsRef() const
{
	return m_gameLogic.m_enemies.m_levels;
}

//------------------------------------------------------------------------------
FormationPool&
IMode::formationsRef() const
{
	return m_gameLogic.m_enemies.m_formationPool;
}

//------------------------------------------------------------------------------
PathPool&
IMode::pathsRef() const
{
	return m_gameLogic.m_enemies.m_pathPool;
}

//------------------------------------------------------------------------------
Level&
IMode::levelRef(const size_t idx) const
{
	auto& levels = levelsRef();
	ASSERT(idx < levels.size());
	return levels[idx];
}

//------------------------------------------------------------------------------
Wave&
IMode::levelWaveRef(size_t levelIdx, size_t waveIdx) const
{
	auto& waves = levelRef(levelIdx).waves;
	ASSERT(waveIdx < waves.size());
	return waves[waveIdx];
}

//------------------------------------------------------------------------------
Formation&
IMode::formationRef(size_t idx) const
{
	auto& formations = formationsRef();
	ASSERT(idx < formations.size());
	return formations[idx];
}

//------------------------------------------------------------------------------
FormationSection&
IMode::formationSectionRef(size_t formationIdx, size_t sectionIdx) const
{
	auto& formation = formationRef(formationIdx);
	ASSERT(sectionIdx < formation.sections.size());
	return formation.sections[sectionIdx];
}

//------------------------------------------------------------------------------
Path&
IMode::pathRef(size_t idx) const
{
	auto& paths = pathsRef();
	ASSERT(idx < paths.size());
	return paths[idx];
}

//------------------------------------------------------------------------------
void
IMode::init()
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::XMVECTOR;

	m_controlInfo.text = controlInfoText();
	m_controlInfo.font = m_resources.fontMono8pt.get();

	XMVECTOR dimensions
		= m_controlInfo.font->MeasureString(m_controlInfo.text.c_str());
	const float height		 = DirectX::XMVectorGetY(dimensions);
	m_controlInfo.position = Vector2(MAIN_AREA_START_X, MAIN_AREA_START_Y);
	m_controlInfo.origin	 = Vector2(0.0f, 0.0f);
	m_controlInfo.color		 = DirectX::Colors::MediumVioletRed;
}

//------------------------------------------------------------------------------
void
IMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);
	const auto& kb = m_resources.kbTracker;
	using DirectX::Keyboard;

	// Navigation Controls
	if (kb.IsKeyPressed(Keyboard::Escape))
	{
		onBack();
	}

	if (kb.IsKeyPressed(Keyboard::Up))
	{
		m_selectedIdx
			= (m_selectedIdx > m_firstIdx) ? m_selectedIdx - 1 : m_lastIdx;
		onItemSelected();
	}
	else if (kb.IsKeyPressed(Keyboard::Down))
	{
		m_selectedIdx
			= (m_selectedIdx < m_lastIdx) ? m_selectedIdx + 1 : m_firstIdx;
		onItemSelected();
	}

	if (
		kb.IsKeyPressed(Keyboard::LeftControl) || kb.IsKeyPressed(Keyboard::Space)
		|| kb.IsKeyPressed(Keyboard::Enter))
	{
		const size_t& createItemIdx = m_lastIdx;
		if (m_selectedIdx == createItemIdx)
		{
			onCreate();
			updateIndices();
			m_selectedIdx = m_lastIdx;
			onItemSelected();
		}
		else
		{
			onItemCommand();
		}
	}

	// Edit Item Controls
	const size_t& createItemIdx = m_lastIdx;
	if (m_selectedIdx < createItemIdx)
	{
		if (kb.IsKeyPressed(Keyboard::Left))
		{
			onLeft();
		}
		else if (kb.IsKeyPressed(Keyboard::Right))
		{
			onRight();
		}
		if (kb.IsKeyPressed(Keyboard::Add) || kb.IsKeyPressed(Keyboard::OemPlus))
		{
			onPlus();
		}
		else if (
			kb.IsKeyPressed(Keyboard::Subtract)
			|| kb.IsKeyPressed(Keyboard::OemMinus))
		{
			onSubtract();
		}
		if (kb.IsKeyPressed(Keyboard::PageUp))
		{
			onPgUp();
		}
		else if (kb.IsKeyPressed(Keyboard::PageDown))
		{
			onPgDn();
		}
		if (kb.IsKeyPressed(Keyboard::Home))
		{
			onHome();
		}
		else if (kb.IsKeyPressed(Keyboard::End))
		{
			onEnd();
		}

		if (kb.IsKeyPressed(Keyboard::C))
		{
			onCreate();
			updateIndices();
		}
		if (kb.IsKeyPressed(Keyboard::Delete))
		{
			onDeleteItem(m_selectedIdx);
			updateIndices();
			onItemSelected();
		}
	}
}

//------------------------------------------------------------------------------
void
IMode::renderUI()
{
	TRACE
	m_controlInfo.draw(*m_resources.m_spriteBatch);

	auto monoFont = m_resources.fontMono8pt.get();
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));

	using DirectX::SimpleMath::Vector2;
	Vector2 position = {MAIN_AREA_START_X, LIST_START_Y};
	Vector2 origin	 = Vector2(0.f, 0.f);
	Vector2 scale		 = Vector2(1.f, 1.f);

	auto drawMenuItem = [&, &spriteBatch = m_resources.m_spriteBatch](
		bool isSelected, const std::wstring& text)
	{
		monoFont->DrawString(
			spriteBatch.get(),
			text.c_str(),
			position,
			(isSelected) ? SELECTED_ITEM_COLOR : NORMAL_ITEM_COLOR,
			0.0f,
			origin,
			scale,
			DirectX::SpriteEffects_None,
			ui::Layer::L5_Default);
		position.y += yAscent;
	};

	std::wstring title = menuTitle();
	size_t titleSize	 = title.size();
	drawMenuItem(false, title);
	drawMenuItem(false, std::wstring(titleSize, '-'));

	size_t idx = m_firstIdx;
	while (idx < m_lastIdx)
	{
		drawMenuItem((idx == m_selectedIdx), itemNameToDisplay(idx));
		idx++;
	}
	drawMenuItem((idx == m_selectedIdx), L"CREATE");
}

//------------------------------------------------------------------------------
