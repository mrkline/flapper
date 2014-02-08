#ifndef __RECTANGLE_HPP__
#define __RECTANGLE_HPP__

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

	int adjacentTo(int x, int y) const { return x >= left - 1 && x <= right + 1 && y >= top - 1 && y <= bottom + 1; }

	int adjacentTo(Rectangle o) const
	{
		// http://stackoverflow.com/a/306332/713961
		return left - 1 <= o.right && right + 1 >= o.left && top - 1 <= o.bottom && bottom + 1 >= o.top;
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

	// All bounds are inclusive
	int left, top, right, bottom;
};

#endif
