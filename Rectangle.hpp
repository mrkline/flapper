#ifndef __RECTANGLE_HPP__
#define __RECTANGLE_HPP__

#include <algorithm>

struct Point
{
	Point(int x, int y) : x(x), y(y) { }

	int x, y;
};

struct Rectangle {

	Rectangle() : left(0), top(0), right(0), bottom(0) { }

	Rectangle(Point p) : left(p.x), top(p.y), right(p.x), bottom(p.y) { }

	Rectangle(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) { }

	int getWidth() const { return right - left + 1; }
	int getHeight() const { return bottom - top + 1; }
	int getArea() const { return getWidth() * getHeight(); }
	int getCenterX() const { return (left + right) / 2; }
	int getCenterY() const { return (top + bottom) / 2; }
	Point getCenter() const { return Point(getCenterX(), getCenterY()); }

	int contains(int x, int y) const { return x >= left && x <= right && y >= top && y <= bottom; }

	int adjacentTo(int x, int y, int tol = 1) const
	{ return x >= left - tol && x <= right + tol && y >= top - tol && y <= bottom + tol; }

	int adjacentTo(Rectangle o, int tol = 1) const
	{
		// http://stackoverflow.com/a/306332/713961
		return left - tol <= o.right && right + tol >= o.left && top - tol <= o.bottom && bottom + tol >= o.top;
	}

	void expandTo(int x, int y)
	{
		if (x < left)
			left = x;
		else if (x > right)
			right = x;

		if (y < top)
			top = y;
		else if (y > bottom)
			bottom = y;
	}

	void expandTo(Rectangle o)
	{
		expandTo(o.left, o.top);
		expandTo(o.right, o.bottom);
	}

	void expandBy(int amount)
	{
		left -= amount;
		top -= amount;
		bottom += amount;
		right += amount;
	}

	void constrainBy(Rectangle o)
	{
		left = std::max(left, o.left);
		top = std::max(top, o.top);
		right = std::min(right, o.right);
		bottom = std::min(bottom, o.bottom);
	}

	// All bounds are inclusive
	int left, top, right, bottom;
};

#endif
