#include "pch.h"
#include "GameLogic.h"
#include "AppContext.h"
#include "AppResources.h"
#include "StepTimer.h"
#include "Entity.h"
#include "fmt/format.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float PLAYER_DEATH_TIME_S			 = 1.0f;
constexpr float PLAYER_REVIVE_TIME_S		 = 2.0f;
static const Vector3 PLAYER_MAX_POSITION = {45.0f, 27.0f, 0.0f};
static const Vector3 PLAYER_MIN_POSITION = -PLAYER_MAX_POSITION;
static const Vector3 PLAYER_START_POS(0.0f, -PLAYER_MAX_POSITION.y, 0.0f);

constexpr int POINTS_PER_KILL = 1000;

//------------------------------------------------------------------------------
GameLogic::GameLogic(AppContext& context, AppResources& resources)
		: m_context(context)
		, m_resources(resources)
		, m_enemies(context, resources)
{
	reset();
}

//------------------------------------------------------------------------------
void
GameLogic::reset()
{
	m_enemies.reset();

	for (size_t i = PLAYER_SHOTS_IDX; i < ENEMIES_END; ++i)
	{
		m_context.entities[i].isAlive			= false;
		m_context.entities[i].isColliding = false;
	}

	auto& player			 = m_context.entities[PLAYERS_IDX];
	player.isAlive		 = true;
	player.isColliding = false;
	player.position		 = PLAYER_START_POS;

	m_context.playerScore = 0;
	m_context.playerLives = INITIAL_NUM_PLAYER_LIVES;
	m_context.playerState = PlayerState::Normal;
	m_hudDirty						= true;

	resetCamera();
}

//------------------------------------------------------------------------------
void
GameLogic::resetCamera()
{
	const auto& atP = Vector3{0.0f, 0.0f, 0.0f};
	static const XMVECTORF32 eye
		= {atP.x, atP.y, atP.z + m_context.cameraDistance, 0.0f};
	static const XMVECTORF32 at = {atP.x, atP.y, atP.z, 0.0f};
	static const XMVECTORF32 up = {0.0f, 1.0f, 0.0f, 0.0f};
	m_context.view							= XMMatrixLookAtRH(eye, at, up);
}

//------------------------------------------------------------------------------
GameLogic::GameStatus
GameLogic::update(const DX::StepTimer& timer)
{
	UNREFERENCED_PARAMETER(timer);

	float elapsedTimeS = float(timer.GetElapsedSeconds());
	// float totalTimeS	 = static_cast<float>(timer.GetTotalSeconds());
	m_enemies.update(timer);
	performPhysicsUpdate(timer);

	switch (m_context.playerState)
	{
		case PlayerState::Normal:
			performCollisionTests();
			break;

		case PlayerState::Dying:
			if ((m_context.playerDeathTimerS -= elapsedTimeS) <= 0.0f) {
				if (--m_context.playerLives > -1) {
					m_context.playerState				 = PlayerState::Reviving;
					m_context.playerReviveTimerS = PLAYER_REVIVE_TIME_S;
					m_hudDirty									 = true;
				}
				else
				{
					return GameStatus::GameOver;
				}
			}
			break;

		case PlayerState::Reviving:
			if ((m_context.playerReviveTimerS -= elapsedTimeS) <= 0.0f) {
				m_context.playerState = PlayerState::Normal;
			}
			performCollisionTests();
			break;
	}

	return GameStatus::Playing;
}

