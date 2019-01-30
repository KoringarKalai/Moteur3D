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

void fillTriangle(VectF p0, VectF p1, VectF p2, TGAImage &image, TGAImage &imageTexture, VectF c0, VectF c1, VectF c2, float teinte, float **zbuffer) {
	int xmin = std::min(std::min(p0.x,p1.x),p2.x);
	int xmax = std::max(std::max(p0.x, p1.x), p2.x);
	int ymin = std::min(std::min(p0.y, p1.y), p2.y);
	int ymax = std::max(std::max(p0.y, p1.y), p2.y);
	VectF P;
	for (P.x = xmin; P.x <= xmax; P.x++) {
		for (P.y = ymin; P.y <= ymax; P.y++) {
			VectF bary = barycentric(p0, p1, p2, P);
			if (bary.x < 0 || bary.y < 0 || bary.z < 0) continue;
			P.z = 0;
			P.z += p0.z * bary.x + p1.z * bary.y + p2.z * bary.z;
			if (zbuffer[(int)P.x][(int)P.y] < P.z) {
				zbuffer[(int)P.x][(int)P.y] = P.z;
				float xc = c0.x * bary.x + c1.x * bary.y + c2.x * bary.z;
				float yc = c0.y * bary.x + c1.y * bary.y + c2.y * bary.z;
				TGAColor couleur = imageTexture.get(xc * 1024 , yc * 1024);
				for (int i = 0; i < 3; i++) couleur.raw[i] *= teinte;
				image.set((int)P.x, (int)P.y, couleur);
			}
		}
	}
}

void drawFile(int width, int height, int profondeur, TGAImage &image, TGAColor color) {
	std::vector<VectF> points;
	std::vector<VectF> colors;
	std::vector<std::vector<float>> triangle;
	std::vector<std::vector<float>> texture;
	TGAImage imageTexture = TGAImage();
	//std::ifstream infile("obj/african_head/african_head.obj");
	//imageTexture.read_tga_file("obj/african_head/african_head_diffuse.tga");
	std::ifstream infile("obj/diablo3_pose/diablo3_pose.obj");
	imageTexture.read_tga_file("obj/diablo3_pose/diablo3_pose_diffuse.tga");
	imageTexture.flip_vertically();
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
		} else if (!line.compare(0, 3, "vt ")) {
			float a, b, c;
			iss >> trash >> trash >> a >> b >> c;
			VectF p = VectF(a, b, c);
			colors.push_back(p);
			i++;
		} else if (!line.compare(0, 2, "f ")) {
			iss >> trash;
			std::vector<float> t;
			std::vector<float> tT;
			int num, num2 ,rien;
			while (iss >> num >> trash >> num2 >> trash >> rien) {
				float v = num - 1;
				t.push_back(v);
				v = num2 - 1;
				tT.push_back(v);
			}
			triangle.push_back(t);
			texture.push_back(tT);
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
			float teinte = produit;
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

			int ix0T = texture[i][0];
			int ix1T = texture[i][1];
			int ix2T = texture[i][2];
			VectF c0 = VectF(colors[ix0T].x, colors[ix0T].y, colors[ix0T].z);
			VectF c1 = VectF(colors[ix1T].x, colors[ix1T].y, colors[ix1T].z);
			VectF c2 = VectF(colors[ix2T].x, colors[ix2T].y, colors[ix2T].z);
			fillTriangle(p0, p1, p2, image, imageTexture, c0, c1, c2, teinte, zbuffer);
		}
	}
}

int main(int argc, char** argv) {
	int h = 2000, w = 2000, p = 2000;
	TGAImage image(h, w, TGAImage::RGB);
	drawFile(h, w, p, image, white);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

