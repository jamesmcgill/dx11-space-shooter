#pragma once
#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "Entity.h"		 // ModelData
#include "Starfield.h"
#include "Explosions.h"
#include "MenuManager.h"
#include "ScoreBoard.h"
//------------------------------------------------------------------------------
enum class ModelResource
{
	Player,
	Shot,
	Enemy1,
	Enemy2,
	Enemy3,
	Enemy4,
	Enemy5,
	Enemy6,
	Enemy7,
	Enemy8,
	Enemy9,
};

enum class AudioResource
{
	GameStart,
	GameOver,
	PlayerShot,
	EnemyShot,
	PlayerExplode,
	EnemyExplode,
};

//------------------------------------------------------------------------------
struct AppResources
{
	int m_screenWidth	= 0;
	int m_screenHeight = 0;

	DX::StepTimer m_timer;

	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	DirectX::Keyboard::KeyboardStateTracker kbTracker;
	std::unique_ptr<DirectX::AudioEngine> audioEngine;

	std::unique_ptr<DirectX::CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_starTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_explosionTexture;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> font8pt;
	std::unique_ptr<DirectX::SpriteFont> font32pt;

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
	{
	}
};

//------------------------------------------------------------------------------
