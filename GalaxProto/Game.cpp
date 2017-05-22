//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

//------------------------------------------------------------------------------
constexpr float ROTATION_DEGREES_PER_SECOND = 45.f;
constexpr float CAMERA_SPEED_X							= 1.0f;
constexpr float CAMERA_SPEED_Y							= 1.0f;

//------------------------------------------------------------------------------
Game::Game()
		: m_keyboard(std::make_unique<Keyboard>())
		, m_rotationRadiansPS(XMConvertToRadians(ROTATION_DEGREES_PER_SECOND))
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);

	m_modelLocations["PLAYER"] = L"assets/player.sdkmesh";
	m_modelLocations["SHOT"]	 = L"assets/player.sdkmesh";

	const Vector3 PLAYER_START_POS(0.0f, -0.3f, 0.0f);
	const Vector3 ENEMY_START_POS(0.0f, 0.3f, 0.0f);

	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_entities[i].isAlive	= true;
		m_entities[i].position = PLAYER_START_POS;
	}
	for (size_t i = SHOTS_IDX; i < SHOTS_END; ++i)
	{
	}
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_entities[i].position = ENEMY_START_POS;
	}

	// TEMP, activate one enemy
	m_entities[ENEMIES_IDX].isAlive = true;
}

// Initialize the Direct3D resources required to run.
void
Game::Initialize(HWND window, int width, int height)
{
	m_deviceResources->SetWindow(window, width, height);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	// TODO: Change the timer settings if you want something other than the
	// default variable timestep mode. e.g. for 60 FPS fixed timestep update
	// logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

#pragma region Frame Update
// Executes the basic game loop.
void
Game::Tick()
{
	m_timer.Tick([&]() { Update(m_timer); });

	Render();
}

//------------------------------------------------------------------------------
// Updates the world.
//------------------------------------------------------------------------------
void
Game::Update(DX::StepTimer const& timer)
{
	HandleInput(timer);

	float elapsedTimeS = float(timer.GetElapsedSeconds());
	// float totalTimeS = static_cast<float>(timer.GetTotalSeconds());

	// CAMERA
	const float CAMERA_DIST = 40.5f;

	auto& player = m_entities[PLAYERS_IDX];
	assert(player.model);
	const auto& atP							 = player.position + player.model->bound.Center;
	static const XMVECTORF32 eye = {atP.x, atP.y, atP.z + CAMERA_DIST, 0.0f};
	static const XMVECTORF32 at	= {atP.x, atP.y, atP.z, 0.0f};
	static const XMVECTORF32 up	= {0.0f, 1.0f, 0.0f, 0.0f};

	XMVECTOR eyePos = ::XMVectorSubtract(eye, at);

	float radiansX = static_cast<float>(fmod(m_cameraRotationX, XM_2PI));
	eyePos				 = ::XMVector3Rotate(
		eyePos, XMQuaternionRotationMatrix(XMMatrixRotationX(radiansX)));

	float radiansY = static_cast<float>(fmod(m_cameraRotationY, XM_2PI));
	eyePos				 = ::XMVector3Rotate(
		eyePos, XMQuaternionRotationMatrix(XMMatrixRotationY(radiansY)));

	eyePos = ::XMVectorAdd(eyePos, at);

	m_view = XMMatrixLookAtRH(eyePos, at, up);

#if 0
	// Implicit Model Rotation
	double totalRotation = totalTimeS * m_rotationRadiansPS;
	float radians				 = static_cast<float>(fmod(totalRotation, XM_2PI));

	m_world = Matrix::CreateTranslation(m_modelBound.Center).Invert()
						* Matrix::CreateFromYawPitchRoll(XM_PI - 0.5f, radians, -XM_PI / 2)
						* Matrix::CreateTranslation(
								m_entities[0].m_position + m_modelBound.Center);

#else
	const float PLAYER_SPEED				= 20.0f;
	const float PLAYER_FRICTION			= 6.0f;
	const float PLAYER_MAX_VELOCITY = 2.7f;
	const float PLAYER_MIN_VELOCITY = 0.3f;
	m_playerAccel *= PLAYER_SPEED;

	// Friction
	Vector3 frictionNormal = -player.velocity;
	frictionNormal.Normalize();
	m_playerAccel += PLAYER_FRICTION * frictionNormal;

	player.position = 0.5f * m_playerAccel * (elapsedTimeS * elapsedTimeS)
										+ player.velocity * elapsedTimeS + player.position;
	player.velocity = m_playerAccel * elapsedTimeS + player.velocity;

	// Clamp velocity
	float velocityMagnitude = player.velocity.Length();
	if (velocityMagnitude > PLAYER_MAX_VELOCITY) {
		player.velocity.Normalize();
		player.velocity *= PLAYER_MAX_VELOCITY;
	}
	else if (velocityMagnitude < PLAYER_MIN_VELOCITY)
	{
		player.velocity = Vector3();
	}

	// Collision
	for (size_t srcIdx = 0; srcIdx < NUM_ENTITIES; ++srcIdx)
	{
		m_entities[srcIdx].isColliding = false;
	}

	for (size_t srcIdx = 0; srcIdx < NUM_ENTITIES; ++srcIdx)
	{
		// TODO(James): Use the GCL <notnullable> to compile time enforce assertion
		assert(m_entities[srcIdx].model);
		auto& srcEntity = m_entities[srcIdx];
		if (!srcEntity.isAlive) {
			continue;
		}
		auto& srcBound = srcEntity.model->bound;
		auto srcCenter = srcEntity.position + srcBound.Center;

		for (size_t testIdx = srcIdx + 1; testIdx < NUM_ENTITIES; ++testIdx)
		{
			assert(m_entities[testIdx].model);
			auto& testEntity = m_entities[testIdx];
			if (!testEntity.isAlive) {
				continue;
			}

			auto& testBound = testEntity.model->bound;
			auto testCenter = testEntity.position + testBound.Center;

			auto distance = (srcCenter - testCenter).Length();
			if (distance <= (srcBound.Radius + testBound.Radius)) {
				srcEntity.isColliding	= true;
				testEntity.isColliding = true;
			}
		}
	}

#endif

	m_starField->update(timer);
}

//------------------------------------------------------------------------------
void
Game::HandleInput(DX::StepTimer const& timer)
{
	float elapsedTimeS = static_cast<float>(timer.GetElapsedSeconds());

	// Handle Keyboard Input
	auto kbState = m_keyboard->GetState();
	if (kbState.Escape) {
		ExitGame();
	}

	if (kbState.W) {
		m_cameraRotationX -= elapsedTimeS * CAMERA_SPEED_X;
	}
	else if (kbState.S)
	{
		m_cameraRotationX += elapsedTimeS * CAMERA_SPEED_X;
	}

	if (kbState.A) {
		m_cameraRotationY -= elapsedTimeS * CAMERA_SPEED_Y;
	}
	else if (kbState.D)
	{
		m_cameraRotationY += elapsedTimeS * CAMERA_SPEED_Y;
	}

	m_playerAccel = Vector3();
	if (kbState.Up) {
		m_playerAccel.y = 1.0f;
	}
	else if (kbState.Down)
	{
		m_playerAccel.y = -1.0f;
	}

	if (kbState.Left) {
		m_playerAccel.x = -1.0f;
	}
	else if (kbState.Right)
	{
		m_playerAccel.x = 1.0f;
	}

	if (m_playerAccel.x != 0.0f && m_playerAccel.y != 0.0f) {
		const float UNIT_DIAGONAL_LENGTH = 0.7071067811865475f;
		m_playerAccel *= UNIT_DIAGONAL_LENGTH;
	}
}

//------------------------------------------------------------------------------
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void
Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0) {
		return;
	}

	Clear();

	m_deviceResources->PIXBeginEvent(L"Render");
	auto context = m_deviceResources->GetD3DDeviceContext();
	// auto device = m_deviceResources->GetD3DDevice();

	// TODO: Add your rendering code here.
	m_spriteBatch->Begin();
	m_starField->render(*m_spriteBatch);
	m_spriteBatch->End();

	for (auto& entity : m_entities)
	{
		if (entity.isAlive) {
			renderEntity(entity, context);
		}
	}

	m_deviceResources->PIXEndEvent();

	// Show the new frame.
	m_deviceResources->Present();
}

