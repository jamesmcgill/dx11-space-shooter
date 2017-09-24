#include "pch.h"
#include "Game.h"
#include "DebugDraw.h"

#define ENABLE_TRACE
#include "utils/Log.h"

//------------------------------------------------------------------------------
extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

//------------------------------------------------------------------------------
static const std::wstring MODEL_PATH = L"assets/";
static const std::wstring AUDIO_PATH = L"assets/audio/";

//------------------------------------------------------------------------------
Game::Game()
		: m_gameLogic(m_appContext, m_appResources)
		, m_appStates(m_appContext, m_appResources, m_gameLogic)
{
	TRACE();

	m_appResources.m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_appResources.m_deviceResources->RegisterDeviceNotify(this);

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_appResources.audioEngine = std::make_unique<AudioEngine>(eflags);
	m_appResources.audioEngine->SetMasterVolume(0.5f);

	m_appResources.midiController.loadAndInitDll();
	midi::g_onControllerEvent
		= [& tracker = m_appResources.midiTracker](int controllerId, int value)
	{
		tracker.onEvent(controllerId, value);
	};

	// Setup Resource Names
	auto setModelPath = [&](ModelResource res, wchar_t* path) {
		m_appResources.modelLocations[res] = MODEL_PATH + path;
	};
	setModelPath(ModelResource::Player, L"player.sdkmesh");
	setModelPath(ModelResource::Shot, L"shot.sdkmesh");
	setModelPath(ModelResource::Enemy1, L"ship1.sdkmesh");
	setModelPath(ModelResource::Enemy2, L"ship2.sdkmesh");
	setModelPath(ModelResource::Enemy3, L"ship3.sdkmesh");
	setModelPath(ModelResource::Enemy4, L"ship4.sdkmesh");
	setModelPath(ModelResource::Enemy5, L"ship5.sdkmesh");
	setModelPath(ModelResource::Enemy6, L"ship6.sdkmesh");
	setModelPath(ModelResource::Enemy7, L"ship7.sdkmesh");
	setModelPath(ModelResource::Enemy8, L"ship8.sdkmesh");
	setModelPath(ModelResource::Enemy9, L"ship9.sdkmesh");

	auto setAudioPath = [&](AudioResource res, wchar_t* path) {
		m_appResources.soundEffectLocations[res] = AUDIO_PATH + path;
	};
	setAudioPath(AudioResource::GameStart, L"begin.wav");
	setAudioPath(AudioResource::GameOver, L"gameover.wav");
	setAudioPath(AudioResource::PlayerShot, L"playershot.wav");
	setAudioPath(AudioResource::EnemyShot, L"enemyshot.wav");
	setAudioPath(AudioResource::PlayerExplode, L"playerexplode.wav");
	setAudioPath(AudioResource::EnemyExplode, L"enemyexplode.wav");

	const Vector3 PLAYER_START_POS(0.0f, -0.3f, 0.0f);
	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_appContext.entities[i].isAlive	= true;
		m_appContext.entities[i].position = PLAYER_START_POS;
	}
}

//------------------------------------------------------------------------------
Game::~Game()
{
	TRACE();
	if (m_appResources.audioEngine)
	{
		m_appResources.audioEngine->Reset();
	}
}

//------------------------------------------------------------------------------
// Initialize the Direct3D resources required to run.
//------------------------------------------------------------------------------
void
Game::initialize(HWND window, int width, int height)
{
	TRACE();
	m_appResources.m_deviceResources->SetWindow(window, width, height);

	m_appResources.m_deviceResources->CreateDeviceResources();
	createDeviceDependentResources();

	m_appResources.m_deviceResources->CreateWindowSizeDependentResources();
	createWindowSizeDependentResources();

	// TODO: Change the timer settings if you want something other than the
	// default variable timestep mode. e.g. for 60 FPS fixed timestep update
	// logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/

	m_appStates.changeState(&m_appStates.playing);
}

