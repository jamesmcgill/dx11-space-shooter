#pragma once

#include "LevelData.h"
#include "UIDebugDraw.h"

//------------------------------------------------------------------------------
namespace DX
{
class StepTimer;
};
struct Modes;
struct AppContext;
struct AppResources;
class GameLogic;

//------------------------------------------------------------------------------
struct IMode
{
	Modes& m_modes;
	AppContext& m_context;
	AppResources& m_resources;
	GameLogic& m_gameLogic;

	ui::Text m_controlInfo;
	size_t m_firstIdx		 = 0;
	size_t m_lastIdx		 = 0;
	size_t m_selectedIdx = 0;

	IMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: m_modes(modes)
			, m_context(context)
			, m_resources(resources)
			, m_gameLogic(logic)
	{
	}

	virtual ~IMode() = default;

	virtual void onEnterMode(bool isNavigatingForward = true);
	virtual void onExitMode();

	virtual std::wstring controlInfoText() const				= 0;
	virtual std::wstring menuTitle() const							= 0;
	virtual std::wstring itemName(size_t itemIdx) const = 0;
	virtual std::wstring itemNameToDisplay(size_t itemIdx) const;

	virtual void onBack() {}
	virtual void onCreate() {}
	virtual void onDeleteItem(size_t itemIdx);
	virtual void onItemSelected() = 0;
	virtual void onItemCommand() {}

	virtual void onPlus() {}
	virtual void onSubtract() {}
	virtual void onPgUp() {}
	virtual void onPgDn() {}
	virtual void onLeft() {}
	virtual void onRight() {}
	virtual void onHome() {}
	virtual void onEnd() {}

	virtual size_t firstMenuIdx() const { return 0; }
	virtual size_t lastItemIdx() const = 0;

	void init();
	virtual void update(const DX::StepTimer& timer);
	virtual void handleInput(const DX::StepTimer& timer);
	virtual void render() {}
	virtual void renderUI();

	void updateIndices();

	//----------------------------------------------------------------------------
	void jumpToLevelWave(const size_t levelIdx, const size_t waveIdx);
	void spawnFormation(size_t formationIdx);
	void spawnPath(size_t pathIdx);

	//----------------------------------------------------------------------------
	LevelPool& levelsRef() const;
	FormationPool& formationsRef() const;
	PathPool& pathsRef() const;

	Level& levelRef(const size_t idx) const;
	Wave& levelWaveRef(size_t levelIdx, size_t waveIdx) const;
	Formation& formationRef(size_t idx) const;

	FormationSection&
	formationSectionRef(size_t formationIdx, size_t sectionIdx) const;
	Path& pathRef(size_t idx) const;
};

//------------------------------------------------------------------------------
