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
	float Boundary::getWidth()
	{
		return this->width;
	}
	float Boundary::getHeight()
	{
		return this->height;
	}
	void Boundary::recalcSize()
	{
		topLeft = { center.x - (width / 2.0f), center.y - (height / 2.0f) };
		bottomRight = { center.x + (width / 2.0f), center.y + (height/ 2.0f)};
	}
}