#pragma region Frame Update
//------------------------------------------------------------------------------
// Executes the basic game loop.
//------------------------------------------------------------------------------
void
Game::tick()
{
	m_appResources.m_timer.Tick([&]() { update(); });

	render();
}

//------------------------------------------------------------------------------
void
Game::update()
{
	auto kbState = m_appResources.m_keyboard->GetState();
	m_appResources.kbTracker.Update(kbState);
	if (m_appResources.kbTracker.IsKeyPressed(Keyboard::F1))
	{
		m_appContext.debugDraw = !m_appContext.debugDraw;
	}

	m_appResources.audioEngine->Update();

	const auto& currentState = m_appStates.currentState();
	currentState->handleInput(m_appResources.m_timer);
	currentState->update(m_appResources.m_timer);
}

//------------------------------------------------------------------------------
#pragma endregion

#pragma region Frame Render

//------------------------------------------------------------------------------
// Draws the scene.
//------------------------------------------------------------------------------
void
Game::render()
{
	// Don't try to render anything before the first Update.
	if (m_appResources.m_timer.GetFrameCount() == 0)
	{
		return;
	}

	clear();

	m_appResources.m_deviceResources->PIXBeginEvent(L"Render");
	m_appStates.currentState()->render();
	if (m_appContext.debugDraw)
	{
		drawGlobalDebugInfo();
	}
	m_appResources.m_deviceResources->PIXEndEvent();

	m_appResources.m_deviceResources->Present();
}

//------------------------------------------------------------------------------
void
Game::drawGlobalDebugInfo()
{
	float yPos		= 0.0f;
	auto formatUI = [
		&yPos,
		font				= m_appResources.font8pt.get(),
		screenWidth = m_appResources.m_screenWidth
	](UIText & ui, const wchar_t* fmt, auto&&... vars)
	{
		ui.font							= font;
		ui.text							= fmt::format(fmt, vars...);
		XMVECTOR dimensions = ui.font->MeasureString(ui.text.c_str());
		float height				= XMVectorGetY(dimensions);
		ui.origin						= Vector2(0.0f, 0.0f);
		ui.dimensions				= dimensions;
		ui.position.x				= 0.0f;
		ui.position.y				= yPos;
		yPos += height;
	};
	formatUI(
		m_appContext.uiFrameRate,
		L"fps: {}, Time: {:.2f}ms",
		m_appResources.m_timer.GetFramesPerSecond(),
		m_appResources.m_timer.GetElapsedSecondsSinceTickStarted() * 1000.0f);

	auto drawUI =
		[&font = m_appResources.font8pt,
		&spriteBatch = m_appResources.m_spriteBatch](UIText & ui)
	{
		ui.draw(Colors::MediumVioletRed, *spriteBatch);
	};
	m_appResources.m_spriteBatch->Begin();
	drawUI(m_appContext.uiFrameRate);
	m_appResources.m_spriteBatch->End();
}

//------------------------------------------------------------------------------
// Helper method to clear the back buffers.
//------------------------------------------------------------------------------
void
Game::clear()
{
	m_appResources.m_deviceResources->PIXBeginEvent(L"Clear");

	// Clear the views.
	auto context			= m_appResources.m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_appResources.m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_appResources.m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, Colors::Black);
	context->ClearDepthStencilView(
		depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	auto viewport = m_appResources.m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	m_appResources.m_deviceResources->PIXEndEvent();
}

//------------------------------------------------------------------------------
#pragma endregion

#pragma region Message Handlers
//------------------------------------------------------------------------------
// Message handlers
//------------------------------------------------------------------------------
void
Game::onActivated()
{
	TRACE();
	// TODO: Game is becoming active window.
}

//------------------------------------------------------------------------------
void
Game::onDeactivated()
{
	TRACE();
	// TODO: Game is becoming background window.
}

//------------------------------------------------------------------------------
void
Game::onSuspending()
{
	TRACE();
	// TODO: Game is being power-suspended (or minimized).
}

