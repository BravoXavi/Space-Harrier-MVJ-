#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleParticles.h"
#include "ModuleRender.h"
#include "ModuleCollision.h"
#include "ModuleFadeToBlack.h"
#include "ModulePlayer.h"
#include "ModuleEnemy.h"
#include "ModuleTime.h"
#include "ModuleShadows.h"
#include "ModuleAudio.h"

const float ModulePlayer::playerDepth = 0.0f;

ModulePlayer::ModulePlayer(bool active) : Module(active)
{
	run.frames.push_back({ 4, 4, 20, 47 });
	run.frames.push_back({ 25, 4, 20, 47 });
	run.frames.push_back({ 49, 2, 25, 49 });
	run.frames.push_back({ 75, 3, 21, 47 });
	run.speed = 4.0f;

	middle.frames.push_back({ 108,2,26,49 });

	left1.frames.push_back({ 142,2,22,50 });
	left2.frames.push_back({ 170,3,20,48 });

	right2.frames.push_back({ 197,3,20,48 });
	right1.frames.push_back({ 221,2,22,50 });

	tripping.frames.push_back({ 6, 118, 22, 38 });
	tripping.frames.push_back({ 45, 120, 20, 33 });
	tripping.frames.push_back({ 83, 123, 24, 29 });
	tripping.speed = 2.0f;
	tripping.loop = false;

	hit.frames.push_back({ 6, 61, 22, 39 });
	hit.frames.push_back({ 34, 61, 26, 40 });
	hit.frames.push_back({ 73, 70, 26, 22 });
	hit.frames.push_back({ 113, 74, 27, 20 });
	hit.frames.push_back({ 158, 65, 26, 31 });
	hit.speed = 0.5f;
	hit.loop = false;

	current_animation = &run;
}

ModulePlayer::~ModulePlayer()
{}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading Player");
	initAnimationTimer = SDL_GetTicks();
	invulnerableTimer = SDL_GetTicks();

	graphics = App->textures->Load("assets/character.png");

	deathSFX = App->audio->LoadFx("assets/sfx/VOICE_Death.wav");
	tripSFX = App->audio->LoadFx("assets/sfx/VOICE_Ouch.wav");
	getreadySFX = App->audio->LoadFx("assets/sfx/VOICE_GetReady.wav");

	gotHit = false;
	gotTrip = false;
	destroyed = false;
	lives = PLAYER_LIVES;
	position.x = (float)(SCREEN_WIDTH / 2) - (run.GetCurrentFrame().w / 2);
	position.y = (float)(SCREEN_HEIGHT - run.GetCurrentFrame().h);

	collider = App->collision->AddCollider({ (int)position.x * SCREEN_SIZE, (int)position.y * SCREEN_SIZE, current_animation->GetCurrentFrame().w * SCREEN_SIZE, current_animation->GetCurrentFrame().h * SCREEN_SIZE }, PLAYER, (int)playerDepth, App->player);

	if (App->renderer->horizonY != (float)FLOOR_Y_MIN) 
		App->renderer->horizonY = (float)FLOOR_Y_MIN;

	hit.animationWithoutLoopEnded = false;
	hit.Reset();
	current_animation = &run;

	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	App->textures->Unload(graphics);

	return true;
}

