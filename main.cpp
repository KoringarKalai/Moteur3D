#include "tgaimage.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (y1>y0?1:-1); 
            error2 -= dx*2; 
        } 
    } 
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
    line(x0,y0,x1,y1,image,color);
    line(x0,y0,x2,y2,image,color);
    line(x2,y2,x1,y1,image,color);
}

void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color) {
	// (x0,y0) et (x1,y1) forme le plus grand segment (hypotenus)
	if (std::abs(y0-y1)<std::abs(y0-y2)) { 
        std::swap(x1, x2); 
        std::swap(y1, y2); 
    }
    // (x0,y0) est plus bas que (x1,y1)
    if (y0 > y1) {
    	std::swap(x0, x1); 
        std::swap(y0, y1);
    }
    for (int y = y0; y < y1; y++) {
    	float t = (y-y0)/(float)(y1-y0); 
        int xg = x0*(1.-t) + x1*t; 
    	t = (y-y0)/(float)(y2-y0); 
    	int xd = x0*(1.-t) + x2*t;
    	if (xd < xg) {
        	std::swap(xg, xd);     		
    	}
    	for (int x = xg; x < xd; x++) {
    		image.set(x, y, color);
    	}
    }
}

void drawFile(int width, int height, TGAImage &image, TGAColor color) {
    float** points = new float*[3];
    for(int i = 0; i < 3; ++i) {
      points[i] = new float[6000];
    }
    int** triangle = new int*[3];
    for(int i = 0; i < 3; ++i) {
      triangle[i] = new int[6000];
    }
    std::ifstream infile("african_head.obj");
    std::string line;
    int i = 0;
    int j = 0;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      
      char trash;
      if (!line.compare(0, 2, "v ")) {
      	float a,b,c;
	iss >> trash >> a >> b >> c;
	points[0][i] = a;
	points[1][i] = b;
	points[2][i] = c;
	i++;
      } else if (!line.compare(0, 2, "f ")) {
	iss >> trash;
	int k = 0;
	int num,rien;
	while (iss >> num >> trash >> rien >> trash >> rien) {
	  triangle[k][j] = num - 1;
	  k++;
	}
	j++;
      }
    }
    for (int k = 0; k < 6000; k++) {
      int ix0 = triangle[0][k];
      int ix1 = triangle[1][k];
      int ix2 = triangle[2][k];
      drawTriangle(points[0][ix0]*width/2+width/2, points[1][ix0]*height/2+height/2, 
		   points[0][ix1]*width/2+width/2, points[1][ix1]*height/2+height/2, 
		   points[0][ix2]*width/2+width/2, points[1][ix2]*height/2+height/2, image,color);
    }
}

int main(int argc, char** argv) {
	TGAImage image(800, 800, TGAImage::RGB);
	//line(18,14,84,76,image,white);
	//triangle(18,20,60,35,40,84,image,white);
	drawFile(800,800,image,white);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

