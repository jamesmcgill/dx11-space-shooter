//--------------------------------------------------------------------------------------
// File: DebugDraw.h
//
// Helpers for drawing various debug shapes using PrimitiveBatch
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright(c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//-------------------------------------------------------------------------------------

#pragma once

#include <DirectXCollision.h>

#include "PrimitiveBatch.h"
#include "VertexTypes.h"

namespace DX
{
void XM_CALLCONV Draw(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	const DirectX::BoundingSphere& sphere,
	DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	const DirectX::BoundingBox& box,
	DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	const DirectX::BoundingOrientedBox& obb,
	DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV Draw(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	const DirectX::BoundingFrustum& frustum,
	DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawGrid(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR xAxis,
	DirectX::FXMVECTOR yAxis,
	DirectX::FXMVECTOR origin,
	size_t xdivs,
	size_t ydivs,
	DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawRing(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR origin,
	DirectX::FXMVECTOR majorAxis,
	DirectX::FXMVECTOR minorAxis,
	DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawRay(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR origin,
	DirectX::FXMVECTOR direction,
	bool normalize					 = true,
	DirectX::FXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawTriangle(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR pointA,
	DirectX::FXMVECTOR pointB,
	DirectX::FXMVECTOR pointC,
	DirectX::GXMVECTOR color = DirectX::Colors::White);

void XM_CALLCONV DrawCurve(
	DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
	DirectX::FXMVECTOR startPos,
	DirectX::FXMVECTOR endPos,
	DirectX::FXMVECTOR control,
	DirectX::FXMVECTOR color = DirectX::Colors::White);
}