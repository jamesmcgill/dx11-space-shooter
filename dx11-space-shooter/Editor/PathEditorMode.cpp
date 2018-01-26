#include "pch.h"
#include "Editor/PathEditorMode.h"
#include "Editor/Modes.h"
#include "AppContext.h"
#include "AppResources.h"
#include "GameLogic.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
static constexpr float CAMERA_DIST_MULT = 1.25f;

//------------------------------------------------------------------------------
std::wstring
PathEditorMode::controlInfoText() const
{
	return L"Select Point(Mouse Hover), Create(C), Delete(Del), "
				 "Move Points(Mouse Drag), Back(Esc)";
}

//------------------------------------------------------------------------------
std::wstring
PathEditorMode::menuTitle() const
{
	return L"Path Editor";
}

//------------------------------------------------------------------------------
std::wstring
PathEditorMode::itemName(size_t itemIdx) const
{
	return fmt::format(
		L"Path:{}-{} ", pathRef(m_context.editorPathIdx).id, itemIdx);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onBack()
{
	m_modes.enterMode(&m_modes.pathListMode, false);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onCreate()
{
	pathRef(m_context.editorPathIdx).waypoints.emplace_back(Waypoint());
}

//------------------------------------------------------------------------------
void
PathEditorMode::onDeleteItem(size_t itemIdx)
{
	auto& path = pathRef(m_context.editorPathIdx);
	path.waypoints.erase(path.waypoints.begin() + itemIdx);
}

//------------------------------------------------------------------------------
size_t
PathEditorMode::lastItemIdx() const
{
	return pathRef(m_context.editorPathIdx).waypoints.size() - 1;
}

//------------------------------------------------------------------------------
void
PathEditorMode::onEnterMode(bool isNavigatingForward)
{
	m_gameLogic.m_enemies.reset();
	m_context.cameraDistance = m_context.defaultCameraDistance * CAMERA_DIST_MULT;
	m_context.updateViewMatrix();
	IMode::onEnterMode(isNavigatingForward);
}

//------------------------------------------------------------------------------
void
PathEditorMode::onExitMode()
{
	m_context.cameraDistance = m_context.defaultCameraDistance;
	m_context.updateViewMatrix();
	IMode::onExitMode();
}

//------------------------------------------------------------------------------
void
PathEditorMode::render()
{
	size_t pointIdx		= (isControlSelected) ? -1 : m_selectedIdx;
	size_t controlIdx = (isControlSelected) ? m_selectedIdx : -1;

	pathRef(m_context.editorPathIdx)
		.debugRender(m_resources.m_batch.get(), pointIdx, controlIdx);
	m_gameLogic.renderPlayerBoundary();
}

//------------------------------------------------------------------------------
void
PathEditorMode::handleInput(const DX::StepTimer& timer)
{
	TRACE
	UNREFERENCED_PARAMETER(timer);

	IMode::handleInput(timer);

	auto& path = pathRef(m_context.editorPathIdx);

	using DirectX::SimpleMath::Vector3;
	DirectX::SimpleMath::Matrix worldToScreen = m_context.worldToView
																							* m_context.viewToProjection
																							* m_context.projectionToPixels;

	DirectX::SimpleMath::Matrix screenToView
		= m_context.pixelsToProjection * m_context.viewToProjection.Invert();

	DirectX::SimpleMath::Matrix screenToWorld
		= m_context.pixelsToProjection * m_context.viewToProjection.Invert()
			* m_context.worldToView.Invert();

	const auto& mouseBtns	= m_resources.mouseTracker;
	const auto& mouseState = m_resources.m_mouse->GetState();

	auto isMouseOverPoint
		= [&worldToScreen](
				const DirectX::Mouse::State& mouse, const Vector3& point) -> bool {

		auto screenPos					= Vector3::Transform(point, worldToScreen);
		const float mX					= static_cast<float>(mouse.x);
		const float mY					= static_cast<float>(mouse.y);
		static const float SIZE = 10.f;

		return (mX >= screenPos.x - SIZE) && (mX <= screenPos.x + SIZE)
					 && (mY >= screenPos.y - SIZE) && (mY <= screenPos.y + SIZE);
	};

	for (size_t i = 0; i < path.waypoints.size(); ++i)
	{
		auto& waypoint = path.waypoints[i];
		if (isMouseOverPoint(mouseState, waypoint.wayPoint))
		{
			m_selectedIdx			= i;
			isControlSelected = false;
		}
		if (isMouseOverPoint(mouseState, waypoint.controlPoint))
		{
			m_selectedIdx			= i;
			isControlSelected = true;
		}
	}

	// Move points with mouse
	using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
	if (mouseBtns.leftButton == ButtonState::HELD)
	{
		Vector3 mouseScreenPos = {
			static_cast<float>(mouseState.x), static_cast<float>(mouseState.y), 1.0f};
		auto mouseInWorldPos = Vector3::Transform(mouseScreenPos, screenToWorld);

		const Vector3& cameraPos = m_context.cameraPos();
		const Vector3 rayDir		 = mouseInWorldPos - cameraPos;

		static const DirectX::SimpleMath::Plane plane(
			Vector3(), Vector3(0.0f, 0.0f, 1.0f));
		DirectX::SimpleMath::Ray ray(cameraPos, rayDir);

		float dist;
		if (ray.Intersects(plane, dist))
		{
			auto& point = (isControlSelected)
											? path.waypoints[m_selectedIdx].controlPoint
											: path.waypoints[m_selectedIdx].wayPoint;
			point = cameraPos + (rayDir * dist);
		}
	}
}

//------------------------------------------------------------------------------
