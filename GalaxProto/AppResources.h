#pragma once
#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "Entity.h"	// ModelData
#include "Starfield.h"
#include "MenuManager.h"

//------------------------------------------------------------------------------
struct AppResources
{
	int m_screenWidth = 0;
	int m_screenHeight = 0;

	DX::StepTimer m_timer;

	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	DirectX::Keyboard::KeyboardStateTracker kbTracker;

	std::unique_ptr<DirectX::CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_font;

	std::unique_ptr<DX::DebugBatchType> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_debugInputLayout;
	std::unique_ptr<DirectX::BasicEffect> m_debugEffect;

	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	std::shared_ptr<DirectX::BasicEffect> m_debugBoundEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_debugBoundInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive> m_debugBound;

	std::unique_ptr<StarField> starField;
	std::unique_ptr<MenuManager> menuManager;

	std::map<char*, wchar_t*> modelLocations;
	std::map<char*, ModelData> modelData;

	AppResources()
			: m_keyboard(std::make_unique<DirectX::Keyboard>())
	{
	}
};

//------------------------------------------------------------------------------
