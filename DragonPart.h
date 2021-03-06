#ifndef __DRAGONPART_H__
#define __DRAGONPART_H__

#include "Globals.h"
#include "Enemy.h"

class DragonPart : public Enemy
{
public:
	DragonPart();
	DragonPart(const Enemy& alienShip, const fPoint& pos, const collisionType& colType, const int& moveSelector, const float& oscillationAngle);
	~DragonPart();

	void Update();
	Enemy* createEnemyInstance(const Enemy& e, const fPoint& pos, const collisionType& colType, const int& moveSelector, const float& oscillationAngle) const;

private:
	const void selectMovementPatron(const int& moveSelector);

private:
	bool forward = true;
	float oscillationX = 0.0f;
	float oscillationY = 0.0f;
	Uint32 fireBallTimer = 0;
};

#endif // __DRAGONPART_H__