#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleCollision.h"

using namespace std;

ModuleCollision::ModuleCollision()
{
	//Create the matrix that will decide if a collision is possible (true) or won't affect to the gameplay (false)
	collisionMatrix[PLAYER][PLAYER] = false;
	collisionMatrix[PLAYER][P_LASER] = false;

	collisionMatrix[ENEMY][ENEMY] = false;
	collisionMatrix[ENEMY][E_LASER] = false;
	collisionMatrix[ENEMY][D_OBSTACLE] = false;
	collisionMatrix[ENEMY][NOLETHAL_D_OBSTACLE] = false;
	collisionMatrix[ENEMY][ND_ENEMY] = false;

	collisionMatrix[P_LASER][P_LASER] = false;
	collisionMatrix[P_LASER][E_LASER] = false;

	collisionMatrix[E_LASER][E_LASER] = false;
	collisionMatrix[E_LASER][D_OBSTACLE] = false;
	collisionMatrix[E_LASER][ND_ENEMY] = false;
	collisionMatrix[E_LASER][NOLETHAL_D_OBSTACLE] = false;

	collisionMatrix[D_OBSTACLE][D_OBSTACLE] = false;
	collisionMatrix[D_OBSTACLE][ND_ENEMY] = false;
	collisionMatrix[D_OBSTACLE][NOLETHAL_D_OBSTACLE] = false;

	collisionMatrix[NOLETHAL_D_OBSTACLE][NOLETHAL_D_OBSTACLE] = false;
	collisionMatrix[NOLETHAL_D_OBSTACLE][ND_ENEMY] = false;

	collisionMatrix[ND_ENEMY][ND_ENEMY] = false;
}

ModuleCollision::~ModuleCollision()
{}

update_status ModuleCollision::PreUpdate()
{
	// Remove all colliders scheduled for deletion
	for (list<Collider*>::iterator it = colliders.begin(); it != colliders.end();)
	{
		if ((*it)->to_delete == true)
		{
			RELEASE(*it);
			it = colliders.erase(it);
		}
		else
			++it;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleCollision::Update()
{
	//Check if any collision is happening and, in case of positive response, call the corresponding module callbacks to react (Minimum possible checks ensured)
	bool collisionDone = false;

	for (list<Collider*>::const_iterator it = colliders.cbegin(), end = colliders.cend(); it != end; it++)
	{
		for (list<Collider*>::const_iterator it2 = next(it, 1); it2 != end; it2++)
		{
			if (collisionMatrix[(*it)->colType][(*it2)->colType] && collisionMatrix[(*it2)->colType][(*it)->colType])
			{
				if ((*it)->CheckCollision((*it2)->rect, (*it2)->depth))
				{
					(*it)->modCallback->onCollision((*it), (*it2));
					(*it2)->modCallback->onCollision((*it2), (*it));

					collisionDone = true;
					break;
				}
			}
		}

		if (collisionDone)
		{
			collisionDone = false;
			break;
		}
	}

	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug == true)
		DebugDraw();

	return UPDATE_CONTINUE;
}

//Debug draw of colliders (F1 key)
const void ModuleCollision::DebugDraw()
{
	for (list<Collider*>::iterator it = colliders.begin(); it != colliders.end(); ++it)
		App->renderer->DrawQuad((*it)->rect, 255, 0, 0, 80);
}

// Called before quitting
bool ModuleCollision::CleanUp()
{
	LOG("Freeing all colliders");

	for (list<Collider*>::iterator it = colliders.begin(); it != colliders.end(); ++it)
		RELEASE(*it);

	colliders.clear();

	return true;
}

//Add a new collider
Collider* ModuleCollision::AddCollider(const SDL_Rect& rect, collisionType colType, const int& zDepth, Module* callback)
{
	Collider* ret = new Collider(rect, colType, zDepth, callback);
	colliders.push_back(ret);

	return ret;
}

// -----------------------------------------------------

//Check if two certain colliders are generating a collision.
const bool Collider::CheckCollision(const SDL_Rect& r, const int& depth) const
{
	bool xColl = true;
	bool yColl = true;
	bool zColl = false;

	if ( r.x > (this->rect.x + this->rect.w) || (r.x + r.w) < this->rect.x ) xColl = false;
	if ( r.y > (this->rect.y + this->rect.h) || (r.y + r.h) < this->rect.y ) yColl = false;

	if ( this->depth == depth || abs(this->depth - depth) <= 1) zColl = true;
	
	return (xColl && yColl && zColl);
}