//------------------------------------------------------------------------------
void
GameLogic::render()
{
	auto dc = m_resources.m_deviceResources->GetD3DDeviceContext();

	size_t idx = 0;
	for (auto& entity : m_context.entities)
	{
		if (PLAYERS_IDX == idx) {
			renderPlayerEntity(entity);
		}
		else if (entity.isAlive)
		{
			renderEntity(entity);
		}
		++idx;
	}

	// Explosions & HUD
	CommonStates states(m_resources.m_deviceResources->GetD3DDevice());
	auto& spriteBatch = m_resources.m_spriteBatch;
	spriteBatch->Begin(SpriteSortMode_Deferred, states.Additive());

	m_resources.explosions->render(*spriteBatch);
	drawHUD();

	spriteBatch->End();

	// Debug Drawing
	if (m_context.debugDraw) {
		dc->OMSetBlendState(m_resources.m_states->Opaque(), nullptr, 0xFFFFFFFF);
		dc->OMSetDepthStencilState(m_resources.m_states->DepthNone(), 0);
		dc->RSSetState(m_resources.m_states->CullNone());

		m_resources.m_debugEffect->SetView(m_context.view);
		m_resources.m_debugEffect->SetProjection(m_context.proj);
		m_resources.m_debugEffect->Apply(dc);
		dc->IASetInputLayout(m_resources.m_debugInputLayout.Get());

		m_resources.m_batch->Begin();
		for (auto& entity : m_context.entities)
		{
			if (entity.isAlive) {
				renderEntityBound(entity);
			}
		}
		m_enemies.debugRender(m_resources.m_batch.get());
		m_resources.m_batch->End();

		spriteBatch->Begin(SpriteSortMode_Deferred, states.Additive());
		drawDebugVariables();
		spriteBatch->End();
	}
}

//------------------------------------------------------------------------------
void
GameLogic::performPhysicsUpdate(const DX::StepTimer& timer)
{
	float elapsedTimeS = float(timer.GetElapsedSeconds());

	// Player input forces
	auto& player = m_context.entities[PLAYERS_IDX];
	m_context.playerAccel *= m_context.playerSpeed;

	Vector3 frictionNormal = -player.velocity;
	frictionNormal.Normalize();
	m_context.playerAccel += m_context.playerFriction * frictionNormal;

	// Ballistic entities
	for (size_t i = BALLISTIC_IDX; i < BALLISTIC_END; ++i)
	{
		const bool isPlayer = (i < PLAYERS_END);
		auto& e							= m_context.entities[i];

		// Integrate Position
		const Vector3& accel = (isPlayer) ? m_context.playerAccel : Vector3();

		e.position = 0.5f * accel * (elapsedTimeS * elapsedTimeS)
								 + e.velocity * elapsedTimeS + e.position;
		e.velocity = accel * elapsedTimeS + e.velocity;

		if (isPlayer) {
			auto slide = [& incident = e.velocity](Vector3 normal)
			{
				return incident - 1.0f * incident.Dot(normal) * normal;
			};

			// Limit position
			if (e.position.x < -PLAYER_MAX_POSITION.x) {
				e.position.x = -PLAYER_MAX_POSITION.x;
				e.velocity	 = slide(Vector3(1.0f, 0.0f, 0.0f));
			}
			else if (e.position.x > PLAYER_MAX_POSITION.x)
			{
				e.position.x = PLAYER_MAX_POSITION.x;
				e.velocity	 = slide(Vector3(-1.0f, 0.0f, 0.0f));
			}
			if (e.position.y < -PLAYER_MAX_POSITION.y) {
				e.position.y = -PLAYER_MAX_POSITION.y;
				e.velocity	 = slide(Vector3(0.0f, 1.0f, 0.0f));
			}
			else if (e.position.y > PLAYER_MAX_POSITION.y)
			{
				e.position.y = PLAYER_MAX_POSITION.y;
				e.velocity	 = slide(Vector3(0.0f, -1.0f, 0.0f));
			}

			// Clamp velocity
			float velocityMagnitude = e.velocity.Length();
			if (velocityMagnitude > m_context.playerMaxVelocity) {
				e.velocity.Normalize();
				e.velocity *= m_context.playerMaxVelocity;
			}
			else if (velocityMagnitude < m_context.playerMinVelocity)
			{
				e.velocity = Vector3();
			}
		}
	}
}