// Update: draw background
update_status ModulePlayer::Update()
{
	Uint32 tickCheck = SDL_GetTicks();

	float speed = 150.0f * App->time->getDeltaTime();
	playerWidth = current_animation->GetCurrentFrame().w;
	playerHeight = current_animation->GetCurrentFrame().h;
	
	if (tickCheck - invulnerableTimer > 2000.0f)
		invulnerableState = false;

	//If the Player is able of doign anything (Is not tripping, it isn't hit, it's not the start of the game), it can move around or shoot
	if (tickCheck - initAnimationTimer > 2500.0f && !gotTrip && !gotHit && !App->renderer->stopUpdating)
	{
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			if (position.x > 0.0f) 
				position.x -= speed;

			if (current_animation == &run) 
				checkHorizontalAnimation(true);
			else 
				checkHorizontalAnimation();

			moveCollider();
		}

		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			if (position.x + (float)playerWidth < (float)SCREEN_WIDTH) 
				position.x += speed;

			if (current_animation == &run) 
				checkHorizontalAnimation(true);
			else 
				checkHorizontalAnimation();

			moveCollider();
		}

		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		{
			if (position.y < SCREEN_HEIGHT - playerHeight)
			{
				position.y += speed;
				modifyHorizonY();
			}
			else
			{
				current_animation = &run;
			}

			if (current_animation != &run) 
				moveCollider();
		}

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		{
			if (position.y > 0)
			{
				position.y -= speed;
				modifyHorizonY();
			}

			if (current_animation == &run)
				checkHorizontalAnimation();

			moveCollider();
		}

		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			App->particles->AddParticle(App->particles->p_laser, position.x + (2.0f *(float)playerWidth) / 2.0f, position.y + (float)playerHeight / 3.0f, 1.0f, P_LASER);

		if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
			godMode = !godMode;
	}
	else
	{
		if (gotTrip)
		{
			current_animation = &tripping;
			if (current_animation->animationWithoutLoopEnded)
			{
				gotTrip = false;
				current_animation->animationWithoutLoopEnded = false;
				current_animation->Reset();				
				checkHorizontalAnimation();
			}
		}
		else if (gotHit)
		{
			current_animation = &hit;
			if (position.y < (float)SCREEN_HEIGHT - current_animation->GetCurrentFrame().h)
			{
				position.y += speed / 1.5f;
				modifyHorizonY();
			}

			if (current_animation->animationWithoutLoopEnded)
			{
				if (lives > 0)
				{
					invulnerableState = true;
					invulnerableTimer = SDL_GetTicks();
					App->audio->PlayFx(getreadySFX);
					current_animation->animationWithoutLoopEnded = false;
					current_animation->Reset();
					App->renderer->stopUpdating = false;
					gotHit = false;
					current_animation = &run;
					position.y = (float)SCREEN_HEIGHT - current_animation->GetCurrentFrame().h;
				}
			}

			moveCollider();
		}
		else if (tickCheck - initAnimationTimer > 1000.f)
		{
			run.speed = 2.0f;
			if (position.y > (float)SCREEN_HEIGHT / 2)
			{
				position.y -= speed;
				modifyHorizonY();
				checkHorizontalAnimation();
			}

			moveCollider();
		}
	}

	//If the player is in invulnerable state (just got hit) the sprite will appear with a certain alpha so the user can notice
	if (invulnerableState || godMode) 
		SDL_SetTextureAlphaMod(graphics, 100);
	else 
		SDL_SetTextureAlphaMod(graphics, 250);

	BlitTarget* dataToBlit = new BlitTarget(graphics, position.x, position.y, playerDepth, (float)playerWidth, (float)playerHeight, &(current_animation->GetCurrentFrame()));

	if (destroyed == false)
	{
		App->shadows->DrawShadow(position.x, position.y, position.z, (float)playerWidth);
		App->renderer->depthBuffer[(int)dataToBlit->z].push_back(*dataToBlit);
	}

	delete dataToBlit;

	return UPDATE_CONTINUE;
}

const void ModulePlayer::moveCollider() const
{
	collider->SetPos((int)position.x, (int)position.y, (int)playerDepth, playerWidth, playerHeight);
}

const void ModulePlayer::checkHorizontalAnimation(bool running)
{
	int realPosition = (int)position.x + middle.GetCurrentFrame().w / 2;
	
	if (realPosition <= SCREEN_DIVISOR)
	{
		if(!running) 
			current_animation = &left2;

		setCharSpeed();
		return;
	}
	else if (realPosition <= (SCREEN_DIVISOR * 2))
	{
		if (!running) 
			current_animation = &left1;
		setCharSpeed();

		return;
	}
	else if (realPosition <= (SCREEN_DIVISOR * 3))
	{
		if (!running) 
			current_animation = &middle;
		setCharSpeed();

		return;
	}
	else if (realPosition <= (SCREEN_DIVISOR * 4))
	{
		if (!running) 
			current_animation = &right1;
		setCharSpeed();

		return;
	}
	else if (realPosition <= (SCREEN_DIVISOR * 5))
	{
		if (!running) 
			current_animation = &right2;
		setCharSpeed();

		return;
	}
}

const void ModulePlayer::modifyHorizonY() const
{
	float offsetValue = (float)SCREEN_HEIGHT - (float)playerHeight;
	float temp = (offsetValue - (float)position.y) / offsetValue;
	float newHorizonValue = (temp * ((float)FLOOR_Y_MAX - (float)FLOOR_Y_MIN)) + (float)FLOOR_Y_MIN;
	App->renderer->actualHorizonY = newHorizonValue;
}

const void ModulePlayer::setCharSpeed()
{
	int screenPosition = ((int)position.x + (run.GetCurrentFrame().w / 2)) - (SCREEN_WIDTH / 2);
	float speedPercent = ((float)screenPosition * 1.0f) / ((float)SCREEN_WIDTH / 2.0f);

	if (current_animation == &middle) 
		speedPercent = 0.0f;

	App->renderer->playerSpeed = (speedPercent * 150.0f) * App->time->getDeltaTime();
}

const bool ModulePlayer::onCollision(Collider* moduleOwner, Collider* otherCollider) 
{
	bool ret = true;

	if(!invulnerableState && !godMode)
	{
		if (otherCollider->colType == NOLETHAL_D_OBSTACLE && !gotHit)
		{
			if (!gotTrip)
			{
				App->audio->PlayFx(tripSFX);
				gotTrip = true;
			}
		}
		else
		{
			if (!gotHit)
			{
				App->audio->PlayFx(deathSFX);
				App->renderer->stopUpdating = true;
				gotHit = true;
				LoseOneLive();
			}
		}
	}

	return ret;
}

const void ModulePlayer::LoseOneLive()
{
	lives -= 1;
}
