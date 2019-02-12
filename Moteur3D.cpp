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
	VectF cp = crossProduct(v1, v2);
	if (std::abs(cp.z) > 1e-2) return VectF(1.f - (cp.x + cp.y) / cp.z, cp.y / cp.z, cp.x / cp.z);
	return VectF(-1, 1, 1);
}

struct Matrix {
	std::vector<std::vector<float>> m;
	int row, col;

	Matrix(int r, int c) : m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), row(r), col(c) { }

	void identity() {
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < col; j++) {
				if (i == j)
					m[i][j] = 1;
				else 
					m[i][j] = 0;
			}
		}
	}

	void rotX(float alpha) {
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < col; j++) {
				if (i == j)
					m[i][j] = 1;
				else
					m[i][j] = 0;
			}
		}
		m[0][0] = std::cos(alpha);
		m[1][0] = std::sin(alpha);
		m[1][1] = std::cos(alpha);
		m[0][1] = -std::sin(alpha);
	}

	void rotZ(float alpha) {
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < col; j++) {
				if (i == j)
					m[i][j] = 1;
				else
					m[i][j] = 0;
			}
		}
		m[1][1] = std::cos(alpha);
		m[2][1] = std::sin(alpha);
		m[2][2] = std::cos(alpha);
		m[1][2] = -std::sin(alpha);
	}

	void rotY(float alpha) {
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < col; j++) {
				if (i == j)
					m[i][j] = 1;
				else
					m[i][j] = 0;
			}
		}
		m[0][0] = std::cos(alpha);
		m[2][0] = -std::sin(alpha);
		m[2][2] = std::cos(alpha);
		m[0][2] = std::sin(alpha);
	}
};

