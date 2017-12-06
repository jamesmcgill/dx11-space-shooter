#include "pch.h"
#include "Game.h"
#include "DebugDraw.h"
#include "UIDebugDraw.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
extern void ExitGame();

//------------------------------------------------------------------------------
static const std::wstring MODEL_PATH = L"assets/";
static const std::wstring AUDIO_PATH = L"assets/audio/";

//------------------------------------------------------------------------------
Game::Game()
		: m_gameLogic(m_context, m_resources)
		, m_appStates(m_context, m_resources, m_gameLogic)
{
	TRACE
	m_resources.m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_resources.m_deviceResources->RegisterDeviceNotify(this);

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | DirectX::AudioEngine_Debug;
#endif
	m_resources.audioEngine = std::make_unique<DirectX::AudioEngine>(eflags);
	m_resources.audioEngine->SetMasterVolume(0.5f);

	m_context.isMidiConnected = m_resources.midiController.loadAndInitDll();
	midi::g_onControllerEvent
		= [& tracker = m_resources.midiTracker](int controllerId, int value)
	{
		tracker.onEvent(controllerId, value);
	};

	// Setup Resource Names
	auto setModelPath = [&](ModelResource res, const wchar_t* path) {
		m_resources.modelLocations[res] = MODEL_PATH + path;
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

	auto setAudioPath = [&](AudioResource res, const wchar_t* path) {
		m_resources.soundEffectLocations[res] = AUDIO_PATH + path;
	};
	setAudioPath(AudioResource::GameStart, L"begin.wav");
	setAudioPath(AudioResource::GameOver, L"gameover.wav");
	setAudioPath(AudioResource::PlayerShot, L"playershot.wav");
	setAudioPath(AudioResource::EnemyShot, L"enemyshot.wav");
	setAudioPath(AudioResource::PlayerExplode, L"playerexplode.wav");
	setAudioPath(AudioResource::EnemyExplode, L"enemyexplode.wav");

	const DirectX::SimpleMath::Vector3 PLAYER_START_POS(0.0f, -0.3f, 0.0f);
	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_context.entities[i].isAlive	= true;
		m_context.entities[i].position = PLAYER_START_POS;
	}
}

//------------------------------------------------------------------------------
Game::~Game()
{
	TRACE
	if (m_resources.audioEngine)
	{
		m_resources.audioEngine->Reset();
	}
}

//------------------------------------------------------------------------------
// Initialize the Direct3D resources required to run.
//------------------------------------------------------------------------------
void
Game::initialize(HWND window, int width, int height)
{
	TRACE
	m_resources.m_mouse->SetWindow(window);
	m_resources.m_deviceResources->SetWindow(window, width, height);

	m_resources.m_deviceResources->CreateDeviceResources();
	createDeviceDependentResources();

	m_resources.m_deviceResources->CreateWindowSizeDependentResources();
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
	{
		TRACE
		m_resources.m_timer.Tick([&]() { update(); });

		render();
	}

	ui::DebugDraw ui(m_context, m_resources);
	ui.begin2D();
	switch (m_context.profileViz)
	{
		case ProfileViz::List:
			drawProfilerList();
			break;
		case ProfileViz::FlameGraph:
			drawFlameGraph();
			break;
	}

	m_context.uiControlInfo.draw(*m_resources.m_spriteBatch);
	drawBasicProfileInfo();		 // Do this at the end to include profiler cost
	ui.end2D();

	m_resources.m_deviceResources->Present();

	logger::Stats::signalFrameEnd();
}

//------------------------------------------------------------------------------
void
Game::update()
{
	TRACE
	auto kbState = m_resources.m_keyboard->GetState();
	m_resources.kbTracker.Update(kbState);

	auto mouseState = m_resources.m_mouse->GetState();
	m_resources.mouseTracker.Update(mouseState);

	if (m_resources.kbTracker.IsKeyPressed(DirectX::Keyboard::F1))
	{
		switch (m_context.profileViz)
		{
			case ProfileViz::Basic:
				m_context.profileViz = ProfileViz::List;
				break;
			case ProfileViz::List:
				m_context.profileViz = ProfileViz::FlameGraph;
				break;
			case ProfileViz::FlameGraph:
				m_context.profileViz = ProfileViz::Basic;
				break;
		}
	}
	m_resources.audioEngine->Update();

	const auto& currentState = m_appStates.currentState();
	currentState->handleInput(m_resources.m_timer);
	currentState->update(m_resources.m_timer);
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
	TRACE
	// Don't try to render anything before the first Update.
	if (m_resources.m_timer.GetFrameCount() == 0)
	{
		return;
	}

	clear();

	m_resources.m_deviceResources->PIXBeginEvent(L"Render");
	m_appStates.currentState()->render();
	m_resources.m_deviceResources->PIXEndEvent();
}

