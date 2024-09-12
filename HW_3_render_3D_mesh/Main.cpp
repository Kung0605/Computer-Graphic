#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "./Library/glut/glut.h"
#include "./Library/SOIL/include/SOIL.h"

#pragma comment( lib, "./Library/glut/glut32.lib" )
#pragma comment( lib, "./Library/SOIL/lib/SOIL.lib" )
#define KEY_LEFT 100
#define KEY_UP 101
#define KEY_RIGHT 102
#define KEY_DOWN 103

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
bool firstTime = true;
int startX, startY;
float light_pos_0[4] = { 1, 1, 1, 0 };
float light_pos_1[4] = { -1, 1, 1, 0 };
float light_pos_2[4] = { 0, -1, 0, 0 };
std::vector<float*> vertices;
std::vector<int*> faces;
const float Step = 1;
const float dino_size = 116;
const int winWidth = 800, winHeight = 600;

// Trackball parameters initialization 
float angle = 0.0, axis[3], trans[3];

bool trackingMouse = false;
bool redrawContinue = false;
bool trackballMove = false;

float lastPos[3] = { 0.0, 0.0, 0.0 };
int curx, cury;



// Translation & Rotation
float x_trans = 0.0f; // translate object in x direction
float y_trans = 0.0f; // translate object in y direction
float x_fullTrans = 0.0f;	// total translation in x direction
float y_fullTrans = 0.0f;	// total translation in y direction
float zoom = 1.0f; // zoom for scaling


void cross(float* v1, float* v2, float* vec) {
	vec[0] = v1[1] * v2[2] - v1[2] * v2[1];
	vec[1] = v1[2] * v2[0] - v1[0] * v2[2];
	vec[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void get_normal(float* p1, float* p2, float* p3, float norm[3]) {
	float v1[3] = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
	float v2[3] = { p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2] };
	cross(v1, v2, norm);
	float length = sqrtf(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]);
	norm[0] /= length;
	norm[1] /= length;
	norm[2] /= length;
}

void load_dino(const char* path) {
	FILE* file;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		printf("File not found\n");
		return;
	}
	vertices.push_back(new float[3]);
	while (1) {
		char lh;
		int res = fscanf_s(file, "%c", &lh, 1);
		if (res == EOF)
			break;
		if (lh == 'v') {
			float* vec = new float[3];
			fscanf_s(file, "%f %f %f\n", &vec[0], &vec[1], &vec[2]);
			vec[0] /= dino_size; vec[1] /= dino_size; vec[2] /= dino_size;
			vertices.push_back(vec);
		}
		else if (lh == 'f') {
			int* vec = new int[3];
			fscanf_s(file, "%d %d %d\n", &vec[0], &vec[1], &vec[2]);
			faces.push_back(vec);
		}
	}
}

void Init(int w, int h)
{
	float aspectRatio = (float)w / (float)h;
	glViewport(0, 0, w, h);
	glShadeModel(GL_SMOOTH);								// Set Smooth Shading 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);			    	// Background Color 
	glClearDepth(1.0f);										// Depth buffer setup 
	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing 
	glDepthFunc(GL_LEQUAL);									// The Type Of Depth Test To Do 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Use perspective correct interpolation if available

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w < h)
		glFrustum(-1, 1, -1 / aspectRatio, 1 / aspectRatio, 0, 100);
	else
		glFrustum(-aspectRatio, aspectRatio, -1, 1, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();							// Reset The Modelview Matrix
	// set light position
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos_0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_pos_1);
	glLightfv(GL_LIGHT2, GL_POSITION, light_pos_2);
	if (firstTime)
	{
		// set light
		float ambient0[] = { 1, 0, 1, 1 };
		float ambient1[] = { 0, 1, 1, 1 };
		float ambient2[] = { 0, 0, 1, 1 };
		float diffuse0[] = { 0.5, 0.5, 0.5, 1 };
		float diffuse1[] = { 0, 0.5, 0, 1 };
		float diffuse2[] = { 0.5, 0.1, 0.1, 1 };
		float specular0[] = { 0.1, 0, 0.1, 1 };
		float specular1[] = { 0, 0.1, 0.1, 1 };
		float specular2[] = { 0.1, 0.1, 0.1, 1 };
		glEnable(GL_LIGHTING);

		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.0);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.5);

		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
		glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 2.0);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 2.5);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.5);

		glEnable(GL_LIGHT2);
		glLightfv(GL_LIGHT2, GL_AMBIENT, ambient2);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse2);
		glLightfv(GL_LIGHT2, GL_SPECULAR, specular2);
		glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.5);
		glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 1.0);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 1.0);
		firstTime = false;
	}
}


