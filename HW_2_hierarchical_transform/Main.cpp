#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <conio.h>

#include "./Library/glut/glut.h"
#include "./Library/SOIL/include/SOIL.h"

#pragma comment( lib, "./Library/glut/glut32.lib" )
#pragma comment( lib, "./Library/SOIL/lib/SOIL.lib" )

#define LEFT 0
#define RIGHT 1

using namespace std;

int spinX = 0;
int spinY = 0;
int des = 0;
int mov = 0;
int theta_x = 0, theta_y = 0, theta_z = 0;
int angle_x = 0, angle_y = 0, angle_z = 0;

struct position {
	double x;
	double y;
	double z;
};
position head{ 0, 4, 10 };
position body{ 0, -1.2, 10 };
position left_shoulder{ 4.5, -1.3, 10 };
position right_shoulder{ -4.5, -1.3, 10 };
position right_arm{ -4.5, -3.8, 10 };
position left_arm{ 4.5, -3.8, 10 };
position right_hand{ -4.5, -6.4, 10 };
position left_hand{ 4.5, -6.4, 10 };
position left_leg{ 1, -6.0, 10 };
position left_buttom{ 1, -5.5, 10 };
position right_leg{ -1, -6.0, 10 };
position right_buttom{ -1, -5.5, 10 };
position left_feet{ 1.2, -7, 10 };
position right_feet{ -1.2, -7, 10 };
bool hand_move_x, hand_move_y, hand_move_z;
bool leg_spin_x, leg_spin_y, leg_spin_z;
bool backward = false;

void TimerFunc(int value) {
	if (hand_move_z) 
		theta_z = theta_z > 360 ? 0 : theta_z + 2;
	if (hand_move_y) 
		theta_y = theta_y > 360 ? 0 : theta_y + 2;
	if (hand_move_x) 
		theta_x = theta_x > 360 ? 0 : theta_x + 2;
	if (leg_spin_z) 
		angle_z = angle_z > 360 ? 0 : angle_z + 2;
	if (leg_spin_y)
		angle_y = angle_y > 360 ? 0 : angle_y + 2;
	if (leg_spin_x) {
		if (angle_x > 45) 
			backward = true;
		if (angle_x < -45) 
			backward = false;
		angle_x = backward ? angle_x - 2 : angle_x + 2;
	}
	glutPostRedisplay();
	glutTimerFunc(20, TimerFunc, 1);
}
void drawBall(double R, position pos, double x_size = 1, double y_size = 1, double z_size = 1) {
	glPushMatrix();
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;
	glTranslated(x, y, z);
	glScalef(x_size, y_size, z_size);
	glutSolidSphere(R, 20, 20);
	glPopMatrix();
}

void init() {
	GLfloat position[] = { -80.0, 50.0, 25.0, 1.0 };
	GLfloat lmodel_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat ambient[] = { 0.5, 0.8, 0.1, 0.1 };
	GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat local_view[] = { 0.0 };
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	GLfloat mat_ambients[] = { 0.5, 0.5, 0.5, 1.0 };   
	GLfloat mat_diffuses[] = { 0.3, 0.3, 0.3, 1.0 };  
	GLfloat mat_speculars[] = { 1.0, 1.0, 1.0, 1.0 }; 
	GLfloat mat_shininesss[] = { 75.0 };    

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambients);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuses);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_speculars);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininesss);
	glutTimerFunc(20, TimerFunc, 1);
}

