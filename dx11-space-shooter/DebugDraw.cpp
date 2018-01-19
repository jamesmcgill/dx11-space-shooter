//------------------------------------------------------------------------------
// File: DebugDraw.cpp
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright(c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//------------------------------------------------------------------------------

#include "pch.h"
#include "DebugDraw.h"
#include "AppContext.h"
#include "AppResources.h"

using namespace DirectX;

namespace
{
//------------------------------------------------------------------------------
inline void XM_CALLCONV
DrawCube(DX::DebugBatchType* batch, CXMMATRIX matWorld, FXMVECTOR color)
{
	static const XMVECTORF32 s_verts[8] = {{-1.f, -1.f, -1.f, 0.f},
																				 {1.f, -1.f, -1.f, 0.f},
																				 {1.f, -1.f, 1.f, 0.f},
																				 {-1.f, -1.f, 1.f, 0.f},
																				 {-1.f, 1.f, -1.f, 0.f},
																				 {1.f, 1.f, -1.f, 0.f},
																				 {1.f, 1.f, 1.f, 0.f},
																				 {-1.f, 1.f, 1.f, 0.f}};

	static const WORD s_indices[]
		= {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

	VertexPositionColor verts[8];
	for (size_t i = 0; i < 8; ++i)
	{
		XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
		XMStoreFloat3(&verts[i].position, v);
		XMStoreFloat4(&verts[i].color, color);
	}

	batch->DrawIndexed(
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		s_indices,
		_countof(s_indices),
		verts,
		8);
}

//------------------------------------------------------------------------------
}		 // namespace anon

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace DX
{
//------------------------------------------------------------------------------
DrawContext::DrawContext(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
{
}

//------------------------------------------------------------------------------
DrawContext::~DrawContext()
{
	if (m_isOpen)
	{
		end();
	}
}

//------------------------------------------------------------------------------
void
DrawContext::begin(Projection proj)
{
	auto& states	 = *m_resources.m_states;
	auto textBlend = states.AlphaBlend();
	auto blend		 = states.Opaque();
	auto depth		 = states.DepthDefault();		 // DepthNone
	auto raster		 = states.CullNone();
	auto sampler	 = states.LinearClamp();

	setProjection(proj);
	setStates(blend, depth, raster, sampler);

	m_resources.m_spriteBatch->Begin(
		DirectX::SpriteSortMode_Deferred, textBlend, sampler, depth, raster);

	m_resources.m_batch->Begin();

	m_isOpen = true;
}

//------------------------------------------------------------------------------
void
DrawContext::setProjection(Projection proj)
{
	switch (proj)
	{
		case Projection::Screen:
			setScreenProjection();
			break;
		case Projection::World:
			setWorldProjection();
			break;
	};
}

//------------------------------------------------------------------------------
void
DrawContext::setScreenProjection()
{
	m_resources.m_debugEffect->SetView(DirectX::SimpleMath::Matrix::Identity);
	m_resources.m_debugEffect->SetProjection(m_context.pixelsToProjection);
}

//------------------------------------------------------------------------------
void
DrawContext::setWorldProjection()
{
	m_resources.m_debugEffect->SetView(m_context.worldToView);
	m_resources.m_debugEffect->SetProjection(m_context.viewToProjection);
}

//------------------------------------------------------------------------------
void
DrawContext::end()
{
	m_isOpen = false;

	m_resources.m_batch->End();
	m_resources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
void
DrawContext::setStates(
	ID3D11BlendState* blend,
	ID3D11DepthStencilState* depth,
	ID3D11RasterizerState* raster,
	ID3D11SamplerState* sampler)
{
	auto dc = m_resources.m_deviceResources->GetD3DDeviceContext();
	dc->OMSetBlendState(blend, nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(depth, 0);
	dc->RSSetState(raster);
	dc->PSSetSamplers(0, 1, &sampler);
	dc->IASetInputLayout(m_resources.m_debugInputLayout.Get());

	m_resources.m_debugEffect->Apply(dc);
}

//------------------------------------------------------------------------------
}		 // namespace DX

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::Draw(
	DX::DebugBatchType* batch, const BoundingSphere& sphere, FXMVECTOR color)
{
	XMVECTOR origin = XMLoadFloat3(&sphere.Center);

	const float radius = sphere.Radius;

	XMVECTOR xaxis = g_XMIdentityR0 * radius;
	XMVECTOR yaxis = g_XMIdentityR1 * radius;
	XMVECTOR zaxis = g_XMIdentityR2 * radius;

	DrawRing(batch, origin, xaxis, zaxis, color);
	DrawRing(batch, origin, xaxis, yaxis, color);
	DrawRing(batch, origin, yaxis, zaxis, color);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::Draw(DX::DebugBatchType* batch, const BoundingBox& box, FXMVECTOR color)
{
	XMMATRIX matWorld
		= XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
	XMVECTOR position = XMLoadFloat3(&box.Center);
	matWorld.r[3]			= XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::Draw(
	DX::DebugBatchType* batch, const BoundingOrientedBox& obb, FXMVECTOR color)
{
	XMMATRIX matWorld
		= XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
	XMMATRIX matScale
		= XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
	matWorld					= XMMatrixMultiply(matScale, matWorld);
	XMVECTOR position = XMLoadFloat3(&obb.Center);
	matWorld.r[3]			= XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

	DrawCube(batch, matWorld, color);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::Draw(
	DX::DebugBatchType* batch, const BoundingFrustum& frustum, FXMVECTOR color)
{
	XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
	frustum.GetCorners(corners);

	VertexPositionColor verts[24] = {};
	verts[0].position							= corners[0];
	verts[1].position							= corners[1];
	verts[2].position							= corners[1];
	verts[3].position							= corners[2];
	verts[4].position							= corners[2];
	verts[5].position							= corners[3];
	verts[6].position							= corners[3];
	verts[7].position							= corners[0];

	verts[8].position	= corners[0];
	verts[9].position	= corners[4];
	verts[10].position = corners[1];
	verts[11].position = corners[5];
	verts[12].position = corners[2];
	verts[13].position = corners[6];
	verts[14].position = corners[3];
	verts[15].position = corners[7];

	verts[16].position = corners[4];
	verts[17].position = corners[5];
	verts[18].position = corners[5];
	verts[19].position = corners[6];
	verts[20].position = corners[6];
	verts[21].position = corners[7];
	verts[22].position = corners[7];
	verts[23].position = corners[4];

	for (size_t j = 0; j < _countof(verts); ++j)
	{
		XMStoreFloat4(&verts[j].color, color);
	}

	batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, verts, _countof(verts));
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawGrid(
	DX::DebugBatchType* batch,
	FXMVECTOR xAxis,
	FXMVECTOR yAxis,
	FXMVECTOR origin,
	size_t xdivs,
	size_t ydivs,
	GXMVECTOR color)
{
	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float percent	= float(i) / float(xdivs);
		percent				 = (percent * 2.f) - 1.f;
		XMVECTOR scale = XMVectorScale(xAxis, percent);
		scale					 = XMVectorAdd(scale, origin);

		VertexPositionColor v1(XMVectorSubtract(scale, yAxis), color);
		VertexPositionColor v2(XMVectorAdd(scale, yAxis), color);
		batch->DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; i++)
	{
		FLOAT percent	= float(i) / float(ydivs);
		percent				 = (percent * 2.f) - 1.f;
		XMVECTOR scale = XMVectorScale(yAxis, percent);
		scale					 = XMVectorAdd(scale, origin);

		VertexPositionColor v1(XMVectorSubtract(scale, xAxis), color);
		VertexPositionColor v2(XMVectorAdd(scale, xAxis), color);
		batch->DrawLine(v1, v2);
	}
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawRing(
	DX::DebugBatchType* batch,
	FXMVECTOR origin,
	FXMVECTOR majorAxis,
	FXMVECTOR minorAxis,
	GXMVECTOR color)
{
	static const size_t c_ringSegments = 32;

	VertexPositionColor verts[c_ringSegments + 1];

	FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
	// Instead of calling cos/sin for each segment we calculate
	// the sign of the angle delta and then incrementally calculate sin
	// and cosine from then on.
	XMVECTOR cosDelta											= XMVectorReplicate(cosf(fAngleDelta));
	XMVECTOR sinDelta											= XMVectorReplicate(sinf(fAngleDelta));
	XMVECTOR incrementalSin								= XMVectorZero();
	static const XMVECTORF32 s_initialCos = {1.f, 1.f, 1.f, 1.f};
	XMVECTOR incrementalCos								= s_initialCos.v;
	for (size_t i = 0; i < c_ringSegments; i++)
	{
		XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
		pos					 = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
		XMStoreFloat3(&verts[i].position, pos);
		XMStoreFloat4(&verts[i].color, color);
		// Standard formula to rotate a vector.
		XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
		XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
		incrementalCos	= newCos;
		incrementalSin	= newSin;
	}
	verts[c_ringSegments] = verts[0];

	batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawRay(
	DX::DebugBatchType* batch,
	FXMVECTOR origin,
	FXMVECTOR direction,
	bool normalize,
	FXMVECTOR color)
{
	VertexPositionColor verts[3];
	XMStoreFloat3(&verts[0].position, origin);

	XMVECTOR normDirection = XMVector3Normalize(direction);
	XMVECTOR rayDirection	= (normalize) ? normDirection : direction;

	XMVECTOR perpVector = XMVector3Cross(normDirection, g_XMIdentityR1);

	if (XMVector3Equal(XMVector3LengthSq(perpVector), g_XMZero))
	{
		perpVector = XMVector3Cross(normDirection, g_XMIdentityR2);
	}
	perpVector = XMVector3Normalize(perpVector);

	XMStoreFloat3(&verts[1].position, XMVectorAdd(rayDirection, origin));
	perpVector		= XMVectorScale(perpVector, 0.0625f);
	normDirection = XMVectorScale(normDirection, -0.25f);
	rayDirection	= XMVectorAdd(perpVector, rayDirection);
	rayDirection	= XMVectorAdd(normDirection, rayDirection);
	XMStoreFloat3(&verts[2].position, XMVectorAdd(rayDirection, origin));

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);

	batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawTriangle(
	DX::DebugBatchType* batch,
	FXMVECTOR pointA,
	FXMVECTOR pointB,
	FXMVECTOR pointC,
	GXMVECTOR color)
{
	VertexPositionColor verts[4];
	XMStoreFloat3(&verts[0].position, pointA);
	XMStoreFloat3(&verts[1].position, pointB);
	XMStoreFloat3(&verts[2].position, pointC);
	XMStoreFloat3(&verts[3].position, pointA);

	XMStoreFloat4(&verts[0].color, color);
	XMStoreFloat4(&verts[1].color, color);
	XMStoreFloat4(&verts[2].color, color);
	XMStoreFloat4(&verts[3].color, color);

	batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
}

//------------------------------------------------------------------------------
static FXMVECTOR
bezier(FLOAT t, FXMVECTOR startPos, FXMVECTOR endPos, FXMVECTOR control)
{
	// https://pomax.github.io/bezierinfo/
	float t2	= t * t;
	float mt	= 1 - t;
	float mt2 = mt * mt;
	return (startPos * mt2) + (control * (2 * mt * t)) + (endPos * t2);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawCurve(
	DX::DebugBatchType* batch,
	FXMVECTOR startPos,
	FXMVECTOR endPos,
	FXMVECTOR control,
	GXMVECTOR color)
{
	static const size_t numSegments = 20;
	VertexPositionColor verts[numSegments + 1];
	const FLOAT tDelta = 1.0f / float(numSegments);
	FLOAT t						 = tDelta;

	XMStoreFloat3(&verts[0].position, startPos);
	XMStoreFloat4(&verts[0].color, color);

	for (size_t i = 1; i < numSegments; ++i)
	{
		XMVECTOR pos = bezier(t, startPos, endPos, control);
		XMStoreFloat3(&verts[i].position, pos);
		XMStoreFloat4(&verts[i].color, color);
		t += tDelta;
	}

	XMStoreFloat3(&verts[numSegments].position, endPos);
	XMStoreFloat4(&verts[numSegments].color, color);

	batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, numSegments + 1);
}

//------------------------------------------------------------------------------
void XM_CALLCONV
DX::DrawLine(
	DX::DebugBatchType* batch,
	FXMVECTOR startPos,
	FXMVECTOR endPos,
	FXMVECTOR color)
{
	VertexPositionColor v1(startPos, color);
	VertexPositionColor v2(endPos, color);
	batch->DrawLine(v1, v2);
}

//------------------------------------------------------------------------------
