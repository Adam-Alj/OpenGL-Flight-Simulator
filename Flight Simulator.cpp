/************************************************************************************

FlightSimulator.cpp

Adam Al-Jumaily

*************************************************************************************/

#define _CRT_SECURE_NO_DEPRECATE

/* include the library header files */
#include <stdlib.h>
#include <freeglut.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

/* Definitions */
#define PI 3.14159265
#define DEG_TO_RAD PI/180.0

vector<vector<float>> planeVertices = vector<vector<float>>();
vector<vector<float>> planeNormals = vector<vector<float>>();
vector<vector<int>> planePolygons = vector<vector<int>>();
vector<int> planeColorControl = vector<int>();

vector<vector<float>> propVertices = vector<vector<float>>();
vector<vector<float>> propNormals = vector<vector<float>>();
vector<vector<int>> propPolygons = vector<vector<int>>();
vector<int> propColorControl = vector<int>();


// reshape constants
const int originalWidth = 1500;
const int originalHeight = 1000;

// Scene control variables.
bool wireFrame = false;
bool fullScreen = false;
bool texturedMountains = true;
bool fogEnabled = true;

// Camera position vertex.
float cameraPosition[3] = {
	0, 100, -200
};

// Camera lookat vertex.
float lookAtPosition[3] = {
	0, 0, 0
};

/* 
	[0] X/Y angle.
	[1] Y/Z angle.
*/
float cameraAngles[2] = {
	0, 0
};

float planeSpeed = 3;

float propRotationAngle = 0;
float propRotationInc = 20;

// Increment that the XY lookat will increase by in degrees.
float degreeIncrementXY = 0;

// For drawing spheres.
GLUquadric *quad = gluNewQuadric();

// grid size constant. Number of quads.
const int GRID_SIZE = 100;


// directional light position array.
float lightPosition[] = { -1000, 250, 1000, 1 };
float lightDirection[] = { 1, -.25, -1 };

float zeroMaterial[] = { 0, 0, 0, 1 };
float whiteMaterial[] = { 1, 1, 1, 1 };
float redMaterial[] = { 1, 0, 0, 1 };
float greenMaterial[] = { 0, 1, 0, 1 };
float blueMaterial[] = { 0, 0, 1, 1 };
float yellowMaterial[] = { 1, 1, 0, 1 };
float purpleMaterial[] = { .86, .61, .86, 1 };
float greyMaterial[] = { .2, .2, .2, 1 };
float lightBlueMaterial[] = {.52, .8, 1};
float shine = 100;

GLuint seaText;
GLuint skyText;
GLuint mountText;
float mountTextCoords[80][80][2];

float mountain1[80][80][3];
float mountain2[80][80][3];
float mountain3[80][80][3];
float mountain4[80][80][3];

float mount1Normals[80][80][3];
float mount2Normals[80][80][3];
float mount3Normals[80][80][3];
float mount4Normals[80][80][3];

float fogColor[] = { .86, .61, .86, 1 };

/*
function: parseString

Reads in a string of four tokens delimited by spaces and
returns a pointer to a generated array.

*/
vector<string>* parseString(string line) {
	int lineIndex = 2;
	string substring;
	vector<string>* stringVec = new vector<string>;
	(*stringVec).push_back(string(1, line[0]));
	while(lineIndex > 0){
		substring = line.substr(lineIndex, line.find_first_of(" ", lineIndex) - lineIndex);
		(*stringVec).push_back(substring);
		lineIndex = line.find_first_of(" ", lineIndex) + 1;
	}
	return stringVec;
}

// Dumps controls
void dumpControls() {
	cout << "     CONTROLS     " << endl;
	cout << "PAGEUP:  Increase plane speed." << endl;
	cout << "PAGEDOWN:  Decrease plane speed." << endl;
	cout << "UP:  Move plane up." << endl;
	cout << "DOWN:  Move plane down." << endl;
	cout << "F:  Toggle fullscreen." << endl;
	cout << "W:  Toggle wiremesh." << endl;
	cout << "F:  Toggle fullscreen." << endl;
	cout << "T:  Toggle mountain textures." << endl;
	cout << "B:  Toggle fog." << endl;
	cout << "Q:  Quit application." << endl;
}