//------------------------------------------------------------------------------
void
Game::drawBasicProfileInfo()
{
	ui::Text uiText;
	uiText.font			= m_resources.fontMono8pt.get();
	uiText.position = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
	uiText.color		= DirectX::Colors::MediumVioletRed;
	uiText.text			= fmt::format(
		L"fps: {}, Time: {:.2f}ms",
		m_resources.m_timer.GetFramesPerSecond(),
		m_resources.m_timer.GetElapsedSecondsSinceTickStarted() * 1000.0f);

	uiText.draw(*m_resources.m_spriteBatch);
}

//------------------------------------------------------------------------------
void
Game::drawProfilerList()
{
	auto monoFont = m_resources.fontMono8pt.get();

	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));
	float yPos = yAscent;
	ui::Text uiText;
	uiText.font				= monoFont;
	uiText.position.x = 0.0f;
	uiText.color			= DirectX::Colors::MediumVioletRed;

	auto drawText =
		[&yAscent, &yPos, &uiText, &spriteBatch = m_resources.m_spriteBatch](
			const wchar_t* fmt, auto&&... vars)
	{
		uiText.text				= fmt::format(fmt, vars...);
		uiText.position.y = yPos;
		yPos += yAscent;
		uiText.draw(*spriteBatch);
	};

	auto singleFrame = logger::Stats::getCollatedFrameRecords(0);
	std::vector<std::pair<size_t, logger::CollatedRecord>> sortedRecords;
	for (const auto& rec : singleFrame)
	{
		sortedRecords.push_back(std::make_pair(rec.first, rec.second));
	}
	std::sort(
		sortedRecords.begin(), sortedRecords.end(), [](auto& lhs, auto& rhs) {
			return lhs.second.ticks > rhs.second.ticks;
		});
	auto accumulatedRecords = logger::Stats::accumulateRecords();

	for (auto& entry : sortedRecords)
	{
		auto& record			= entry.second;
		auto& accumRecord = accumulatedRecords[entry.first];
		drawText(
			L"{:<35} ({:>2})h    ({:>5.4f} / {:<5.4f})ms",
			record.function,
			accumRecord.callsCount.average(),
			logger::Timing::ticksToMilliSeconds(accumRecord.ticks.min),
			logger::Timing::ticksToMilliSeconds(accumRecord.ticks.max));
	}
}

//------------------------------------------------------------------------------
void
Game::updateControlsInfo(ui::Text& uiText)
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::XMVECTOR;

	uiText.text
		= L"Profiler Mode(F1), Debug Draw(F2), Editor(F3), WASDR(Camera control)";
	uiText.font			= m_resources.fontMono8pt.get();
	uiText.position = Vector2(m_context.screenHalfWidth, m_context.screenHeight);
	XMVECTOR dimensions = uiText.font->MeasureString(uiText.text.c_str());
	const float width		= DirectX::XMVectorGetX(dimensions);
	const float height	= DirectX::XMVectorGetY(dimensions);
	uiText.origin				= Vector2(width * 0.5f, height);
	uiText.color				= DirectX::Colors::MediumVioletRed;
}

//------------------------------------------------------------------------------
// Typical Breadth First Traversal but with added depth tracking
// to keep track of the current depth in the tree.
//------------------------------------------------------------------------------
template <typename Func>
void
visitFlameGraph(const logger::TimedRecord* head, Func visitFunc)
{
	using RecordQueue = std::queue<const logger::TimedRecord*>;
	ASSERT(head);

	int depth = 0;
	RecordQueue queue;
	RecordQueue nextDepthQueue;

	queue.push(head);
	while (true)
	{
		auto node = queue.front();
		queue.pop();
		visitFunc(node, depth);

		for (const auto c : node->childNodes)
		{
			nextDepthQueue.push(c);
		}

		if (queue.empty())		// Finished this depth level
		{
			if (nextDepthQueue.empty())		 // No more levels left
			{
				return;
			}
			else
			{
				// Setup next depth level
				std::swap(queue, nextDepthQueue);
				depth++;
			}
		}
	}
}