//------------------------------------------------------------------------------
void
Game::onResuming()
{
	TRACE();
	m_appResources.m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

//------------------------------------------------------------------------------
void
Game::onWindowSizeChanged(int width, int height)
{
	TRACE();
	if (!m_appResources.m_deviceResources->WindowSizeChanged(width, height))
		return;

	createWindowSizeDependentResources();

	// TODO: Game window is being resized.
}

//------------------------------------------------------------------------------
// Properties
//------------------------------------------------------------------------------
void
Game::getDefaultSize(int& width, int& height) const
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	width	= 1280;
	height = 720;
}

//------------------------------------------------------------------------------
#pragma endregion

#pragma region Direct3D Resources
//------------------------------------------------------------------------------
// These are the resources that depend on the device.
//------------------------------------------------------------------------------
void
Game::createDeviceDependentResources()
{
	TRACE();
	auto device	= m_appResources.m_deviceResources->GetD3DDevice();
	auto context = m_appResources.m_deviceResources->GetD3DDeviceContext();

	m_appResources.m_states = std::make_unique<CommonStates>(
		m_appResources.m_deviceResources->GetD3DDevice());
	m_appResources.m_debugEffect = std::make_unique<BasicEffect>(device);
	m_appResources.m_debugEffect->SetVertexColorEnabled(true);

	m_appResources.m_effectFactory = std::make_unique<EffectFactory>(device);

	IEffectFactory::EffectInfo info;
	info.ambientColor									= {0.0f, 1.0f, 0.0f};
	m_appResources.m_debugBoundEffect = std::static_pointer_cast<BasicEffect>(
		m_appResources.m_effectFactory->CreateEffect(info, context));
	m_appResources.m_debugBoundEffect->SetColorAndAlpha({0.0f, 1.0f, 0.0f, 0.4f});
	// m_debugBoundEffect->SetLightingEnabled(false);

	m_appResources.m_spriteBatch = std::make_unique<SpriteBatch>(context);

	DX::ThrowIfFailed(CreateDDSTextureFromFile(
		device,
		L"assets/star.dds",
		nullptr,
		m_appResources.m_starTexture.ReleaseAndGetAddressOf()));
	m_appResources.starField
		= std::make_unique<StarField>(m_appResources.m_starTexture.Get());

	DX::ThrowIfFailed(CreateDDSTextureFromFile(
		device,
		L"assets/explosion.dds",
		nullptr,
		m_appResources.m_explosionTexture.ReleaseAndGetAddressOf()));
	m_appResources.explosions = std::make_unique<Explosions>(
		m_appContext, m_appResources.m_explosionTexture.Get());

	m_appResources.menuManager = std::make_unique<MenuManager>();
	m_appResources.scoreBoard	= std::make_unique<ScoreBoard>();
	m_appResources.scoreBoard->loadFromFile();

	m_appResources.font8pt
		= std::make_unique<SpriteFont>(device, L"assets/verdana8.spritefont");

	m_appResources.font32pt
		= std::make_unique<SpriteFont>(device, L"assets/verdana32.spritefont");

	m_appResources.m_batch = std::make_unique<DX::DebugBatchType>(context);
	{
		void const* shaderByteCode;
		size_t byteCodeLength;
		m_appResources.m_debugEffect->GetVertexShaderBytecode(
			&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(device->CreateInputLayout(
			VertexPositionColor::InputElements,
			VertexPositionColor::InputElementCount,
			shaderByteCode,
			byteCodeLength,
			m_appResources.m_debugInputLayout.ReleaseAndGetAddressOf()));
	}

	m_appResources.m_debugBound = GeometricPrimitive::CreateSphere(context, 2.0f);
	m_appResources.m_debugBound->CreateInputLayout(
		m_appResources.m_debugBoundEffect.get(),
		&m_appResources.m_debugBoundInputLayout);

	// Load the models
	for (const auto& res : m_appResources.modelLocations)
	{
		auto& data = m_appResources.modelData[res.first];
		auto& path = res.second;
		data.model = Model::CreateFromSDKMESH(
			device, path.c_str(), *m_appResources.m_effectFactory);
		data.bound				= {};
		data.bound.Radius = 0.0f;
		for (const auto& mesh : data.model->meshes)
		{
			BoundingSphere::CreateMerged(
				data.bound, mesh->boundingSphere, data.bound);
		}
	}

	// Load the audio effects
	for (const auto& res : m_appResources.soundEffectLocations)
	{
		auto& effect = m_appResources.soundEffects[res.first];
		auto& path	 = res.second;
		effect			 = std::make_unique<SoundEffect>(
			m_appResources.audioEngine.get(), path.c_str());
	}

	// TODO(James): Critical these are not null for any entity. <NOT_NULLABLE>?
	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_appContext.entities[i].model
			= &m_appResources.modelData[ModelResource::Player];
	}
	for (size_t i = PLAYER_SHOTS_IDX; i < PLAYER_SHOTS_END; ++i)
	{
		m_appContext.entities[i].model
			= &m_appResources.modelData[ModelResource::Shot];
	}
	for (size_t i = ENEMY_SHOTS_IDX; i < ENEMY_SHOTS_END; ++i)
	{
		m_appContext.entities[i].model
			= &m_appResources.modelData[ModelResource::Shot];
	}
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_appContext.entities[i].model
			= &m_appResources.modelData[ModelResource::Enemy1];
	}
}