// Initializes the propellers.
void initializePropellers() {
	vector<string>* splitLine = nullptr;
	string line;
	int numOfVertices = 0;
	int numOfPolygons = 0;
	int numOfNormals = 0;
	float largestX = 0;
	float smallestX = 0;
	float largestY = 0;
	float smallestY = 0;
	ifstream infile;
	infile.open("propeller.txt");

	if (!infile) {
		std::cout << "cant find it" << endl;
	}

	vector<float> tempVecFloat = vector<float>();
	vector<int> tempVecInt = vector<int>();

	// Fill vectors with vertices/normals/topology info.
	int colorCount = 0;
	while (!infile.eof()) {
		getline(infile, line);
		if (line.length() > 0) {
			splitLine = parseString(line);
			if ((*splitLine)[0] == "v") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					tempVecFloat.push_back(stof((*splitLine)[i]) * 3);
				}
				propVertices.push_back(tempVecFloat);
				tempVecFloat.clear();
				numOfVertices++;
			}
			// propeller.txt file includes double spaces for polygon topology, need to remove them.
			else if ((*splitLine)[0] == "f") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					if ((*splitLine)[i] != "") {
						tempVecInt.push_back(stoi((*splitLine)[i]) - 1);
					}
				}
				propPolygons.push_back(tempVecInt);
				tempVecInt.clear();
				numOfPolygons++;
				colorCount++;
			}
			else if ((*splitLine)[0] == "n") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					tempVecFloat.push_back(stof((*splitLine)[i]));
				}
				propNormals.push_back(tempVecFloat);
				tempVecFloat.clear();
				numOfNormals++;
			}
			else if ((*splitLine)[0] == "g") {
				if (colorCount != 0) {
					propColorControl.push_back(colorCount);
					colorCount = 0;
				}
			}
			delete splitLine;
		}
	}
	propColorControl.push_back(colorCount);
	infile.close();

}


// Loads a texture from a bitmap image. Returns a pointer to the texture id.
GLuint loadTexture(const char * filename, int width, int height)
{

	GLuint texture;

	unsigned char * data;

	FILE * file;

	file = fopen(filename, "rb");

	if (file == NULL) return 0;

	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);
	fclose(file);

	for (int i = 0; i < width * height; ++i)
	{
		int index = i * 3;
		unsigned char B, R;
		B = data[index];
		R = data[index + 2];

		data[index] = R;
		data[index + 2] = B;

	}


	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);


	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	free(data);

	return texture;
}

/* Initializes the textures and fills an array for textcoords */
void initializeTextures() {
	seaText = loadTexture("sea.bmp", 1600, 1200);
	skyText = loadTexture("sky.bmp", 896, 385);
	mountText = loadTexture("mount.bmp", 1280, 1104);
	for (int i = 0; i < 80; i++) {
		for (int j = 0; j < 80; j++) {
			mountTextCoords[i][j][0] = (float)i / 79;
			mountTextCoords[i][j][1] = (float)j / 79;
		}
	}
}

