// Moteur3D.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "pch.h"
#include <iostream>
#include "tgaimage.h"
#include <cmath>
#include <algorithm> 
#include <fstream>
#include <sstream>
#include <string>
#include <Vector>
#include <stdlib.h>
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);


struct VectF {
	float x, y, z;

	VectF(float x, float y, float z) : x(x), y(y), z(z) {}

	VectF() { x = 0; y = 0; z = 0; }

	float norme() {
		return sqrt(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f));
	}

	float cosAngle(VectF p) {
		return (x * p.x + y * p.y + z * p.z) / sqrt((pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f)) * (pow(p.x, 2.0f) + pow(p.y, 2.0f) + pow(p.z, 2.0f)));
	}

	void normalize() {
		float length = sqrt((pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f)));
		x = x / length;
		y = y / length;
		z = z / length;
	}
};

VectF crossProduct(VectF v1, VectF v2) {
	VectF pv = VectF(
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	);
	return pv;
}

VectF barycentric(VectF p0, VectF p1, VectF p2, VectF p) {
	VectF v1 = VectF(p2.x - p0.x, p1.x - p0.x, p0.x - p.x);
	VectF v2 = VectF(p2.y - p0.y, p1.y - p0.y, p0.y - p.y);
	VectF cp = crossProduct(v1,v2);
	if (std::abs(cp.z) > 1e-2) return VectF(1.f - (cp.x + cp.y) / cp.z, cp.y / cp.z, cp.x / cp.z);
	return VectF(-1, 1, 1);
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	line(x0, y0, x1, y1, image, color);
	line(x0, y0, x2, y2, image, color);
	line(x2, y2, x1, y1, image, color);
}

void drawTriangle(VectF p0, VectF p1, VectF p2, TGAImage &image, TGAColor color) {
	line(p0.x, p0.y, p1.x, p1.y, image, color);
	line(p0.x, p0.y, p2.x, p2.y, image, color);
	line(p2.x, p2.y, p1.x, p1.y, image, color);
}

void fillTriangle(VectF p0, VectF p1, VectF p2, TGAImage &image, TGAColor color, float **zbuffer) {
	int xmin = std::min(std::min(p0.x,p1.x),p2.x);
	int xmax = std::max(std::max(p0.x, p1.x), p2.x);
	int ymin = std::min(std::min(p0.y, p1.y), p2.y);
	int ymax = std::max(std::max(p0.y, p1.y), p2.y);
	for (int x = xmin; x <= xmax; x++) {
		for (int y = ymin; y <= ymax; y++) {
			VectF P = barycentric(p0, p1, p2, VectF(x,y,0));
			if (P.x < 0 || P.y < 0 || P.z < 0) continue;
			P.z = 0;
			P.z += p0.z * P.x + p1.z * P.y + p2.z * P.z;
			if (zbuffer[x][y] < P.z) {
				zbuffer[x][y] = P.z;
				image.set(x, y, color);
			}
		}
	}
}

void drawFile(int width, int height, int profondeur, TGAImage &image, TGAColor color) {
	std::vector<VectF> points;
	std::vector<std::vector<float>> triangle;
	std::ifstream infile("obj/african_head/african_head.obj");
	//std::ifstream infile("obj/diablo3_pose/diablo3_pose.obj");
	std::string line;
	int i = 0;
	int j = 0;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		char trash;
		if (!line.compare(0, 2, "v ")) {
			float a, b, c;
			iss >> trash >> a >> b >> c;
			VectF p = VectF(a, b, c);
			points.push_back(p);
			i++;
		}
		else if (!line.compare(0, 2, "f ")) {
			iss >> trash;
			std::vector<float> t;
			int num, rien;
			while (iss >> num >> trash >> rien >> trash >> rien) {
				float v = num - 1;
				t.push_back(v);
			}
			triangle.push_back(t);
			j++;
		}
	}
	float** zbuffer = new float*[height];
	for (int i = 0; i < height; ++i) {
		zbuffer[i] = new float[width];
		for (int j = 0; j < width; j++) {
			zbuffer[i][j] = std::numeric_limits<float>::min();
		}
	}
	for (int i = 0; i < triangle.size(); i++) {
		int ix0 = triangle[i][0];
		int ix1 = triangle[i][1];
		int ix2 = triangle[i][2];
		
		// VectFeur p0p1
		VectF v1 = VectF(
			points[ix1].x - points[ix0].x,
			points[ix1].y - points[ix0].y,
			points[ix1].z - points[ix0].z
		);
		// VectFeur p0p2
		VectF v2 = VectF(
			points[ix2].x - points[ix0].x,
			points[ix2].y - points[ix0].y,
			points[ix2].z - points[ix0].z
		);
		// Produit VectForiel v1 x v2
		VectF pv = crossProduct(v1,v2);

		// Produit scalaire    VectFCamera . (v1 x v2)
		VectF VectCamera = VectF(0, 0, 1);
		pv.normalize();
		float produit = (VectCamera.x * pv.x + VectCamera.y * pv.y + VectCamera.z * pv.z);
		// Si le produit est positif on dessine sinon non 
		if (produit > 0) {
			int teinte = produit * 255;
			TGAColor color = TGAColor(teinte, teinte, teinte, 255);
			VectF p0 = VectF(
				int(points[ix0].x * width / 2 + width / 2), 
				int(points[ix0].y * height / 2 + height / 2),
				int(points[ix0].z * profondeur / 2 + profondeur / 2));
			VectF p1 = VectF(
				int(points[ix1].x * width / 2 + width / 2),
				int(points[ix1].y * height / 2 + height / 2),
				int(points[ix1].z * profondeur / 2 + profondeur / 2));
			VectF p2 = VectF(
				int(points[ix2].x * width / 2 + width / 2),
				int(points[ix2].y * height / 2 + height / 2),
				int(points[ix2].z * profondeur / 2 + profondeur / 2));
			fillTriangle(p0, p1, p2, image, color, zbuffer);
		}
	}
}

int main(int argc, char** argv) {
	int h = 2000, w = 2000, p = 2000;
	TGAImage image(h, w, TGAImage::RGB);
	//line(18,14,84,76,image,white);
	//drawTriangle(18,20,60,35,40,84,image,white);
	//fillTriangle(118, 20, 160, 35, 140, 84, image, white);
	//fillTriangle(20, 20, 50, 50, 20, 80, image, white);
	//drawTriangle(20, 20, 50, 50, 20, 80, image, red);
	//fillTriangle(120, 20, 150, 50, 120, 80, image, white);
	//fillTriangle(80, 180, 50, 150, 20, 180, image, white);
	//drawTriangle(80, 180, 50, 150, 20, 180, image, red);
	//fillTriangle(180, 180, 150, 150, 120, 180, image, white);
	//fillTriangle(80, 220, 50, 250, 80, 280, image, white);
	//drawTriangle(80, 220, 50, 250, 80, 280, image, red);
	//fillTriangle(180, 220, 150, 250, 180, 280, image, white);
	//fillTriangle(80, 320, 50, 350, 20, 320, image, white);
	//drawTriangle(80, 320, 50, 350, 20, 320, image, red);
	//fillTriangle(180, 320, 150, 350, 120, 320, image, white);
	
	drawFile(h, w, p, image, white);

	//Point p1 = Point(18, 14, 13);
	//Point p2 = Point(60, 35, 84);
	//Point p3 = Point(40, 80, 84);
	//drawTriangle(p1, p2, p3, image, white);

	//fillTriangle(p1, p2, p3, image, white);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