//------------------------------------------------------------------------------
void
Game::drawFlameGraph()
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;

	auto& currentSnapShot						= logger::Stats::getFrameRecords(0);
	const logger::TimedRecord* head = (overriddenFlameHead == nullptr)
																			? currentSnapShot.callGraphHead
																			: overriddenFlameHead;
	if (!head)
	{
		return;
	}

	auto mouseState												 = m_resources.m_mouse->GetState();
	bool isDrawToolTip										 = false;
	const logger::TimedRecord* toolTipNode = nullptr;

	auto monoFont = m_resources.fontMono8pt.get();
	const float yAscent
		= ceil(DirectX::XMVectorGetY(monoFont->MeasureString(L"X")));
	const float xStartPos = 0.0f;
	const float xWidth		= ceil(m_context.screenWidth / 7.0f);
	const float yStartPos = yAscent;
	const float yRange		= m_context.screenHeight - (2 * yAscent);

	const logger::Ticks baseTick = head->startTime;
	const float ticksToYPos			 = yRange / head->duration;

	ui::DebugDraw ui(m_context, m_resources);
	ui::Text uiText;
	uiText.font		= monoFont;
	uiText.origin = Vector2(0.0f, yAscent * 0.5f);
	uiText.color	= DirectX::Colors::White;

	float colLerp													 = 0.0f;
	DirectX::XMVECTORF32 color1ForDepth[2] = {
		DirectX::Colors::Peru,
		DirectX::Colors::SandyBrown,
	};
	DirectX::XMVECTORF32 color2ForDepth[2] = {
		DirectX::Colors::Firebrick,
		DirectX::Colors::OrangeRed,
	};

	auto drawFunc = [&](const logger::TimedRecord* node, int depth) {
		const float xPos = xStartPos + (depth * xWidth);
		const float yPos = yStartPos + ((node->startTime - baseTick) * ticksToYPos);
		const float yHeight			= node->duration * ticksToYPos;
		const float yHalfHeight = yHeight * 0.5f;

		int colorIdx = (depth % 2) == 0 ? 1 : 0;
		colLerp += 0.3f;
		if (colLerp > 1.0f)
		{
			colLerp -= 1.0f;
		}
		Vector3 color = XMVectorLerp(
			color1ForDepth[colorIdx], color2ForDepth[colorIdx], colLerp);
		ui::drawBox(*m_resources.m_batch, xPos, yPos, xWidth - 2, yHeight, color);

		if (
			(mouseState.x >= xPos) && (mouseState.x <= xPos + xWidth)
			&& (mouseState.y >= yPos) && (mouseState.y <= yPos + yHeight))
		{
			isDrawToolTip = true;
			toolTipNode		= node;
		}

		uiText.text			= fmt::format(L"{}", node->function);
		uiText.position = Vector2(ceil(xPos + 5.0f), ceil(yPos + yHalfHeight));
		uiText.draw(*m_resources.m_spriteBatch);
	};

	// Draw the Graph
	visitFlameGraph(head, drawFunc);

	// Draw Tooltip
	if (isDrawToolTip)
	{
		ASSERT(toolTipNode);
		static const float TOOLTIP_HEIGHT = 20.0f;
		ui::drawBox(
			*m_resources.m_batch,
			static_cast<float>(mouseState.x),
			static_cast<float>(mouseState.y) - TOOLTIP_HEIGHT,
			2.0f * xWidth,
			TOOLTIP_HEIGHT,
			DirectX::Colors::DarkRed,
			ui::Layer::L4_Mid_Front);

		uiText.text = fmt::format(
			L"{}({:<5.4f}ms)",
			toolTipNode->function,
			logger::Timing::ticksToMilliSeconds(toolTipNode->duration));
		uiText.position = Vector2(
			mouseState.x + (TOOLTIP_HEIGHT / 2.0f),
			mouseState.y - (TOOLTIP_HEIGHT / 2.0f));
		uiText.layer = ui::Layer::L4_Mid_Front;
		uiText.draw(*m_resources.m_spriteBatch);

		using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
		if (m_resources.mouseTracker.leftButton == ButtonState::PRESSED)
		{
			overriddenFlameHead = toolTipNode;
		}
	}
	if (mouseState.rightButton)
	{
		overriddenFlameHead = nullptr;
	}
}