/*
function initializePlane

Reads in from the plane file and fills a vector with information about
vertex location and mesh topology.
*/
void initializePlane() {
	vector<string>* splitLine = nullptr;
	string line;
	int numOfVertices = 0;
	int numOfPolygons = 0;
	int numOfNormals = 0;
	ifstream infile;
	infile.open("cessna.txt");

	if (!infile) {
		std::cout << "cant find it" << endl;
	}

	vector<float> tempVecFloat = vector<float>();
	vector<int> tempVecInt = vector<int>();

	// Fill vectors with vertices/normals/topology info.
	int colorCount = 0;
	while (!infile.eof()) {
		getline(infile, line);
		if (line.length() > 0) {
			splitLine = parseString(line);
			if ((*splitLine)[0] == "v") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					tempVecFloat.push_back(stof((*splitLine)[i]) * 3);
				}
				planeVertices.push_back(tempVecFloat);
				tempVecFloat.clear();
				numOfVertices++;
			}
			// Cessna.txt file includes double spaces for polygon topology, need to remove them.
			else if ((*splitLine)[0] == "f") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					if ((*splitLine)[i] != "") {
						tempVecInt.push_back(stoi((*splitLine)[i])-1);
					}
				}
				planePolygons.push_back(tempVecInt);
				tempVecInt.clear();
				numOfPolygons++;
				colorCount++;
			}
			else if ((*splitLine)[0] == "n") {
				for (int i = 1; i < (*splitLine).size(); i++) {
					tempVecFloat.push_back(stof((*splitLine)[i]));
				}
				planeNormals.push_back(tempVecFloat);
				tempVecFloat.clear();
				numOfNormals++;
			}
			else if ((*splitLine)[0] == "g") {
				if (colorCount != 0) {
					planeColorControl.push_back(colorCount);
					colorCount = 0;
				}
			}
			delete splitLine;
		}
	}
	planeColorControl.push_back(colorCount);
	infile.close();


}

/* 
  Recursive function that raises the height of mountain vertices.
*/
void giveHeight(float mountain[][80][3], int leftIndex, int rightIndex, int topIndex, int botIndex, int iteration = -1) {
	
	if (iteration < 0) {
		iteration = 1;
	}

	int width = abs(rightIndex - leftIndex);
	int height = abs(topIndex - botIndex);

	if (iteration >= 8) {
		return;
	}
	
	mountain[leftIndex + width / 2][topIndex][1] = (mountain[leftIndex][topIndex][1] + mountain[rightIndex][topIndex][1]) / 2;
	mountain[leftIndex + width / 2][botIndex][1] = (mountain[leftIndex][botIndex][1] + mountain[rightIndex][botIndex][1]) / 2 + ((rand()%6)/iteration);
	mountain[leftIndex][topIndex + height / 2][1] = (mountain[leftIndex][topIndex][1] + mountain[leftIndex][botIndex][1]) / 2;
	mountain[rightIndex][topIndex + height/2][1] = (mountain[rightIndex][topIndex][1] + mountain[rightIndex][botIndex][1]) / 2 + ((rand() % 6) / iteration);
	mountain[leftIndex + width / 2][topIndex + height / 2][1] = (mountain[rightIndex][topIndex][1] + mountain[rightIndex][botIndex][1]) / 2 + ((rand() % 6) / iteration+1);

	iteration++;

	giveHeight(mountain, leftIndex, leftIndex + width / 2, topIndex, topIndex + height / 2, iteration);
	giveHeight(mountain, leftIndex, leftIndex + width / 2, topIndex + height / 2, botIndex, iteration);
	giveHeight(mountain, leftIndex + width / 2, rightIndex, topIndex, topIndex + height / 2, iteration);
	giveHeight(mountain, leftIndex + width / 2, rightIndex, topIndex + height / 2, botIndex, iteration);

}

/*
  Initializes the mountains.
*/
void initializeMountains(float mountain[][80][3]) {
	// First we create the vertex for the first point.
	float initialPointX = rand() % 2000 - 1000;
	float initialPointY = 0;
	float initialPointZ = rand() % 2000 - 1000;
	mountain[0][0][0] = initialPointX;
	mountain[0][0][1] = initialPointY;
	mountain[0][0][2] = initialPointZ;

	// Then we fill the array with the other points, all with height 0.
	for (int i = 0; i < 80; i++) {
		for (int j = 0; j < 80; j++) {
			mountain[i][j][0] = initialPointX + j * 3;
			mountain[i][j][1] = 0;
			mountain[i][j][2] = initialPointZ + i * 3;
		}
	}

	// We give the mountain it's peak height
	mountain[39][39][1] = rand()% 25 + 25;

	// Then we raise the y values recursively
	giveHeight(mountain, 0, 39, 0, 39);
	giveHeight(mountain, 39, 79, 0, 39);
	giveHeight(mountain, 0, 39, 39, 79);
	giveHeight(mountain, 39, 79, 39, 79);

}


