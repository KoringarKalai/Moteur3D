#pragma once
class Point
{
public:
	float x, y, z;
	Point(float x, float y, float z);
	float norme();
	float cosAngle(Point p);
	Point produitVectoriel(Point p);
	void normalize();
};