//------------------------------------------------------------------------------
void
GameLogic::performCollisionTests()
{
	for (size_t i = 0; i < NUM_ENTITIES; ++i)
	{
		m_context.entities[i].isColliding = false;
	}

	auto& player = m_context.entities[PLAYERS_IDX];

	auto onPlayerShotHitsEnemy =
		[	&score				= m_context.playerScore,
			&explosions		= m_resources.explosions,
			&soundEffects	= m_resources.soundEffects,
		  &m_hudDirty		= this->m_hudDirty
		](Entity& shot, Entity& testEntity)
	{
		auto pos = shot.position + shot.model->bound.Center;
		explosions->emit(pos, Vector3());
		soundEffects[AudioResource::EnemyExplode]->Play();

		shot.isColliding			 = true;
		testEntity.isColliding = true;
		shot.isAlive					 = false;
		testEntity.isAlive		 = false;
		score += POINTS_PER_KILL;
		m_hudDirty = true;
	};

	auto onPlayerHit =
		[	&context			= m_context,
			&explosions		= m_resources.explosions,
			&soundEffects	= m_resources.soundEffects
		](Entity & player, Entity & enemy)
	{
		auto pos = player.position + player.model->bound.Center;
		explosions->emit(pos, Vector3());
		soundEffects[AudioResource::PlayerExplode]->Play();

		pos = enemy.position + enemy.model->bound.Center;
		explosions->emit(pos, Vector3());
		soundEffects[AudioResource::EnemyExplode]->Play();

		player.isColliding = true;
		enemy.isColliding	= true;

		enemy.isAlive							= false;
		context.playerState				= PlayerState::Dying;
		context.playerDeathTimerS = PLAYER_DEATH_TIME_S;
	};

	// Pass 1 - PlayerShots		-> Enemies
	for (size_t srcIdx = PLAYER_SHOTS_IDX; srcIdx < PLAYER_SHOTS_END; ++srcIdx)
	{
		auto& srcEntity = m_context.entities[srcIdx];
		collisionTestEntity(
			srcEntity, ENEMIES_IDX, ENEMIES_END, onPlayerShotHitsEnemy);
	}

	// Player is invulnerable, no more collision tests
	if (m_context.playerState == PlayerState::Reviving) {
		return;
	}

	// Pass 2 - Player				-> Enemies
	collisionTestEntity(player, ENEMIES_IDX, ENEMIES_END, onPlayerHit);

	// Pass 3 - Player				-> EnemyShots
	collisionTestEntity(player, ENEMY_SHOTS_IDX, ENEMY_SHOTS_END, onPlayerHit);
}

