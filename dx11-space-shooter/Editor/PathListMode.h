#pragma once

#include "Editor/IRenameableMode.h"

//------------------------------------------------------------------------------
struct PathListMode : public IRenameableMode
{
	PathListMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IRenameableMode(modes, context, resources, logic)
	{
	}

	std::wstring controlInfoText() const override;
	std::wstring menuTitle() const override;
	std::wstring itemName(size_t itemIdx) const override;
	std::wstring itemNameToDisplay(size_t itemIdx) const override;
	void setItemName(size_t itemIdx, const std::wstring& newName) override;

	void onCreate() override;
	void onDeleteItem(size_t itemIdx) override;
	void onItemSelected() override;
	void onItemCommand() override;

	size_t firstMenuIdx() const override;
	size_t lastItemIdx() const override;
};

//------------------------------------------------------------------------------
