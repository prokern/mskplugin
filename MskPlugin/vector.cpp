#include "pch.h"
#include "vector.h"
#include <cmath>

double Vector::Cross(Vector v)
{
	return X * v.Y - Y * v.X;
}
bool Vector::operator==(const Vector& other)
{
	return DoubleIsZero(X - other.X) && DoubleIsZero(Y - other.Y);
}

bool Vector::LineSegementsIntersect(Vector p, Vector p2, Vector q, Vector q2, Vector& intersection, bool considerCollinearOverlapAsIntersect)
{
	auto r = p2 - p;
	auto s = q2 - q;
	auto rxs = r.Cross(s);
	auto qpxr = (q - p).Cross(r);

	// If r x s = 0 and (q - p) x r = 0, then the two lines are collinear.
	if (DoubleIsZero(rxs) && DoubleIsZero(qpxr))
	{
		// 1. If either  0 <= (q - p) * r <= r * r or 0 <= (p - q) * s <= * s
		// then the two lines are overlapping,
		if (considerCollinearOverlapAsIntersect)
			if ((0 <= (q - p) * r && (q - p) * r <= r * r) || (0 <= (p - q) * s && (p - q) * s <= s * s))
				return true;

		// 2. If neither 0 <= (q - p) * r = r * r nor 0 <= (p - q) * s <= s * s
		// then the two lines are collinear but disjoint.
		// No need to implement this expression, as it follows from the expression above.
		return false;
	}

	// 3. If r x s = 0 and (q - p) x r != 0, then the two lines are parallel and non-intersecting.
	if (DoubleIsZero(rxs) && !DoubleIsZero(qpxr))
		return false;

	// t = (q - p) x s / (r x s)
	auto t = (q - p).Cross(s) / rxs;

	// u = (q - p) x r / (r x s)

	auto u = (q - p).Cross(r) / rxs;

	// 4. If r x s != 0 and 0 <= t <= 1 and 0 <= u <= 1
	// the two line segments meet at the point p + t r = q + u s.
	if (!DoubleIsZero(rxs) && (0 <= t && t <= 1) && (0 <= u && u <= 1))
	{
		// We can calculate the intersection point using either t or u.
		intersection = p + t * r;

		// An intersection was found.
		return true;
	}

	// 5. Otherwise, the two line segments are not parallel but do not intersect.
	return false;
}