//------------------------------------------------------------------------------
void
Game::renderEntity(Entity& entity, ID3D11DeviceContext* context)
{
	// TODO(James): Use <notnullable> to enforce assertion
	assert(entity.model);
	const auto& modelData		= entity.model;
	const auto& boundCenter = entity.model->bound.Center;

	Matrix world = Matrix::CreateTranslation(boundCenter).Invert()
								 * Matrix::CreateFromYawPitchRoll(XM_PI, -XM_2PI, XM_PI)
								 * Matrix::CreateTranslation(entity.position + boundCenter);

	Matrix boundWorld = Matrix::CreateTranslation(entity.position + boundCenter);
	boundWorld.m[0][0] *= modelData->bound.Radius;
	boundWorld.m[1][1] *= modelData->bound.Radius;
	boundWorld.m[2][2] *= modelData->bound.Radius;

	modelData->model->Draw(
		m_deviceResources->GetD3DDeviceContext(), *m_states, world, m_view, m_proj);

	// DEBUG BOUND
	Vector4 color = entity.isColliding ? Vector4(1.0f, 0.0f, 0.0f, 0.4f)
																		 : Vector4(0.0f, 1.0f, 0.0f, 0.4f);
	m_debugBoundEffect->SetColorAndAlpha(color);

	m_debugBoundEffect->SetView(m_view);
	m_debugBoundEffect->SetProjection(m_proj);
	m_debugBoundEffect->SetWorld(boundWorld);
	m_debugBoundEffect->Apply(context);

	context->OMSetBlendState(m_states->AlphaBlend(), Colors::White, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullNone());

	m_debugBound->Draw(
		m_debugBoundEffect.get(), m_debugBoundInputLayout.Get(), true, true);
}

