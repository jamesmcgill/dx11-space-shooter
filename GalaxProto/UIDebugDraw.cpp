#include "pch.h"
#include "UIDebugDraw.h"
#include "AppContext.h"
#include "AppResources.h"

//------------------------------------------------------------------------------
UIDebugDraw::UIDebugDraw(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
{
}

//------------------------------------------------------------------------------
void
UIDebugDraw::drawBox(
	float x, float y, float width, float height, DirectX::FXMVECTOR color)
{
	auto dc			 = m_resources.m_deviceResources->GetD3DDeviceContext();
	auto& states = *m_resources.m_states;

	// Transform screen coordinates
	x -= m_context.screenHalfWidth;
	y -= m_context.screenHalfHeight;
	y = -y;

	dc->OMSetBlendState(states.Opaque(), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(states.DepthNone(), 0);
	dc->RSSetState(states.CullNone());
	m_resources.m_debugEffect->SetView(m_context.view);
	m_resources.m_debugEffect->SetProjection(m_context.orthoProj);
	m_resources.m_debugEffect->Apply(dc);
	dc->IASetInputLayout(m_resources.m_debugInputLayout.Get());

	DirectX::SimpleMath::Vector2 topLeft(x, y);
	DirectX::SimpleMath::Vector2 botLeft(x, y - height);
	DirectX::SimpleMath::Vector2 botRight(x + width, y - height);
	DirectX::SimpleMath::Vector2 topRight(x + width, y);

	DirectX::VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, topLeft);
	XMStoreFloat3(&verts[1].position, botLeft);
	XMStoreFloat3(&verts[2].position, botRight);
	XMStoreFloat3(&verts[3].position, topRight);

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);
	XMStoreFloat4(&verts[3].color, color);

	m_resources.m_batch->DrawQuad(verts[0], verts[1], verts[2], verts[3]);
}

//------------------------------------------------------------------------------
