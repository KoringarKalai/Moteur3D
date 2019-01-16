// Moteur3D.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "pch.h"
#include "Point.h"
#include <iostream>
#include "tgaimage.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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

void drawTriangle(Point p0, Point p1, Point p2, TGAImage &image, TGAColor color) {
	line(p0.x, p0.y, p1.x, p1.y, image, color);
	line(p0.x, p0.y, p2.x, p2.y, image, color);
	line(p2.x, p2.y, p1.x, p1.y, image, color);
}

void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	if (y0 > y1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	if (y0 > y2) {
		std::swap(x0, x2);
		std::swap(y0, y2);
	}
	if (y2 > y1) {
		std::swap(x2, x1);
		std::swap(y2, y1);
	}
	for (int y = y0; y < y1; y++) {
		float t;
		float xd;
		if (y < y2) { 
			t = (y - y0) / (float)(y2 - y0);
			xd = x0 * (1. - t) + x2 * t;
		} else {
			t = (y - y1) / (float)(y2 - y1);
			xd = x1 * (1. - t) + x2 * t;
		}
		t = (y - y0) / (float)(y1 - y0);
		float xg = x0 * (1. - t) + x1 * t;
		if (xd < xg) {
			std::swap(xg, xd);
		}
		for (int x = xg; x < xd; x++) {
			image.set(x, y, color);
		}
	}
}

void fillTriangle(Point p0, Point p1, Point p2, TGAImage &image, TGAColor color) {
	if (p0.y > p1.y) {
		std::swap(p0, p1);
	}
	if (p0.y > p2.y) {
		std::swap(p0, p2);
	}
	if (p2.y > p1.y) {
		std::swap(p2, p1);
	}
	for (int y = p0.y; y < p1.y; y++) {
		float t;
		float xd;
		if (y < p2.y) {
			t = (y - p0.y) / (float)(p2.y - p0.y);
			xd = p0.x * (1. - t) + p2.x * t;
		}
		else {
			t = (y - p1.y) / (float)(p2.y - p1.y);
			xd = p1.x * (1. - t) + p2.x * t;
		}
		t = (y - p0.y) / (float)(p1.y - p0.y);
		float xg = p0.x * (1. - t) + p1.x * t;
		if (xd < xg) {
			std::swap(xg, xd);
		}
		for (int x = xg; x < xd; x++) {
			image.set(x, y, color);
		}
	}
}

void drawFile(int width, int height, TGAImage &image, TGAColor color) {
	std::vector<std::vector<float>> points;
	std::vector<std::vector<float>> triangle;
	std::ifstream infile("african_head.obj");
	std::string line;
	int i = 0;
	int j = 0;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);

		char trash;
		if (!line.compare(0, 2, "v ")) {
			float a, b, c;
			iss >> trash >> a >> b >> c;
			std::vector<float> p;
			p.push_back(a);
			p.push_back(b);
			p.push_back(c);
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
	int c = 0, r = 255, g = 255, b = 0;
	for (int i = 0; i < triangle.size(); i++) {
		int ix0 = triangle[i][0];
		int ix1 = triangle[i][1];
		int ix2 = triangle[i][2];		
		TGAColor color = TGAColor(rand(), rand(), rand(), 255);
		fillTriangle(
			points[ix0][0] * width / 2 + width / 2, points[ix0][1] * height / 2 + height / 2,
			points[ix1][0] * width / 2 + width / 2, points[ix1][1] * height / 2 + height / 2,
			points[ix2][0] * width / 2 + width / 2, points[ix2][1] * height / 2 + height / 2, 
			image, color);
	}
}

void drawFile2(int width, int height, TGAImage &image, TGAColor color) {
	std::vector<Point> points;
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
			Point p = Point(a, b, c);
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
	for (int i = 0; i < triangle.size(); i++) {
		int ix0 = triangle[i][0];
		int ix1 = triangle[i][1];
		int ix2 = triangle[i][2];

		// Vecteur p0p1
		Point v1 = Point(
			points[ix1].x - points[ix0].x,
			points[ix1].y - points[ix0].y,
			points[ix1].z - points[ix0].z
		);
		// Vecteur p0p2
		Point v2 = Point(
			points[ix2].x - points[ix0].x,
			points[ix2].y - points[ix0].y,
			points[ix2].z - points[ix0].z
		);

		// Produit vectoriel v1 x v2
		Point pv = v1.produitVectoriel(v2);

		// Produit scalaire    vectCamera . (v1 x v2)
		Point vectCamera = Point(0, 0, 1);
		pv.normalize();
		float produit = vectCamera.norme() * pv.norme() * vectCamera.cosAngle(pv);
		// Si le produit est positif on dessine sinon non 
		if (produit > 0) {
			int teinte = produit * 255;
			TGAColor color = TGAColor(teinte, teinte, teinte, 255);
			fillTriangle(
				points[ix0].x * width / 2 + width / 2, points[ix0].y * height / 2 + height / 2,
				points[ix1].x * width / 2 + width / 2, points[ix1].y * height / 2 + height / 2,
				points[ix2].x * width / 2 + width / 2, points[ix2].y * height / 2 + height / 2,
				image, color);
		}
	}
}

int main(int argc, char** argv) {
	TGAImage image(800, 800, TGAImage::RGB);
	//line(18,14,84,76,image,white);
	//drawTriangle(18,20,60,35,40,84,image,white);
	//fillTriangle(118, 20, 160, 35, 140, 84, image, white);
	//drawTriangle(20, 20, 50, 50, 20, 80, image, white);
	//fillTriangle(120, 20, 150, 50, 120, 80, image, white);
	//drawTriangle(80, 180, 50, 150, 20, 180, image, white);
	//fillTriangle(180, 180, 150, 150, 120, 180, image, white);
	//drawTriangle(80, 220, 50, 250, 80, 280, image, white);
	//fillTriangle(180, 220, 150, 250, 180, 280, image, white);
	//drawTriangle(80, 320, 50, 350, 20, 320, image, white);
	//fillTriangle(180, 320, 150, 350, 120, 320, image, white);
	
	drawFile2(800, 800, image, white);

	//Point p1 = Point(18, 14, 13);
	//Point p2 = Point(60, 35, 84);
	//Point p3 = Point(40, 80, 84);
	//drawTriangle(p1, p2, p3, image, white);

	//fillTriangle(p1, p2, p3, image, white);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