//------------------------------------------------------------------------------
// Helper method to clear the back buffers.
void
Game::Clear()
{
	m_deviceResources->PIXBeginEvent(L"Clear");

	// Clear the views.
	auto context			= m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, Colors::Black);
	context->ClearDepthStencilView(
		depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void
Game::OnActivated()
{
	// TODO: Game is becoming active window.
}

void
Game::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void
Game::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void
Game::OnResuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

void
Game::OnWindowSizeChanged(int width, int height)
{
	if (!m_deviceResources->WindowSizeChanged(width, height)) return;

	CreateWindowSizeDependentResources();

	m_starField->setWindowSize(width, height);

	// TODO: Game window is being resized.
}

// Properties
void
Game::GetDefaultSize(int& width, int& height) const
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	width	= 1024;
	height = 768;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void
Game::CreateDeviceDependentResources()
{
	auto device	= m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());

	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	DX::ThrowIfFailed(CreateDDSTextureFromFile(
		device, L"assets/star.dds", nullptr, m_texture.ReleaseAndGetAddressOf()));
	m_starField = std::make_unique<StarField>(m_texture.Get());

	m_effectFactory = std::make_unique<EffectFactory>(device);

	IEffectFactory::EffectInfo info;
	info.ambientColor	= {0.0f, 1.0f, 0.0f};
	m_debugBoundEffect = std::static_pointer_cast<BasicEffect>(
		m_effectFactory->CreateEffect(info, context));
	m_debugBoundEffect->SetColorAndAlpha({0.0f, 1.0f, 0.0f, 0.4f});
	// m_debugBoundEffect->SetLightingEnabled(false);

	m_debugBound = GeometricPrimitive::CreateSphere(context, 2.0f);
	m_debugBound->CreateInputLayout(
		m_debugBoundEffect.get(), &m_debugBoundInputLayout);

	// Load the models
	for (const auto& res : m_modelLocations)
	{
		auto& data = m_modelData[res.first];
		data.model = Model::CreateFromSDKMESH(device, res.second, *m_effectFactory);
		data.bound = {};
		data.bound.Radius = 0.0f;
		for (const auto& mesh : data.model->meshes)
		{
			BoundingSphere::CreateMerged(
				data.bound, mesh->boundingSphere, data.bound);
		}
	}

	// TODO(James): Critical these are not null for any entity. <NOT_NULLABLE>?
	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_entities[i].model = &m_modelData["PLAYER"];
	}
	for (size_t i = SHOTS_IDX; i < SHOTS_END; ++i)
	{
		m_entities[i].model = &m_modelData["SHOT"];
	}
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_entities[i].model = &m_modelData["PLAYER"];
	}
}

// Allocate all memory resources that change on a window SizeChanged event.
void
Game::CreateWindowSizeDependentResources()
{
	RECT outputSize = m_deviceResources->GetOutputSize();

	const float fovAngleY = 30.0f * XM_PI / 180.0f;

	float aspectRatio = float(outputSize.right - outputSize.left)
											/ (outputSize.bottom - outputSize.top);

	m_view = Matrix::Identity;
	m_proj = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY, aspectRatio, 0.01f, 100.f);

	m_starField->setWindowSize(outputSize.right, outputSize.bottom);
}

void
Game::OnDeviceLost()
{
	for (auto& modelData : m_modelData)
	{
		modelData.second.model.reset();
	}

	m_debugBound.reset();
	m_debugBoundInputLayout.Reset();
	m_debugBoundEffect.reset();
	m_effectFactory.reset();

	m_starField.reset();
	m_spriteBatch.reset();
	m_texture.Reset();
	m_states.reset();
}

void
Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