Matrix mult(Matrix m1, Matrix m2) {
	Matrix res = Matrix(m1.row, m2.col);
	for (int i = 0; i < m1.row; i++) {
		for (int j = 0; j < m2.col; j++) {
			res.m[i][j] = 0.f;
			for (int k = 0; k < m1.row; k++) {
				res.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return res;
}

Matrix lookAt(VectF eye, VectF center, VectF v_up) {
	VectF z = VectF(eye.x - center.x, eye.y - center.y, eye.z - center.z);
	z.normalize();
	VectF x = crossProduct(v_up, z);
	x.normalize();
	VectF y = crossProduct(z, x);
	y.normalize();

	Matrix res = Matrix(4, 4);
	res.identity();
	
	res.m[0][0] = x.x;
	res.m[0][1] = x.y;
	res.m[0][2] = x.z;
	res.m[1][0] = y.x;
	res.m[1][1] = y.y;
	res.m[1][2] = y.z;
	res.m[2][0] = z.x;
	res.m[2][1] = z.y;
	res.m[2][2] = z.z;
	res.m[0][3] = -center.x;
	res.m[1][3] = -center.y;
	res.m[2][3] = -center.z;

	return res;
}

Matrix viewPort(int x, int y, int w, int h, int d) {
	Matrix m = Matrix(4,4);
	m.identity();
	m.m[0][3] = x + w / 2.f;
	m.m[1][3] = y + h / 2.f;
	m.m[2][3] = d / 2.f;

	m.m[0][0] = w / 2.f;
	m.m[1][1] = h / 2.f;
	m.m[2][2] = d / 2.f;
	return m;
}

Matrix v2m(VectF v) {
	Matrix m = Matrix(4, 1);
	m.m[0][0] = v.x;
	m.m[1][0] = v.y;
	m.m[2][0] = v.z;
	m.m[3][0] = 1;
	return m;
}

VectF m2v(Matrix m) {
	return VectF(m.m[0][0] / m.m[3][0], m.m[1][0] / m.m[3][0], m.m[2][0] / m.m[3][0]);
}

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

int anaglyph;
int h = 800, w = 800, p = 255;
VectF eye;

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

void fillTriangle(VectF p0, VectF p1, VectF p2, TGAImage &image, TGAImage &imageTexture, TGAImage &imageNm, VectF c0, VectF c1, VectF c2, float **zbuffer) {
	int xmin = std::min(std::min(p0.x,p1.x),p2.x);
	xmin = std::max(0, xmin);
	int xmax = std::max(std::max(p0.x, p1.x), p2.x);
	xmax = std::min(w-1, xmax);
	int ymin = std::min(std::min(p0.y, p1.y), p2.y);
	ymin = std::max(0, ymin);
	int ymax = std::max(std::max(p0.y, p1.y), p2.y);
	ymax = std::min(h-1, ymax);
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
				TGAColor colorNm = imageNm.get(xc * 1024, yc * 1024);

				VectF color = VectF((float)colorNm.r /128 - 1, (float)colorNm.g / 128 - 1, (float)colorNm.b / 128 - 1);
				VectF lumiere = eye;
				lumiere.normalize();

				float intensity = (lumiere.x * color.x + lumiere.y * color.y + lumiere.z * color.z);

				intensity = std::max(std::min(intensity, 1.f), 0.f);
				for (int i = 0; i < 3; i++) couleur.raw[i] *= intensity;
				
				if (anaglyph > 0) {
					// Anaglyph
					if (anaglyph == 1) {
						TGAColor oldColor = image.get((int)P.x, (int)P.y);
						oldColor.raw[2] = couleur.r;
						image.set((int)P.x, (int)P.y, oldColor);
					}
					else {
						TGAColor oldColor = image.get((int)P.x, (int)P.y);
						oldColor.raw[0] = couleur.b;
						oldColor.raw[1] = couleur.g;
						image.set((int)P.x, (int)P.y, oldColor);
					}
				} else {
					// Normal 
					image.set((int)P.x, (int)P.y, couleur);
				}
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
	TGAImage imageNm = TGAImage();
	TGAImage imageSpec = TGAImage();

	//std::ifstream infile("obj/african_head/african_head.obj");
	//imageTexture.read_tga_file("obj/african_head/african_head_diffuse.tga");
	//imageNm.read_tga_file("obj/african_head/african_head_nm.tga");

	std::ifstream infile("obj/diablo3_pose/diablo3_pose.obj");
	imageTexture.read_tga_file("obj/diablo3_pose/diablo3_pose_diffuse.tga");
	imageNm.read_tga_file("obj/diablo3_pose/diablo3_pose_nm.tga");
	//imageSpec.read_tga_file("obj/diablo3_pose/diablo3_pose_spec.tga");
	
	imageTexture.flip_vertically();
	imageNm.flip_vertically();
	imageSpec.flip_vertically();
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
		VectF p0 = VectF(
			(points[ix0].x), 
			(points[ix0].y),
			(points[ix0].z));
		VectF p1 = VectF(
			(points[ix1].x),
			(points[ix1].y),
			(points[ix1].z));
		VectF p2 = VectF(
			(points[ix2].x),
			(points[ix2].y),
			(points[ix2].z));

		int ix0T = texture[i][0];
		int ix1T = texture[i][1];
		int ix2T = texture[i][2];
		VectF c0 = VectF(colors[ix0T].x, colors[ix0T].y, colors[ix0T].z);
		VectF c1 = VectF(colors[ix1T].x, colors[ix1T].y, colors[ix1T].z);
		VectF c2 = VectF(colors[ix2T].x, colors[ix2T].y, colors[ix2T].z);

		float eyesep = 0.1;
		eye = VectF(0 - eyesep, 0, 2);
		if (anaglyph == 1)
			eye = VectF(eye.x + eyesep, eye.y, eye.z);
		VectF center = VectF(0, 0, 0);
		Matrix ModelView = lookAt(eye,center, VectF(0, 1, 0));
		VectF n = VectF(eye.x - center.x, eye.y - center.y, eye.z - center.z);
		Matrix Projection = Matrix(4, 4);
		Projection.identity();
		Projection.m[3][2] = -1.f / n.norme();
		Matrix ViewPort = viewPort(width / 8, height / 8, width * 3 / 4, height * 3 / 4, profondeur * 3 / 4);
		
		Matrix rot = Matrix(4,4);
		rot.identity();
		Matrix rotX = Matrix(4, 4);
		rotX.rotX(45 * 3.14 / 180);
		Matrix rotY = Matrix(4, 4);
		rotY.rotY(45 * 3.14 / 180);
		Matrix rotZ = Matrix(4, 4);
		rotZ.rotZ(45 * 3.14 / 180);

		//rot = mult(rot, rotX);
		//rot = mult(rot, rotY);
		//rot = mult(rot, rotZ);

		Matrix vpp = mult(ViewPort, Projection);
		Matrix vppvm = mult(vpp, ModelView);
		Matrix vppvmrot = mult(vppvm, rot);
		p0 = m2v(mult(vppvmrot, v2m(p0)));
		p1 = m2v(mult(vppvmrot, v2m(p1)));
		p2 = m2v(mult(vppvmrot, v2m(p2)));
		
		fillTriangle(p0, p1, p2, image, imageTexture, imageNm, c0, c1, c2, zbuffer);
	}
}

int main(int argc, char** argv) {
	TGAImage image(w, h, TGAImage::RGB);
	anaglyph = 1;
	drawFile(h, w, p, image, white);
	
	if (anaglyph > 0) {
		anaglyph = 2;
		drawFile(h, w, p, image, white);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