//------------------------------------------------------------------------------
template <typename Func>
void
GameLogic::collisionTestEntity(
	Entity& entity,
	const size_t rangeStartIdx,
	const size_t rangeOnePastEndIdx,
	Func onCollision)
{
	if (!entity.isAlive) {
		return;
	}

	// TODO(James): Use the GCL <notnullable> to compile time enforce assertion
	assert(entity.model);
	auto& srcBound = entity.model->bound;
	auto srcCenter = entity.position + srcBound.Center;

	for (size_t testIdx = rangeStartIdx; testIdx < rangeOnePastEndIdx; ++testIdx)
	{
		assert(m_context.entities[testIdx].model);
		auto& testEntity = m_context.entities[testIdx];
		if (!testEntity.isAlive) {
			continue;
		}

		auto& testBound = testEntity.model->bound;
		auto testCenter = testEntity.position + testBound.Center;

		auto distance = (srcCenter - testCenter).Length();
		if (distance <= (srcBound.Radius + testBound.Radius)) {
			onCollision(entity, testEntity);
		}
	}
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void
GameLogic::renderPlayerEntity(Entity& entity)
{
	switch (m_context.playerState)
	{
		case PlayerState::Normal:
			renderEntity(entity, XM_PI);
			break;

		case PlayerState::Dying:
			// TODO(James): Render Player Death
			break;

		case PlayerState::Reviving:
			// Flash the player every half second
			if (
				static_cast<int>(m_context.playerReviveTimerS * PLAYER_REVIVE_TIME_S)
					% 2
				!= 0)
			{
				renderEntity(entity, XM_PI);
			}
			break;
	}
}

//------------------------------------------------------------------------------
void
GameLogic::renderEntity(Entity& entity, float orientation)
{
	// TODO(James): Use <notnullable> to enforce assertion
	assert(entity.model);
	const auto& modelData		= entity.model;
	const auto& boundCenter = entity.model->bound.Center;

	Matrix world = Matrix::CreateTranslation(boundCenter).Invert()
								 * Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, orientation)
								 * Matrix::CreateTranslation(entity.position + boundCenter);

	modelData->model->Draw(
		m_resources.m_deviceResources->GetD3DDeviceContext(),
		*m_resources.m_states,
		world,
		m_context.view,
		m_context.proj);

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
GameLogic::renderEntityBound(Entity& entity)
{
	// TODO(James): Use <notnullable> to enforce assertion
	assert(entity.model);
	// const auto& modelData = entity.model;

	auto bound	 = entity.model->bound;
	bound.Center = bound.Center + entity.position;
	DX::Draw(
		m_resources.m_batch.get(),
		bound,
		(entity.isColliding) ? Colors::Red : Colors::Lime);

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
void
GameLogic::updateUIScore()
{
	auto& score					= m_context.uiScore;
	score.font					= m_resources.font32pt.get();
	score.text					= fmt::format(L"Score: {}", m_context.playerScore);
	XMVECTOR dimensions = score.font->MeasureString(score.text.c_str());
	score.origin				= Vector2((XMVectorGetX(dimensions) / 2.f), 0.0f);
	score.dimensions		= dimensions;
	score.position.x		= m_resources.m_screenWidth / 2.f;
	score.position.y		= 0.0f;
}

//------------------------------------------------------------------------------
void
GameLogic::updateUILives()
{
	auto& lives					= m_context.uiLives;
	lives.font					= m_resources.font32pt.get();
	lives.text					= fmt::format(L"Lives: {}", m_context.playerLives);
	XMVECTOR dimensions = lives.font->MeasureString(lives.text.c_str());
	lives.origin				= Vector2(0.0f, XMVectorGetY(dimensions));
	lives.dimensions		= dimensions;
	lives.position.x		= 0.0f;
	lives.position.y		= static_cast<float>(m_resources.m_screenHeight);
}

//------------------------------------------------------------------------------
void
GameLogic::drawHUD()
{
	if (m_hudDirty) {
		m_hudDirty = false;
		updateUIScore();
		updateUILives();
	}

	m_context.uiScore.draw(Colors::Yellow, *m_resources.m_spriteBatch);

	m_context.uiLives.draw(Colors::Yellow, *m_resources.m_spriteBatch);
}

//------------------------------------------------------------------------------
void
GameLogic::updateUIDebugVariables()
{
	float yPos = 0.0f;

	auto formatUI = [
		&yPos,
		font				= m_resources.font8pt.get(),
		screenWidth = m_resources.m_screenWidth
	](UIText & ui, const wchar_t* fmt, float fVar)
	{
		ui.font							= font;
		ui.text							= fmt::format(fmt, fVar);
		XMVECTOR dimensions = ui.font->MeasureString(ui.text.c_str());
		float width					= XMVectorGetX(dimensions);
		float height				= XMVectorGetY(dimensions);
		ui.origin						= Vector2(width, 0.0f);
		ui.dimensions				= dimensions;
		ui.position.x				= static_cast<float>(screenWidth);
		ui.position.y				= yPos;
		yPos += height;
	};

	formatUI(m_context.uiPlayerSpeed, L"Player Speed: {}", m_context.playerSpeed);
	formatUI(
		m_context.uiPlayerFriction,
		L"Player Friction: {}",
		m_context.playerFriction);
	formatUI(
		m_context.uiPlayerMaxVelocity,
		L"Player Max Velocity: {}",
		m_context.playerMaxVelocity);
	formatUI(
		m_context.uiPlayerMinVelocity,
		L"Player Min Velocity: {}",
		m_context.playerMinVelocity);
	formatUI(
		m_context.uiCameraDist, L"Camera Dist: {}", m_context.cameraDistance);
}

//------------------------------------------------------------------------------
void
GameLogic::drawDebugVariables()
{
	// TODO(James): Move to somewhere called infrequently (ie. when they change)
	updateUIDebugVariables();

	auto drawUI =
		[& font				= m_resources.font8pt,
		 &spriteBatch = m_resources.m_spriteBatch](UIText & ui)
	{
		ui.draw(Colors::MediumVioletRed, *spriteBatch);
	};

	drawUI(m_context.uiPlayerSpeed);
	drawUI(m_context.uiPlayerFriction);
	drawUI(m_context.uiPlayerMaxVelocity);
	drawUI(m_context.uiPlayerMinVelocity);
	drawUI(m_context.uiCameraDist);
}

//------------------------------------------------------------------------------