void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	GLfloat objectform[16];							//Array to save matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, objectform);	//Save matrix for future use
	glLoadIdentity();								//Load identity matrix to apply current transformations
	glTranslatef(x_fullTrans + x_trans, y_fullTrans + y_trans, 0.0f);	//Move object back to original position, plus current translation
	glScalef(zoom, zoom, zoom);						//Scale object
	glRotatef(angle, axis[0], axis[1], axis[2]);	//Rotate object by rotation angle around rotation axis
	glTranslatef(-x_fullTrans, -y_fullTrans, 0.0f);	//Translate object back to origin
	glMultMatrixf(objectform);						//Load matrix to preserve previous transformations

	// load dino
	load_dino("./Dino.obj");
	vertices.push_back(nullptr);
	glBegin(GL_TRIANGLES);
	glColor3f(1, 1, 1);
	for (auto& face : faces) {
		float* p1 = vertices[face[0]];
		float* p2 = vertices[face[1]];
		float* p3 = vertices[face[2]];
		float norm[3];
		get_normal(p1, p2, p3, norm);
		glNormal3fv(norm);
		glVertex3fv(p1);
		glVertex3fv(p2);
		glVertex3fv(p3);
	}
	glEnd();

	glFlush();
	glutSwapBuffers();

	x_fullTrans += x_trans;		//Add current translation to total translation
	y_fullTrans += y_trans;		
	x_trans = 0.0f;				//Reset current translation
	y_trans = 0.0f;
	zoom = 1.0f;				//Reset scaling
	angle = 0.0f;				//Reset rotation angle
}


void mouseMotion(int x, int y)
{

	if (trackingMouse) //If the left button has been clicked
	{
		float curPos[3], dx, dy, dz;
		float d, norm;
		curPos[0] = (2.0f * x - winWidth) / winWidth; //Calculate x component for vector at mouse's current position
		curPos[1] = (winHeight - 2.0f * y) / winHeight; //Calculate y component for vector at mouse's current position
		d = sqrtf(curPos[0] * curPos[0] + curPos[1] * curPos[1]); //Calculate z component
		d = (d < 1.0f) ? d : 1.0f; //Project vector onto surface of trackball
		curPos[2] = sqrtf(1.001f - d * d); //Calculate z component
		norm = 1.0 / sqrt(curPos[0] * curPos[0] + curPos[1] * curPos[1] + curPos[2] * curPos[2]);
		curPos[0] *= norm; //Normalize vecor
		curPos[1] *= norm;
		curPos[2] *= norm;
		dx = curPos[0] - lastPos[0]; //Check if mouse has moved from last position
		dy = curPos[1] - lastPos[1];
		dz = curPos[2] - lastPos[2];
		if (dx || dy || dz) //If mouse has moved
		{
			angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz); //Calculate rotation angle
			axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1]; //Calculate rotation axis
			axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
			axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];
			lastPos[0] = curPos[0]; //Set the last position to the current
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
	}
	
	glutPostRedisplay();
}




void mouseButton(int button, int state, int x, int y)
{
	//Detect mouse click
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		trackingMouse = true; //Set boolean so that future mouse movements are tracked

		float d, norm;
		lastPos[0] = (2.0f * x - winWidth) / winWidth; //Calculate the x and y components of the initial movement vector
		lastPos[1] = (winHeight - 2.0f * y) / winHeight;
		d = sqrtf(lastPos[0] * lastPos[0] + lastPos[1] * lastPos[1]); //Calculate the z component of the vector using pythagorean theorem
		d = (d < 1.0f) ? d : 1.0f; //If the z component is not on the surface of the trackball, set it to 1: the radius of the trackball
		lastPos[2] = sqrtf(1.001f - d * d);
		norm = 1.0 / sqrt(lastPos[0] * lastPos[0] + lastPos[1] * lastPos[1] + lastPos[2] * lastPos[2]); //Calculate vector length
		lastPos[0] *= norm; //Normalize vector
		lastPos[1] *= norm;
		lastPos[2] *= norm;
	}
	else
		trackingMouse = false; //If button not clicked, set boolean so that no mouse movement is tracked
}





int main(int argc, char* argv[])
{
	// glut initialization functions:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ImageViewer");

	Init(winWidth, winHeight);

	// display, onMouseButton, mouse_motion, onKeyPress, and resize are functions defined above
	glutDisplayFunc(Draw);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(Init);


	// start glutMainLoop -- infinite loop
	glutMainLoop();

	return 0;
}