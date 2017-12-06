#include "pch.h"
#include "UIDebugDraw.h"
#include "AppContext.h"
#include "AppResources.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace ui
{
//------------------------------------------------------------------------------
void
drawBox(
	DX::DebugBatchType& primitiveBatch,
	float x,
	float y,
	float width,
	float height,
	FXMVECTOR color,
	float layer)
{
	const XMVECTOR topLeft	= {x, y, layer};
	const XMVECTOR topRight = {x + width, y, layer};
	const XMVECTOR botRight = {x + width, y + height, layer};
	const XMVECTOR botLeft	= {x, y + height, layer};

	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, topLeft);
	XMStoreFloat3(&verts[1].position, topRight);
	XMStoreFloat3(&verts[2].position, botRight);
	XMStoreFloat3(&verts[3].position, botLeft);

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);
	XMStoreFloat4(&verts[3].color, color);

	primitiveBatch.DrawQuad(verts[0], verts[1], verts[2], verts[3]);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
DebugDraw::DebugDraw(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
{
}

//------------------------------------------------------------------------------
void
DebugDraw::begin2D()
{
	auto dc			 = m_resources.m_deviceResources->GetD3DDeviceContext();
	auto& states = *m_resources.m_states;

	auto textBlend				 = states.AlphaBlend();
	auto blendState				 = states.Opaque();
	auto depthStencilState = states.DepthDefault();
	auto rasterizerState	 = states.CullNone();
	auto samplerState			 = states.LinearClamp();

	dc->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(depthStencilState, 0);
	dc->RSSetState(rasterizerState);
	dc->PSSetSamplers(0, 1, &samplerState);
	dc->IASetInputLayout(m_resources.m_debugInputLayout.Get());

	m_resources.m_debugEffect->SetView(Matrix::Identity);
	m_resources.m_debugEffect->SetProjection(m_context.pixelsToProjection);
	m_resources.m_debugEffect->Apply(dc);

	m_resources.m_spriteBatch->Begin(
		DirectX::SpriteSortMode_Deferred,
		textBlend,
		samplerState,
		depthStencilState,
		rasterizerState);

	m_resources.m_batch->Begin();
}

//------------------------------------------------------------------------------
void
DebugDraw::end2D()
{
	m_resources.m_batch->End();
	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
};		// namespace ui
