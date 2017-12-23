#ifndef __DRAGONPART_H__
#define __DRAGONPART_H__

#include "Globals.h"
#include "Enemy.h"

class DragonPart : public Enemy
{
public:
	DragonPart();
	DragonPart(const Enemy& alienShip, const fPoint& pos, const collisionType& colType, const int& moveSelector);
	~DragonPart();

	void Update();
	void selectMovementPatron(const int& moveSelector);
	Enemy* createEnemyInstance(const Enemy& e, const fPoint& pos, const collisionType& colType, const int& moveSelector) const;

private:
	bool forward = true;
};

#endif // __DRAGONPART_H__