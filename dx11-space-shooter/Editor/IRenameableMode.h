#pragma once

#include "IMode.h"

//------------------------------------------------------------------------------
struct IRenameableMode : public IMode
{
	static const size_t RENAME_DISABLED = static_cast<size_t>(-1);

	size_t m_renamingIdx = RENAME_DISABLED;
	std::wstring m_renameBuffer;

	IRenameableMode(
		Modes& modes,
		AppContext& context,
		AppResources& resources,
		GameLogic& logic)
			: IMode(modes, context, resources, logic)
	{
	}

	// IMode
	void onEnterMode(bool isNavigatingForward = true) override;
	void handleInput(const DX::StepTimer& timer) override;

	// IRenameableMode
	void handleInput_renaming(const DX::StepTimer& timer);
	virtual void setItemName(size_t itemIdx, std::wstring newName) = 0;

	std::wstring renameText() const;
};

//------------------------------------------------------------------------------