//------------------------------------------------------------------------------
// Helper method to clear the back buffers.
//------------------------------------------------------------------------------
void
Game::clear()
{
	TRACE
	m_resources.m_deviceResources->PIXBeginEvent(L"Clear");

	// Clear the views.
	auto context			= m_resources.m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_resources.m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_resources.m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, DirectX::Colors::Black);
	context->ClearDepthStencilView(
		depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	auto viewport = m_resources.m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	m_resources.m_deviceResources->PIXEndEvent();
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
	TRACE
	// TODO: Game is becoming active window.
	m_resources.kbTracker.Reset();
	m_resources.mouseTracker.Reset();
}

//------------------------------------------------------------------------------
void
Game::onDeactivated()
{
	TRACE
	// TODO: Game is becoming background window.
}

//------------------------------------------------------------------------------
void
Game::onSuspending()
{
	TRACE
	// TODO: Game is being power-suspended (or minimized).
}

//------------------------------------------------------------------------------
void
Game::onResuming()
{
	TRACE
	m_resources.m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

//------------------------------------------------------------------------------
void
Game::onWindowSizeChanged(int width, int height)
{
	TRACE
	if (!m_resources.m_deviceResources->WindowSizeChanged(width, height))
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
	TRACE
	auto device	= m_resources.m_deviceResources->GetD3DDevice();
	auto context = m_resources.m_deviceResources->GetD3DDeviceContext();

	m_resources.m_states = std::make_unique<DirectX::CommonStates>(
		m_resources.m_deviceResources->GetD3DDevice());
	m_resources.m_debugEffect = std::make_unique<DirectX::BasicEffect>(device);
	m_resources.m_debugEffect->SetVertexColorEnabled(true);

	m_resources.m_effectFactory
		= std::make_unique<DirectX::EffectFactory>(device);

	DirectX::IEffectFactory::EffectInfo info;
	info.ambientColor = {0.0f, 1.0f, 0.0f};
	m_resources.m_debugBoundEffect
		= std::static_pointer_cast<DirectX::BasicEffect>(
			m_resources.m_effectFactory->CreateEffect(info, context));
	m_resources.m_debugBoundEffect->SetColorAndAlpha({0.0f, 1.0f, 0.0f, 0.4f});
	// m_debugBoundEffect->SetLightingEnabled(false);

	m_resources.m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
		device,
		L"assets/star.dds",
		nullptr,
		m_resources.m_starTexture.ReleaseAndGetAddressOf()));
	m_resources.starField
		= std::make_unique<StarField>(m_context, m_resources.m_starTexture.Get());

	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
		device,
		L"assets/explosion.dds",
		nullptr,
		m_resources.m_explosionTexture.ReleaseAndGetAddressOf()));
	m_resources.explosions = std::make_unique<Explosions>(
		m_context, m_resources.m_explosionTexture.Get());

	m_resources.menuManager = std::make_unique<MenuManager>(m_context);
	m_resources.scoreBoard	= std::make_unique<ScoreBoard>(m_context);
	m_resources.scoreBoard->loadFromFile();

	m_resources.font8pt = std::make_unique<DirectX::SpriteFont>(
		device, L"assets/verdana8.spritefont");
	m_resources.font32pt = std::make_unique<DirectX::SpriteFont>(
		device, L"assets/verdana32.spritefont");

	m_resources.fontMono8pt
		= std::make_unique<DirectX::SpriteFont>(device, L"assets/mono8.spritefont");
	m_resources.fontMono32pt = std::make_unique<DirectX::SpriteFont>(
		device, L"assets/mono32.spritefont");

	m_resources.m_batch = std::make_unique<DX::DebugBatchType>(context);
	{
		void const* shaderByteCode;
		size_t byteCodeLength;
		m_resources.m_debugEffect->GetVertexShaderBytecode(
			&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(device->CreateInputLayout(
			DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode,
			byteCodeLength,
			m_resources.m_debugInputLayout.ReleaseAndGetAddressOf()));
	}

	m_resources.m_debugBound
		= DirectX::GeometricPrimitive::CreateSphere(context, 2.0f);
	m_resources.m_debugBound->CreateInputLayout(
		m_resources.m_debugBoundEffect.get(), &m_resources.m_debugBoundInputLayout);

	// Load the models
	for (const auto& res : m_resources.modelLocations)
	{
		auto& data = m_resources.modelData[res.first];
		auto& path = res.second;
		data.model = DirectX::Model::CreateFromSDKMESH(
			device, path.c_str(), *m_resources.m_effectFactory);
		data.bound				= {};
		data.bound.Radius = 0.0f;
		for (const auto& mesh : data.model->meshes)
		{
			DirectX::BoundingSphere::CreateMerged(
				data.bound, mesh->boundingSphere, data.bound);
		}
	}

	// Load the audio effects
	for (const auto& res : m_resources.soundEffectLocations)
	{
		auto& effect = m_resources.soundEffects[res.first];
		auto& path	 = res.second;
		effect			 = std::make_unique<DirectX::SoundEffect>(
			m_resources.audioEngine.get(), path.c_str());
	}

	// TODO(James): Critical these are not null for any entity. <NOT_NULLABLE>?
	for (size_t i = PLAYERS_IDX; i < PLAYERS_END; ++i)
	{
		m_context.entities[i].model = &m_resources.modelData[ModelResource::Player];
	}
	for (size_t i = PLAYER_SHOTS_IDX; i < PLAYER_SHOTS_END; ++i)
	{
		m_context.entities[i].model = &m_resources.modelData[ModelResource::Shot];
	}
	for (size_t i = ENEMY_SHOTS_IDX; i < ENEMY_SHOTS_END; ++i)
	{
		m_context.entities[i].model = &m_resources.modelData[ModelResource::Shot];
	}
	for (size_t i = ENEMIES_IDX; i < ENEMIES_END; ++i)
	{
		m_context.entities[i].model = &m_resources.modelData[ModelResource::Enemy1];
	}
}

