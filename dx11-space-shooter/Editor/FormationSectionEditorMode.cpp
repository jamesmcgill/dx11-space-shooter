#include "pch.h"
#include "Editor/FormationSectionEditorMode.h"
#include "Editor/Modes.h"
#include "AppContext.h"
#include "Enemies.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
namespace
{
// Prevent displaying the dummy data at index[0]
constexpr size_t PATH_FIRST_IDX = Enemies::DUMMY_PATH_IDX + 1;
static const int MAX_NUM_SHIPS	= 10;
};		// anon namespace

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::wstring
FormationSectionEditorMode::controlInfoText() const
{
	return L"Navigate(Up/Down), Select(Enter), Create(C), Delete(Del), "
				 "Model(Home/End), Num Ships(-/+), Path(PgUp/PgDn), Back(Esc)";
}

//------------------------------------------------------------------------------
std::wstring
FormationSectionEditorMode::menuTitle() const
{
	return L"Formation Section";
}

//------------------------------------------------------------------------------
std::wstring
FormationSectionEditorMode::itemName(size_t itemIdx) const
{
	auto& section = formationSectionRef(m_context.editorFormationIdx, itemIdx);
	auto& path		= pathRef(section.pathIdx);

	return fmt::format(
		L"{}: Model:{}, numShips:{}, Path:{} ",
		itemIdx,
		static_cast<int>(section.model) + 1,
		section.numShips,
		path.id);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onBack()
{
	m_modes.enterMode(&m_modes.formationListMode, false);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onCreate()
{
	if (PATH_FIRST_IDX >= pathsRef().size())
	{
		return;		 // Do nothing. No paths available
	}

	auto& formation = formationRef(m_context.editorFormationIdx);

	FormationSection section;
	section.pathIdx	= PATH_FIRST_IDX;
	section.numShips = 3;
	section.model		 = ModelResource::Enemy1;
	formation.sections.emplace_back(section);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& formation = formationRef(m_context.editorFormationIdx);
	formation.sections.erase(formation.sections.begin() + itemIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onItemSelected()
{
	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPlus()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	section.numShips
		= (section.numShips < MAX_NUM_SHIPS) ? section.numShips + 1 : MAX_NUM_SHIPS;

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onSubtract()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	if (section.numShips > 1)
	{
		--section.numShips;
	}

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgUp()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	auto& paths = pathsRef();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx > PATH_FIRST_IDX) ? curIdx - 1 : lastIdx;

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onPgDn()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	auto& paths = pathsRef();

	auto& curIdx				 = section.pathIdx;
	const size_t lastIdx = (paths.size() > 0) ? paths.size() - 1 : 0;

	curIdx = (curIdx < lastIdx) ? curIdx + 1 : PATH_FIRST_IDX;

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onHome()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	const int minIdx = static_cast<int>(ModelResource::Enemy1);
	const int maxIdx = static_cast<int>(ModelResource::Player);

	int curIdx		= static_cast<int>(section.model);
	curIdx				= (curIdx > minIdx) ? curIdx - 1 : maxIdx;
	section.model = static_cast<ModelResource>(curIdx);

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
void
FormationSectionEditorMode::onEnd()
{
	auto& section
		= formationSectionRef(m_context.editorFormationIdx, m_selectedIdx);
	const int minIdx = static_cast<int>(ModelResource::Enemy1);
	const int maxIdx = static_cast<int>(ModelResource::Player);

	int curIdx		= static_cast<int>(section.model);
	curIdx				= (curIdx < maxIdx) ? curIdx + 1 : minIdx;
	section.model = static_cast<ModelResource>(curIdx);

	spawnFormation(m_context.editorFormationIdx);
}

//------------------------------------------------------------------------------
size_t
FormationSectionEditorMode::lastItemIdx() const
{
	return formationRef(m_context.editorFormationIdx).sections.size() - 1;
}

//------------------------------------------------------------------------------