/*
function: changeCameraPos

linearly modifies the camera and lookat positions.
*/
void changeCameraPos(float posX, float posY, float posZ, float lookX, float lookY, float lookZ) {
	cameraPosition[0] += posX;
	cameraPosition[1] += posY;
	cameraPosition[2] += posZ;

	lookAtPosition[0] += lookX;
	lookAtPosition[1] += lookY;
	lookAtPosition[2] += lookZ;
}

// Draws the mountains with either color or texture.
void drawMountains(float mountain[][80][3]) {
	if (texturedMountains) {
		glColor3f(1, 1, 1);
		glMaterialfv(GL_FRONT, GL_AMBIENT, whiteMaterial);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteMaterial);
		glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);	
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mountText);
	}
	for (int i = 0; i < 79; i++) {
		for (int j = 0; j < 79; j++) {
			glBegin(GL_POLYGON);
			if (texturedMountains) {
				glTexCoord2fv(mountTextCoords[i][j]); glVertex3fv(mountain[i][j]);
				glTexCoord2fv(mountTextCoords[i + 1][j]); glVertex3fv(mountain[i + 1][j]);
				glTexCoord2fv(mountTextCoords[i + 1][j + 1]); glVertex3fv(mountain[i + 1][j + 1]);
				glTexCoord2fv(mountTextCoords[i][j + 1]); glVertex3fv(mountain[i][j + 1]);
			}
			else {
				if (mountain[i][j][1] > 27) {
					glColor3f(1, 1, 1);
					glMaterialfv(GL_FRONT, GL_AMBIENT, whiteMaterial);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteMaterial);
					glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
				}
				else {
					glColor3f(1, 1, 1);
					glMaterialfv(GL_FRONT, GL_AMBIENT, greenMaterial);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, greenMaterial);
					glMaterialfv(GL_FRONT, GL_SPECULAR, greenMaterial);
				}
				glVertex3fv(mountain[i][j]);
				glVertex3fv(mountain[i + 1][j]);
				glVertex3fv(mountain[i + 1][j + 1]);
				glVertex3fv(mountain[i][j + 1]);
			}
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
}

// Draws the plane.
void drawPlane() {
	int polygonCount = 0;
	int polygonSubsetIndex = 0;
	int colorIndex = 0;
	for (int i = 0; i < planePolygons.size(); i++) {
		if (polygonCount == planeColorControl[polygonSubsetIndex]) {
			polygonSubsetIndex++;
			polygonCount = 0;
		}
			if (polygonSubsetIndex < 3) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex > 3 && polygonSubsetIndex < 6) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, zeroMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex == 6) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, purpleMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, purpleMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex == 7) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex > 7 && polygonSubsetIndex < 14) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex > 13 && polygonSubsetIndex < 26) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
			else if (polygonSubsetIndex > 25) {
				glMaterialfv(GL_FRONT, GL_AMBIENT, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowMaterial);
				glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
			}
		
		glBegin(GL_POLYGON);
		for (int j = 0; j < planePolygons[i].size(); j++) {
			glNormal3f(-planeNormals[planePolygons[i][j]][2],
				planeNormals[planePolygons[i][j]][1],
				-planeNormals[planePolygons[i][j]][0]);
			
			glVertex3f(-planeVertices[planePolygons[i][j]][2] + cameraPosition[0],
				planeVertices[planePolygons[i][j]][1] + cameraPosition[1] - 2,
				-planeVertices[planePolygons[i][j]][0] + cameraPosition[2] + 4);

		}
		glEnd();
		polygonCount++;
	}
}