//------------------------------------------------------------------------------
// Allocate all memory resources that change on a window SizeChanged event.
//------------------------------------------------------------------------------
void
Game::createWindowSizeDependentResources()
{
	TRACE();
	RECT outputSize = m_appResources.m_deviceResources->GetOutputSize();

	const float fovAngleY = 30.0f * XM_PI / 180.0f;

	float aspectRatio = float(outputSize.right - outputSize.left)
											/ (outputSize.bottom - outputSize.top);

	m_appContext.view = Matrix::Identity;
	m_appContext.proj = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY, aspectRatio, 0.01f, 300.f);

	m_appResources.starField->setWindowSize(outputSize.right, outputSize.bottom);
	m_appResources.explosions->setWindowSize(outputSize.right, outputSize.bottom);
	m_appResources.menuManager->setWindowSize(
		outputSize.right, outputSize.bottom);
	m_appResources.scoreBoard->setWindowSize(outputSize.right, outputSize.bottom);

	m_appResources.m_screenWidth	= outputSize.right;
	m_appResources.m_screenHeight = outputSize.bottom;

	// Position HUD
	m_gameLogic.updateUILives();
	m_gameLogic.updateUIScore();
	m_gameLogic.updateUIDebugVariables();

	m_appContext.updateViewMatrix();
}

//------------------------------------------------------------------------------
void
Game::OnDeviceLost()
{
	TRACE();
	for (auto& modelData : m_appResources.modelData)
	{
		modelData.second.model.reset();
	}

	m_appResources.m_debugBound.reset();
	m_appResources.m_debugBoundInputLayout.Reset();
	m_appResources.m_debugBoundEffect.reset();
	m_appResources.m_effectFactory.reset();
	m_appResources.m_debugEffect.reset();

	m_appResources.font32pt.reset();
	m_appResources.font8pt.reset();
	m_appResources.starField.reset();
	m_appResources.explosions.reset();
	m_appResources.scoreBoard.reset();
	m_appResources.menuManager.reset();

	m_appResources.m_batch.reset();
	m_appResources.m_spriteBatch.reset();
	m_appResources.m_explosionTexture.Reset();
	m_appResources.m_starTexture.Reset();
	m_appResources.m_states.reset();
}

//------------------------------------------------------------------------------
void
Game::OnDeviceRestored()
{
	TRACE();
	createDeviceDependentResources();

	createWindowSizeDependentResources();
}

//------------------------------------------------------------------------------
#pragma endregion
