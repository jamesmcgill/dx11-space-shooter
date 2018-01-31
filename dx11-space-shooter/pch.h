//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0602
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

// DirectXTK
#include "CommonStates.h"
#include "GeometricPrimitive.h"
#include "SimpleMath.h"
#include "Effects.h"
#include "Model.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "SimpleMath.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SpriteFont.h"
#include "DDSTextureLoader.h"
#include "DebugDraw.h"
#include "Audio.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <iostream>
#include <random>
#include "fmt/format.h"
#include <crtdbg.h>

#define ASSERT _ASSERTE

namespace DX
{
// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
	com_exception(HRESULT hr)
			: result(hr)
	{
	}

	virtual const char* what() const override
	{
		static char s_str[64] = {0};
		sprintf_s(s_str, "Failure with HRESULT of %08X", result);
		return s_str;
	}

private:
	HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void
ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw com_exception(hr);
	}
}

}		 // namespace DX

namespace strUtils
{
static std::string
wstringToUtf8(const std::wstring& str)
{
	std::string outStr(str.length(), 0);
	WideCharToMultiByte(
		CP_UTF8,
		0,
		str.data(),
		static_cast<int>(str.length()),
		outStr.data(),
		static_cast<int>(outStr.length()),
		NULL,
		NULL);
	return outStr;
}

static std::wstring
utf8ToWstring(const std::string& str)
{
	std::wstring outStr(str.length(), 0);
	MultiByteToWideChar(
		CP_UTF8,
		0,
		str.data(),
		static_cast<int>(str.length()),
		outStr.data(),
		static_cast<int>(outStr.length()));
	return outStr;
}

}		 // namespace strUtils