void drawPropeller1() {
	int polygonCount = 0;
	int polygonSubsetIndex = 0;
	int colorIndex = 0;
	for (int i = 0; i < propPolygons.size(); i++) {
		if (polygonCount == propColorControl[polygonSubsetIndex]) {
			colorIndex++;
		}
		if (colorIndex == 0) {
			glMaterialfv(GL_FRONT, GL_AMBIENT, whiteMaterial);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteMaterial);
			glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
		}
		else {
			glMaterialfv(GL_FRONT, GL_AMBIENT, redMaterial);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, redMaterial);
			glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
		}
		glPushMatrix();
		glTranslatef(cameraPosition[0] - 1.05, cameraPosition[1] - 2.45, cameraPosition[2] + 4);
		glRotatef(propRotationAngle, 0, 0, 1);
		glTranslatef(-cameraPosition[0] + 1.05, -cameraPosition[1] + 2.45, -cameraPosition[2] - 4);
		glBegin(GL_POLYGON);
		for (int j = 0; j < propPolygons[i].size(); j++) {
			glNormal3f(-propNormals[propPolygons[i][j]][2],
				propNormals[propPolygons[i][j]][1],
				-propNormals[propPolygons[i][j]][0]);
			glVertex3f(-propVertices[propPolygons[i][j]][2] + cameraPosition[0],
				propVertices[propPolygons[i][j]][1] + cameraPosition[1] - 2,
				-propVertices[propPolygons[i][j]][0] + cameraPosition[2] + 4);
		}
		glEnd();
		glPopMatrix();
		polygonCount++;
	}
}

void drawPropeller2() {
	int polygonCount = 0;
	int polygonSubsetIndex = 0;
	int colorIndex = 0;
	for (int i = 0; i < propPolygons.size(); i++) {
		if (polygonCount == propColorControl[polygonSubsetIndex]) {
			colorIndex++;
		}
		if (colorIndex == 0) {
			glMaterialfv(GL_FRONT, GL_AMBIENT, whiteMaterial);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteMaterial);
			glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
		}
		else {
			glMaterialfv(GL_FRONT, GL_AMBIENT, redMaterial);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, redMaterial);
			glMaterialfv(GL_FRONT, GL_SPECULAR, whiteMaterial);
		}
		glPushMatrix();
		glTranslatef(cameraPosition[0] + 1.05, cameraPosition[1] - 2.45, cameraPosition[2] + 4);
		glRotatef(propRotationAngle, 0, 0, 1);
		glTranslatef(-cameraPosition[0] - 1.05, -cameraPosition[1] + 2.45, -cameraPosition[2] - 4);
		glBegin(GL_POLYGON);
		for (int j = 0; j < propPolygons[i].size(); j++) {
			glNormal3f(-propNormals[propPolygons[i][j]][2],
				propNormals[propPolygons[i][j]][1],
				-propNormals[propPolygons[i][j]][0]);
			glVertex3f(propVertices[propPolygons[i][j]][2] + cameraPosition[0],
				propVertices[propPolygons[i][j]][1] + cameraPosition[1] - 2,
				-propVertices[propPolygons[i][j]][0] + cameraPosition[2] + 4);
		}
		glEnd();
		glPopMatrix();
		polygonCount++;
	}
}

