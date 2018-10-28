#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    // TODO: Returns the overlap rectangle between the bounding boxes of entities a and b
	if (a->hasComponent<CBoundingBox>() && a->hasComponent<CTransform>() && b->hasComponent<CBoundingBox>() && b->hasComponent<CTransform>())
	{
		auto ab = a->getComponent<CBoundingBox>();
		auto at = a->getComponent<CTransform>();
		auto bb = b->getComponent<CBoundingBox>();
		auto bt = b->getComponent<CTransform>();

		Vec2 delta = Vec2(abs(at->pos.x - bt->pos.x), abs(at->pos.y - bt->pos.y));
		Vec2 overlap = ab->halfSize + bb->halfSize - delta;

		overlap.x = overlap.x < 0 ? 0 : overlap.x;
		overlap.y = overlap.y < 0 ? 0 : overlap.y;

		return overlap;
	}

    return Vec2(0, 0);
}
