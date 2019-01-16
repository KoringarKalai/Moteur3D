#include "pch.h"
#include "Point.h"
#include <math.h> 


Point::Point(float x, float y, float z) : x(x) , y(y) , z(z)
{
}

float Point::norme() {
	return sqrt(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f));
}

float Point::cosAngle(Point p) {
	return (x * p.x + y * p.y + z * p.z) / sqrt((pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f)) * (pow(p.x, 2.0f) + pow(p.y, 2.0f) + pow(p.z, 2.0f)));
}

Point Point::produitVectoriel(Point p) {
	Point pv = Point(
		y * p.z - z * p.y,
		z * p.x - x * p.z,
		x * p.y - y * p.x
	);
	return pv;
}

void Point::normalize() {
	float length = sqrt((pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f)));
	x = x / length;
	y = y / length;
	z = z / length;
}