#pragma once
#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "ResourceIDs.h"		// ModelResource, AudioResource
#include "Entity.h"					// ModelData
#include "Starfield.h"
#include "Explosions.h"
#include "MenuManager.h"
#include "ScoreBoard.h"
#include "midi-controller/MidiController.h"

//------------------------------------------------------------------------------
struct Texture
{
	DirectX::XMVECTOR origin;
	int width	= 0;
	int height = 0;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

	HRESULT CreateFromFile(ID3D11Device* d3dDevice, const wchar_t* fileName);
};

//------------------------------------------------------------------------------
struct AppResources
{
	DX::StepTimer m_timer;

	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	DirectX::Keyboard::KeyboardStateTracker kbTracker;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::Mouse::ButtonStateTracker mouseTracker;

	std::unique_ptr<DirectX::AudioEngine> audioEngine;
	midi::MidiController midiController;
	midi::MidiControllerTracker midiTracker;

	std::unique_ptr<DirectX::CommonStates> m_states;
	Texture starTexture;
	Texture explosionTexture;
	Texture shotTexture;

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> font8pt;
	std::unique_ptr<DirectX::SpriteFont> font16pt;
	std::unique_ptr<DirectX::SpriteFont> font32pt;
	std::unique_ptr<DirectX::SpriteFont> fontMono8pt;
	std::unique_ptr<DirectX::SpriteFont> fontMono16pt;
	std::unique_ptr<DirectX::SpriteFont> fontMono32pt;

	std::unique_ptr<DX::DebugBatchType> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_debugInputLayout;
	std::unique_ptr<DirectX::BasicEffect> m_debugEffect;

	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	std::shared_ptr<DirectX::BasicEffect> m_debugBoundEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_debugBoundInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive> m_debugBound;

	std::unique_ptr<StarField> starField;
	std::unique_ptr<Explosions> explosions;
	std::unique_ptr<MenuManager> menuManager;
	std::unique_ptr<ScoreBoard> scoreBoard;

	std::map<ModelResource, std::wstring> modelLocations;
	std::map<ModelResource, ModelData> modelData;

	std::map<AudioResource, std::wstring> soundEffectLocations;
	std::map<AudioResource, std::unique_ptr<DirectX::SoundEffect>> soundEffects;

	AppResources()
			: m_keyboard(std::make_unique<DirectX::Keyboard>())
			, m_mouse(std::make_unique<DirectX::Mouse>())
	{
	}
};

//------------------------------------------------------------------------------
