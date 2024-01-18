#include "boundary.h"

namespace Render
{
	Boundary& Boundary::instance()
	{
		static Boundary instance;

		return instance;
	}
	void Boundary::resize(float width, float height)
	{
		this->width = width;
		this->height = height;
		recalcSize();
	}
	void Boundary::move(const Vector2f& newPos)
	{
		this->center = newPos;
		recalcSize();
	}
	void Boundary::draw()
	{
		Play::DrawRect(topLeft, bottomRight, Play::cWhite);
	}
	Point2D& Boundary::getTopLeft()
	{
		return topLeft;
	}
	Point2D& Boundary::getBottomRight()
	{
		return bottomRight;
	}
	void Boundary::recalcSize()
	{
		topLeft = { center.x - width, center.y - height };
		bottomRight = { center.x + width, center.y + height };
	}
}