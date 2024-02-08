#pragma once
#include "Play.h"

namespace Render
{
	class Boundary
	{
	public:
		static Boundary& instance();

		void resize(float width, float height);
		void move(const Vector2f& newPos);

		void draw();

		Point2D& getTopLeft();
		Point2D& getBottomRight();
		float getWidth();
		float getHeight();

	private:

		float width;
		float height;
		Vector2f center;

		Point2D topLeft;
		Point2D bottomRight;

		void recalcSize();

		Boundary() {};
		Boundary(const Boundary& ref) = delete;
		~Boundary() {};
	};
}