void drawArm(position pos, position spin) {
	glPushMatrix();
	glTranslatef(spin.x, spin.y, spin.z);
	glRotatef(-theta_z, 0, 0, 1);
	glRotatef(-theta_y, 0, 1, 0);
	glRotatef(theta_x - 30, 1, 0, 0);
	glTranslatef(-spin.x, -spin.y, -spin.z);
	drawBall(1, pos, 1.0, 3.5, 1.0);
	glPopMatrix();
}
void drawHand(position pos, position spin) {
	glPushMatrix();
	glTranslatef(spin.x, spin.y, spin.z);
	glRotatef(-theta_z, 0, 0, 1);
	glRotatef(-theta_y, 0, 1, 0);
	glRotatef(theta_x - 30, 1, 0, 0);
	glTranslatef(-spin.x, -spin.y, -spin.z);
	drawBall(1, pos);
	glPopMatrix();
}
void drawLeg(position pos, position spin, int direction) {
	glPushMatrix();
	glTranslatef(spin.x, spin.y, spin.z);
	glRotatef((direction == LEFT ? angle_z + 10 : angle_z - 10), 0, 0, 1);
	glRotatef(angle_y, 0, 1, 0);
	glRotatef((direction == LEFT ? -angle_x : angle_x), 1, 0, 0);
	glTranslatef(-spin.x, -spin.y, -spin.z);
	drawBall(1, pos, 1.2, 2.0, 2.0);
	glPopMatrix();
}
void drawFeet(position pos, position spin, int direction) {
	glPushMatrix();
	glTranslatef(spin.x, spin.y, spin.z);
	glRotatef((direction == LEFT ? angle_z + 10 : angle_z - 10), 0, 0, 1);
	glRotatef(angle_y, 0, 1, 0);
	glRotatef((direction == LEFT ? -angle_x : angle_x), 1, 0, 0);
	glTranslatef(-spin.x, -spin.y, -spin.z);
	drawBall(1.5, pos, 1.2, 1.2, 1.2);
	glPopMatrix();
}
void reset() {
	theta_x = 0, theta_y = 0, theta_z = 0;
	angle_x = 0, angle_y = 0, angle_z = 0;
	hand_move_x = false, hand_move_y = false, hand_move_z = false;
	leg_spin_x = false, leg_spin_y = false, leg_spin_z = false;
	backward = false;
}
bool clapped = false;
bool help = false;
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glLoadIdentity();
	gluLookAt(-2.0, -5.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	if (help) {
		//load "help.png" as texture and display it on the screen
		GLuint texture = SOIL_load_OGL_texture("./Images/help.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);	
		glEnable(GL_TEXTURE_2D);	
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f(-10, -10, 0);
		glTexCoord2f(1, 0); glVertex3f(10, -10, 0);
		glTexCoord2f(1, 1); glVertex3f(10, 10, 0);
		glTexCoord2f(0, 1); glVertex3f(-10, 10, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);

	}
	else {
		glPushMatrix();
		glRotated(spinX, 0, 1, 0);
		glRotated(spinY, 1, 0, 0);
		glTranslated(0, 0, des);
		drawBall(2, head, 2.5, 2.5, 2.5);
		drawBall(4, body);
		drawBall(1.3, left_shoulder);
		drawBall(1.3, right_shoulder);
		drawArm(right_arm, right_shoulder);
		drawArm(left_arm, left_shoulder);
		drawHand(left_hand, left_shoulder);
		drawHand(right_hand, right_shoulder);
		drawLeg(left_leg, left_buttom, LEFT);
		drawLeg(right_leg, right_buttom, RIGHT);
		drawFeet(left_feet, left_buttom, LEFT);
		drawFeet(right_feet, right_buttom, RIGHT);
		glPopMatrix();
	}
	glutSwapBuffers();
}
int camera_move_x, camera_move_y;
void mouseClick(int btn, int state, int x, int y) {
	camera_move_x = x;
	camera_move_y = y;
	GLfloat ambient[] = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 0.1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glEnable(GL_LIGHT0);
}
void SpecialPressed(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		spinX -= 2;
		break;
	case GLUT_KEY_RIGHT:
		spinX += 2;
		break;
	case GLUT_KEY_UP:
		des += 2;
		break;
	case GLUT_KEY_DOWN:
		des -= 2;
		break;
	case GLUT_KEY_F1:
		help = true;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
void SpecialReleased(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_F1:
		help = false;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
void keyPressed(unsigned char key, int x, int y) {
	switch (key) {
	case 'x':
		hand_move_x = !hand_move_x;
		break;
	case 'y':
		hand_move_y = !hand_move_y;
		break;
	case 'z':
		hand_move_z = !hand_move_z;
		break;
	case 'j':
		leg_spin_x = !leg_spin_x;
		break;
	case 'k':
		leg_spin_y = !leg_spin_y;
		break;
	case 'l':
		leg_spin_z = !leg_spin_z;
		break;
	case 'r':
		reset();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
void mouseMove(int x, int y) {
	int dx = x - camera_move_x;
	int dy = y - camera_move_y;
	spinX += dx;
	spinY += dy;
	glutPostRedisplay();
	camera_move_x = x;
	camera_move_y = y;
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(SpecialPressed);
	glutSpecialUpFunc(SpecialReleased);
	glutMainLoop();
	return 0;
}
