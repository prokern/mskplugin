#pragma once
#include "Common.h"

class Vector
{
public:
	
	double X = 0, Y = 0;

	double Cross(Vector v);
	bool operator ==(const Vector& other);

	friend Vector operator -(Vector& v, Vector& w)
	{
		return Vector{ v.X - w.X, v.Y - w.Y };
	}
	friend Vector operator +(Vector& v, Vector& w)
	{
		return Vector{ v.X + w.X, v.Y + w.Y };
	}
	friend double operator *(Vector& v, Vector& w)
	{
		return v.X * w.X + v.Y * w.Y;
	}
	friend Vector operator *(Vector& v, double mult)
	{
		return Vector{ v.X * mult, v.Y * mult };
	}
	friend Vector operator *(double mult, Vector v)
	{
		return Vector{ v.X * mult, v.Y * mult };
	}
	static bool LineSegementsIntersect(Vector p, Vector p2, Vector q, Vector q2, Vector& intersection, bool considerCollinearOverlapAsIntersect = false);
};