#pragma once

#include "Editor/IMode.h"
#include "Editor/LevelListMode.h"
#include "Editor/LevelEditorMode.h"
#include "Editor/FormationListMode.h"
#include "Editor/FormationSectionEditorMode.h"
#include "Editor/PathListMode.h"
#include "Editor/PathEditorMode.h"

//------------------------------------------------------------------------------
struct Modes
{
	LevelListMode levelListMode;
	LevelEditorMode levelEditorMode;

	FormationListMode formationListMode;
	FormationSectionEditorMode formationSectionEditorMode;

	PathListMode pathListMode;
	PathEditorMode pathEditorMode;

	IMode* pCurrentMode = &levelListMode;

	Modes(AppContext& context, AppResources& resources, GameLogic& logic)
			: levelListMode(*this, context, resources, logic)
			, levelEditorMode(*this, context, resources, logic)
			, formationListMode(*this, context, resources, logic)
			, formationSectionEditorMode(*this, context, resources, logic)
			, pathListMode(*this, context, resources, logic)
			, pathEditorMode(*this, context, resources, logic)
	{
	}

	void initModes()
	{
		levelListMode.init();
		levelEditorMode.init();
		formationListMode.init();
		formationSectionEditorMode.init();
		pathListMode.init();
		pathEditorMode.init();
	}

	void enterMode(IMode* pNewMode, bool isNavigatingForward = true)
	{
		ASSERT(pCurrentMode);
		ASSERT(pNewMode);
		pCurrentMode->onExitMode();
		pCurrentMode = pNewMode;
		pCurrentMode->onEnterMode(isNavigatingForward);
	}
};

//------------------------------------------------------------------------------