//------------------------------------------------------------------------------
// Allocate all memory resources that change on a window SizeChanged event.
//------------------------------------------------------------------------------
void
Game::createWindowSizeDependentResources()
{
	TRACE
	using DirectX::SimpleMath::Matrix;
	RECT outputSize = m_resources.m_deviceResources->GetOutputSize();
	float width			= static_cast<float>(outputSize.right - outputSize.left);
	float height		= static_cast<float>(outputSize.bottom - outputSize.top);
	m_context.screenWidth			 = width;
	m_context.screenHeight		 = height;
	m_context.screenHalfWidth	= ceil(width / 2.0f);
	m_context.screenHalfHeight = ceil(height / 2.0f);

	const float fovAngleY		= 30.0f * DirectX::XM_PI / 180.0f;
	const float aspectRatio = width / height;
	const float nearPlane		= 0.01f;
	const float farPlane		= 300.f;

	m_context.worldToView			 = Matrix::Identity;
	m_context.viewToProjection = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY, aspectRatio, nearPlane, farPlane);

	// Screen pixel coordinates are x-right y-down, origin at top-left.
	m_context.pixelsToProjection = Matrix::CreateOrthographic(
		m_context.screenWidth, m_context.screenHeight, 0.0f, -1.0f);
	m_context.pixelsToProjection.m[1][1] *= -1.0f;		// y-axis goes down
	m_context.pixelsToProjection.m[3][0] = -1.f;		// translate x-origin to left
	m_context.pixelsToProjection.m[3][1] = 1.f;			// translate y-origin to top

	m_context.projectionToPixels = m_context.pixelsToProjection.Invert();

	m_resources.starField->setWindowSize(width, height);

	// Position HUD
	m_gameLogic.updateUILives();
	m_gameLogic.updateUIScore();
	m_gameLogic.updateUIDebugVariables();
	updateControlsInfo(m_context.uiControlInfo);

	m_context.updateViewMatrix();
}

//------------------------------------------------------------------------------
void
Game::OnDeviceLost()
{
	TRACE
	for (auto& modelData : m_resources.modelData)
	{
		modelData.second.model.reset();
	}

	m_resources.m_debugBound.reset();
	m_resources.m_debugBoundInputLayout.Reset();
	m_resources.m_debugBoundEffect.reset();
	m_resources.m_effectFactory.reset();
	m_resources.m_debugEffect.reset();

	m_resources.fontMono32pt.reset();
	m_resources.fontMono8pt.reset();
	m_resources.font32pt.reset();
	m_resources.font8pt.reset();
	m_resources.starField.reset();
	m_resources.explosions.reset();
	m_resources.scoreBoard.reset();
	m_resources.menuManager.reset();

	m_resources.m_batch.reset();
	m_resources.m_spriteBatch.reset();
	m_resources.m_explosionTexture.Reset();
	m_resources.m_starTexture.Reset();
	m_resources.m_states.reset();
}

//------------------------------------------------------------------------------
void
Game::OnDeviceRestored()
{
	TRACE
	createDeviceDependentResources();

	createWindowSizeDependentResources();
}

//------------------------------------------------------------------------------
#pragma endregion
