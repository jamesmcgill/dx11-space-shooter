//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DebugDraw.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

//------------------------------------------------------------------------------
constexpr float ROTATION_DEGREES_PER_SECOND = 45.f;
constexpr float CAMERA_SPEED_X							= 1.0f;
constexpr float CAMERA_SPEED_Y							= 1.0f;

constexpr float PLAYER_SPEED				= 200.0f;
constexpr float PLAYER_FRICTION			= 60.0f;
constexpr float PLAYER_MAX_VELOCITY = 20.0f;
constexpr float PLAYER_MIN_VELOCITY = 0.3f;

constexpr float CAMERA_DIST = 40.5f;

//------------------------------------------------------------------------------
Game::Game()
		: m_keyboard(std::make_unique<Keyboard>())
		, m_kbTracker(std::make_unique<Keyboard::KeyboardStateTracker>())
		, m_rotationRadiansPS(XMConvertToRadians(ROTATION_DEGREES_PER_SECOND))
		, m_gameMaster(m_state)
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);

	m_modelLocations["PLAYER"] = L"assets/player.sdkmesh";
	m_modelLocations["SHOT"]	 = L"assets/shot.sdkmesh";

	const Vector3 PLAYER_START_POS(0.0f, -0.3f, 0.0f);

	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_state.entities[i].isAlive	= true;
		m_state.entities[i].position = PLAYER_START_POS;
	}
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

	// float elapsedTimeS = float(timer.GetElapsedSeconds());
	// float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());

	// CAMERA
	auto& player = m_state.entities[PLAYERS_IDX];
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
								m_state.entities[0].m_position + m_modelBound.Center);

#else

	performPhysicsUpdate(timer);
	performCollisionTests();

	m_gameMaster.Update(timer);

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
	m_kbTracker->Update(kbState);

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

	if (
		m_kbTracker->IsKeyPressed(Keyboard::LeftControl)
		|| m_kbTracker->IsKeyPressed(Keyboard::Space))
	{
		m_gameMaster.emitPlayerShot();
	}
}

//------------------------------------------------------------------------------
void
Game::performPhysicsUpdate(DX::StepTimer const& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());

	auto& player = m_state.entities[PLAYERS_IDX];
	m_playerAccel *= PLAYER_SPEED;
	const Vector3 innertAccel = {};

	// Friction
	Vector3 frictionNormal = -player.velocity;
	frictionNormal.Normalize();
	m_playerAccel += PLAYER_FRICTION * frictionNormal;

	for (size_t i = 0; i < NUM_ENTITIES; ++i)
	{
		const bool isPlayer = (i < PLAYERS_END);
		auto& e							= m_state.entities[i];
		e.isColliding				= false;

		// Integrate Position
		const Vector3& accel = (isPlayer) ? m_playerAccel : innertAccel;
		e.position					 = 0.5f * accel * (elapsedTimeS * elapsedTimeS)
								 + e.velocity * elapsedTimeS + e.position;
		e.velocity = accel * elapsedTimeS + e.velocity;

		if (isPlayer) {
			// Clamp velocity
			float velocityMagnitude = e.velocity.Length();
			if (velocityMagnitude > PLAYER_MAX_VELOCITY) {
				e.velocity.Normalize();
				e.velocity *= PLAYER_MAX_VELOCITY;
			}
			else if (velocityMagnitude < PLAYER_MIN_VELOCITY)
			{
				e.velocity = Vector3();
			}
		}
	}
}

//------------------------------------------------------------------------------
void
Game::performCollisionTests()
{
	auto& player = m_state.entities[PLAYERS_IDX];

	// Pass 1 - Player				-> EnemyShots
	collisionTestEntity(player, ENEMY_SHOTS_IDX, ENEMY_SHOTS_END);

	// Pass 2 - Player				-> Enemies
	collisionTestEntity(player, ENEMIES_IDX, ENEMIES_END);

	// Pass 3 - PlayerShots		-> Enemies
	for (size_t srcIdx = PLAYER_SHOTS_IDX; srcIdx < PLAYER_SHOTS_END; ++srcIdx)
	{
		auto& srcEntity = m_state.entities[srcIdx];
		collisionTestEntity(srcEntity, ENEMIES_IDX, ENEMIES_END);
	}
}

