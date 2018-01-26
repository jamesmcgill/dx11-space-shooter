#pragma once

#include "IMode.h"

//------------------------------------------------------------------------------
struct LevelEditorMode : public IMode
{
	LevelEditorMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override;
	std::wstring menuTitle() const override;
	std::wstring itemName(size_t itemIdx) const override;

	void onBack() override;
	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected() override;
	void onPlus() override;
	void onSubtract() override;
	void onPgUp() override;
	void onPgDn() override;

	size_t lastItemIdx() const override;

	void update(const DX::StepTimer& timer) override;
};

//------------------------------------------------------------------------------