// Draws a cylinder and textures it as the sky.
void drawCylinder() {
	glColor3f(1, 1, 1);
	glMaterialfv(GL_FRONT, GL_EMISSION, whiteMaterial);

	GLUquadric *skyQuad = gluNewQuadric();
	gluQuadricNormals(skyQuad, GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	gluQuadricNormals(skyQuad, GL_SMOOTH);
	gluQuadricTexture(skyQuad, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D, skyText);
	glPushMatrix();
	glTranslatef(0, -20, 0);
	glRotatef(-90, 1, 0, 0);
	gluCylinder(skyQuad, 1250, 1250, 900, 20, 20);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Draws the bottom disk and textures it as the sea.
void drawBottomDisk() {
	GLUquadric *seaQuad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	gluQuadricNormals(seaQuad, GL_SMOOTH);
	gluQuadricTexture(seaQuad, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D, seaText);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	gluDisk(seaQuad, 0, 1275, 50, 4);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_EMISSION, zeroMaterial);
}

void myDisplay(void)
{

	// clear the screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the identity matrix into the model view matrix
	glLoadIdentity();

	// set the camera position
	gluLookAt(
		cameraPosition[0], cameraPosition[1], cameraPosition[2],
		lookAtPosition[0], lookAtPosition[1], lookAtPosition[2],
		0, 1, 0);

	// Draw plane and transform it as necessary.
	glMaterialf(GL_FRONT, GL_SHININESS, 75);
	glPushMatrix();
	glTranslatef(cameraPosition[0], cameraPosition[1] - 2.5, cameraPosition[2]);
	glRotatef(-degreeIncrementXY * 30, 0, 0, 1);
	glTranslatef(-cameraPosition[0], -cameraPosition[1] + 2.5, -cameraPosition[2]);
	drawPlane();
	drawPropeller1();
	drawPropeller2();
	glPopMatrix();
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	
	// Draw scene and transform is as necessary.
	glPushMatrix();
	glTranslatef(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	glRotatef(-cameraAngles[0], 0, 1, 0);
	glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);
	if (wireFrame) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glPushMatrix();
	glTranslatef(0, -10, 0);
	drawMountains(mountain1);
	drawMountains(mountain2);
	drawMountains(mountain3);
	drawMountains(mountain4);
	glPopMatrix();
	drawCylinder();
	if (fogEnabled) {
		glEnable(GL_FOG);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 0.001);
	}
	drawBottomDisk();
	glDisable(GL_FOG);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
	glPopMatrix();


	// swap the drawing buffers
	glutSwapBuffers();
}

/*
function: mouseMove

passive mouse callback function.
Used to move the plane.
*/
void mouseMove(int x, int y) {
	float screenWidth = glutGet(GLUT_WINDOW_WIDTH);
	float centerScreen = screenWidth / 2;

	if (x > centerScreen) {
		degreeIncrementXY = -(((float)x - centerScreen) / 500);
	}
	else if (x < centerScreen) {
		degreeIncrementXY = ((centerScreen - (float)x) / 500);
	}
	glutPostRedisplay();
}

/*
function: specialInput

callback function for special key presses.
Used for plane control from the keyboard.
*/
void specialInput(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_PAGE_UP:
		planeSpeed += 0.1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		planeSpeed -= 0.1;
		break;
	case GLUT_KEY_UP:
		changeCameraPos(0, 5, 0, 0, 5, 0);
		break;
	case GLUT_KEY_DOWN:
		changeCameraPos(0, -5, 0, 0, -5, 0);
		break;
	}

	if (planeSpeed < 2) {
		planeSpeed = 2;
	}

	glutPostRedisplay();
}

/*
function: keyboard

callback function for key presses.
Used for scene control from the keyboard.
*/
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'W':
	case 'w':
		wireFrame = !wireFrame;
		break;
	case 'T':
	case 't':
		texturedMountains = !texturedMountains;
		break;
	case 'B':
	case 'b':
		fogEnabled = !fogEnabled;
		break;
	case 'F':
	case 'f':
		if (fullScreen) {
			glViewport(0, 0, originalWidth, originalHeight);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(90, (float)originalWidth / (float)originalHeight, 0.01, 5000);
			glMatrixMode(GL_MODELVIEW);
			fullScreen = !fullScreen;
			glutLeaveFullScreen();
		}
		else {
			glutFullScreen();
			fullScreen = !fullScreen;
		}
		break;
	case 'Q':
	case 'q':
		exit(0);
	}
	glutPostRedisplay();
}

