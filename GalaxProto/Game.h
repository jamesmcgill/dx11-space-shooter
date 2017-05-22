//
// Game.h
//

#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include "StepTimer.h"
#include "Starfield.h"
#include "Entity.h"
#include <map>

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:
	Game();

	// Initialization and management
	void Initialize(HWND window, int width, int height);

	// Basic game loop
	void Tick();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);

	// Properties
	void GetDefaultSize(int& width, int& height) const;

private:
	void Update(DX::StepTimer const& timer);
	void HandleInput(DX::StepTimer const& timer);
	void Render();

	void renderEntity(Entity& entity, ID3D11DeviceContext* context);

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	// Device resources.
	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<StarField> m_starField;

	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	std::shared_ptr<DirectX::BasicEffect> m_debugBoundEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_debugBoundInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive> m_debugBound;

	std::unique_ptr<DirectX::Model> m_shotModel;
	DirectX::BoundingSphere m_shotBound;

	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;
	float m_rotationRadiansPS = 0.0f;
	float m_cameraRotationX		= 0.0f;
	float m_cameraRotationY		= 0.0f;
	float m_cameraDist				= 1.0f;

	static const size_t NUM_PLAYERS = 1;
	static const size_t PLAYERS_IDX = 0;
	static const size_t PLAYERS_END = PLAYERS_IDX + NUM_PLAYERS;

	static const size_t NUM_SHOTS = 10;
	static const size_t SHOTS_IDX = PLAYERS_END;
	static const size_t SHOTS_END = SHOTS_IDX + NUM_SHOTS;

	static const size_t NUM_ENEMIES = 10;
	static const size_t ENEMIES_IDX = SHOTS_END;
	static const size_t ENEMIES_END = ENEMIES_IDX + NUM_ENEMIES;

	static const size_t NUM_ENTITIES = ENEMIES_END;
	Entity m_entities[NUM_ENTITIES];

	std::map<char*, wchar_t*> m_modelLocations;
	std::map<char*, ModelData> m_modelData;

	DirectX::SimpleMath::Vector3 m_playerAccel = {};

	// Rendering loop timer.
	DX::StepTimer m_timer;
};