//------------------------------------------------------------------------------
void
Game::collisionTestEntity(
	Entity& entity,
	const size_t testRangeStartIdx,
	const size_t testRangeOnePastEndIdx)
{
	if (!entity.isAlive) {
		return;
	}

	// TODO(James): Use the GCL <notnullable> to compile time enforce assertion
	assert(entity.model);
	auto& srcBound = entity.model->bound;
	auto srcCenter = entity.position + srcBound.Center;

	for (size_t testIdx = testRangeStartIdx; testIdx < testRangeOnePastEndIdx;
			 ++testIdx)
	{
		assert(m_state.entities[testIdx].model);
		auto& testEntity = m_state.entities[testIdx];
		if (!testEntity.isAlive) {
			continue;
		}

		auto& testBound = testEntity.model->bound;
		auto testCenter = testEntity.position + testBound.Center;

		auto distance = (srcCenter - testCenter).Length();
		if (distance <= (srcBound.Radius + testBound.Radius)) {
			entity.isColliding		 = true;
			testEntity.isColliding = true;
		}
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

	for (auto& entity : m_state.entities)
	{
		if (entity.isAlive) {
			renderEntity(entity);
		}
	}

	// Debug Drawing
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);
	context->RSSetState(m_states->CullNone());

	m_debugEffect->SetView(m_view);
	m_debugEffect->SetProjection(m_proj);
	m_debugEffect->Apply(context);
	context->IASetInputLayout(m_debugInputLayout.Get());

	m_batch->Begin();
	for (auto& entity : m_state.entities)
	{
		if (entity.isAlive) {
			renderEntityBound(entity);
		}
	}

	Waypoint ways[] = {{Vector3(10.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f)},
										 {Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 10.0f, 0.0f)}};

	DX::DrawCurve(
		m_batch.get(), ways[0].wayPoint, ways[1].wayPoint, ways[1].controlPoint);

	m_batch->End();

	m_deviceResources->PIXEndEvent();

	// Show the new frame.
	m_deviceResources->Present();
}

//------------------------------------------------------------------------------
void
Game::renderEntity(Entity& entity)
{
	// TODO(James): Use <notnullable> to enforce assertion
	assert(entity.model);
	const auto& modelData		= entity.model;
	const auto& boundCenter = entity.model->bound.Center;

	Matrix world = Matrix::CreateTranslation(boundCenter).Invert()
								 * Matrix::CreateFromYawPitchRoll(XM_PI, -XM_2PI, XM_PI)
								 * Matrix::CreateTranslation(entity.position + boundCenter);

	modelData->model->Draw(
		m_deviceResources->GetD3DDeviceContext(), *m_states, world, m_view, m_proj);

#if 0
	// DEBUG BOUND
	Matrix boundWorld = Matrix::CreateTranslation(entity.position + boundCenter);
	boundWorld.m[0][0] *= modelData->bound.Radius;
	boundWorld.m[1][1] *= modelData->bound.Radius;
	boundWorld.m[2][2] *= modelData->bound.Radius;

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
#endif
}

//------------------------------------------------------------------------------
void
Game::renderEntityBound(Entity& entity)
{
	// TODO(James): Use <notnullable> to enforce assertion
	assert(entity.model);
	// const auto& modelData = entity.model;

	auto bound	 = entity.model->bound;
	bound.Center = bound.Center + entity.position;
	DX::Draw(
		m_batch.get(), bound, (entity.isColliding) ? Colors::Red : Colors::Green);

	// Matrix world = Matrix::CreateTranslation(entity.position + boundCenter);
	// world.m[0][0] *= modelData->bound.Radius;
	// world.m[1][1] *= modelData->bound.Radius;
	// world.m[2][2] *= modelData->bound.Radius;

	// DEBUG BOUND
	// Vector4 color = entity.isColliding ? Vector4(1.0f, 0.0f, 0.0f, 0.4f)
	//																	 : Vector4(0.0f, 1.0f, 0.0f, 0.4f);
	// m_debugEffect->SetColorAndAlpha(color);
	// m_debugBoundEffect->SetWorld(world);

	// m_debugBound->Draw(
	//	m_debugBoundEffect.get(), m_debugBoundInputLayout.Get(), true, true);
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
	m_debugEffect = std::make_unique<BasicEffect>(device);
	m_debugEffect->SetVertexColorEnabled(true);

	m_effectFactory = std::make_unique<EffectFactory>(device);

	IEffectFactory::EffectInfo info;
	info.ambientColor	= {0.0f, 1.0f, 0.0f};
	m_debugBoundEffect = std::static_pointer_cast<BasicEffect>(
		m_effectFactory->CreateEffect(info, context));
	m_debugBoundEffect->SetColorAndAlpha({0.0f, 1.0f, 0.0f, 0.4f});
	// m_debugBoundEffect->SetLightingEnabled(false);

	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	DX::ThrowIfFailed(CreateDDSTextureFromFile(
		device, L"assets/star.dds", nullptr, m_texture.ReleaseAndGetAddressOf()));
	m_starField = std::make_unique<StarField>(m_texture.Get());

	m_batch = std::make_unique<PrimitiveBatchType>(context);
	{
		void const* shaderByteCode;
		size_t byteCodeLength;
		m_debugEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(device->CreateInputLayout(
			VertexPositionColor::InputElements,
			VertexPositionColor::InputElementCount,
			shaderByteCode,
			byteCodeLength,
			m_debugInputLayout.ReleaseAndGetAddressOf()));
	}

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
		m_state.entities[i].model = &m_modelData["PLAYER"];
	}
	for (size_t i = PLAYER_SHOTS_IDX; i < PLAYER_SHOTS_END; ++i)
	{
		m_state.entities[i].model = &m_modelData["SHOT"];
	}
	for (size_t i = ENEMY_SHOTS_IDX; i < ENEMY_SHOTS_END; ++i)
	{
		m_state.entities[i].model = &m_modelData["SHOT"];
	}
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_state.entities[i].model = &m_modelData["PLAYER"];
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
	m_debugEffect.reset();

	m_starField.reset();
	m_batch.reset();
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