// Simple reshape function.
void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, (float)width / (float)height, 0.01, 5000);
	glMatrixMode(GL_MODELVIEW);
}


// Idle function that controls the camera angle, plane speed, and propellor rotation.
void idle() {
	cameraAngles[0] += degreeIncrementXY;
	if (cameraAngles[0] > 360) {
		cameraAngles[0] = fmod(cameraAngles[0], 360);
	}
	else if (cameraAngles[0] < 0) {
		cameraAngles[0] = 360 + cameraAngles[0];
	}
	cameraPosition[0] += planeSpeed*sin(DEG_TO_RAD*cameraAngles[0]);
	lookAtPosition[0] += planeSpeed*sin(DEG_TO_RAD*cameraAngles[0]);
	cameraPosition[2] += planeSpeed*cos(DEG_TO_RAD*cameraAngles[0]);
	lookAtPosition[2] += planeSpeed*cos(DEG_TO_RAD*cameraAngles[0]);

	propRotationInc = 20 + planeSpeed * 2;
	if (propRotationInc > 178) {
		propRotationInc = 178;
	}
	propRotationAngle += propRotationInc;
	if (propRotationAngle >= 360) {
		propRotationAngle = 0;
	}

	glutPostRedisplay();
}


/************************************************************************

Function:		initializeGL

Description:	Initializes the OpenGL rendering context for display.

*************************************************************************/
void initializeGL(void)
{

	// define the light color and intensity
	GLfloat ambientLight[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuseLight[] = { 1, 1, 1, 1 };
	GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };

	//  the global ambient light level
	GLfloat globalAmbientLight[] = { 0.4, 0.4, 0.4, 1.0 };

	// set the global ambient light level
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// define the color and intensity for light 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuseLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, specularLight);

	// enable lighting 
	glEnable(GL_LIGHTING);
	// enable light 0
	glEnable(GL_LIGHT0);

	// enable smooth shading.
	glShadeModel(GL_SMOOTH);

	// make sure the normals are unit vectors
	glEnable(GL_NORMALIZE);

	// Initialize plane vectors.
	initializePlane();

	// Initialize propeller.
	initializePropellers();

	// Initialize textures.
	initializeTextures();

	// Initialize mountains.
	initializeMountains(mountain1);
	initializeMountains(mountain2);
	initializeMountains(mountain3);
	initializeMountains(mountain4);



	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// set background color to be black
	glClearColor(0, 0, 0, 1.0);

	// change into projection mode so that we can change the camera properties
	glMatrixMode(GL_PROJECTION);

	// load the identity matrix into the projection matrix
	glLoadIdentity();

	// set window mode to 2D orthographic 
	gluPerspective(90, originalWidth/originalHeight, 0.01, 5000);

	// change into model-view mode so that we can change the object positions
	glMatrixMode(GL_MODELVIEW);
}

/************************************************************************

Function:		main

Description:	Sets up the openGL rendering context and the windowing
system, then begins the display loop.

*************************************************************************/
void main(int argc, char** argv)
{
	// initialize the toolkit
	glutInit(&argc, argv);
	// set display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// set window size
	glutInitWindowSize(originalWidth, originalHeight);
	// set window position on screen
	glutInitWindowPosition(100, 0);
	// open the screen window
	glutCreateWindow("Flight Simulator - A00362836");
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// dump controls
	dumpControls();
	// register redraw function
	glutDisplayFunc(myDisplay);
	// register special function.
	glutSpecialFunc(specialInput);
	// register passive motion function.
	glutPassiveMotionFunc(mouseMove);
	// register Idle function.
	glutIdleFunc(idle);
	// register keyboard function.
	glutKeyboardFunc(keyboard);
	// register reshape function.
	glutReshapeFunc(reshape);
	// initialize the rendering context
	initializeGL();
	// seed random
	srand(time(NULL));

	// go into a perpetual loop
	glutMainLoop();
}

