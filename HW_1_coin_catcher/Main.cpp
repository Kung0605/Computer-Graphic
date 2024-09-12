#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <ctime>
#include <unordered_map>
#include <queue>

#include "./Library/glut/glut.h"
#include "./Library/SOIL/include/SOIL.h"

#pragma comment( lib, "./Library/glut/glut32.lib" )
#pragma comment( lib, "./Library/SOIL/lib/SOIL.lib" )

struct datas {
	int width;
	int height; 
};

struct point {
	GLfloat x;
	GLfloat y;
};

int fontNd;
int score;
int time_now;
int remain_time;
const int max_time = 2.0e4;
bool escape, game_start, game_end, paused, restart;

GLuint rabbit_left_texture, rabbit_right_texture, moon_cake_texture, number_texture, score_texture, start_texture, remain_time_texture, paused_texture, restart_texture;
std::unordered_map<GLuint, datas> image_values;
unsigned char* image;
int width, height, channels;

float rabbit_x;
const float rabbit_y = -0.3;
const float rabbit_size = 0.1;
const float rabbit_step = 0.01;
bool move_left, move_right;

std::queue<point> moon_cakes;
const float moon_cake_size = 0.04;
const float moon_cake_step = 0.01;
const int moon_cake_num = 10;

const float number_size = 0.1;
const float number_x = 0.15;
const float number_y = 0.0;

const float score_size = 0.5;
const float score_y = 0.25;

const float remain_time_size = 0.2;

void draw_number(int number, float x, float y, float xSize, float ySize) {
	float x_left = (float)number / 10;
	float x_right = x_left + 0.1;
	glTexCoord2f(x_left, 0.0); glVertex2f(x - xSize, y - ySize);
	glTexCoord2f(x_right, 0.0); glVertex2f(x + xSize, y - ySize);
	glTexCoord2f(x_right, 1.0); glVertex2f(x + xSize, y + ySize);
	glTexCoord2f(x_left, 1.0); glVertex2f(x - xSize, y + ySize);
}

void set_texture(GLuint& texture, const char* file) {
	// load image
	image = SOIL_load_image(file, &width, &height, &channels, SOIL_LOAD_AUTO);

	if (image) {
		// create OpenGL texture
		texture = SOIL_create_OGL_texture(image, width, height, channels, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

		// release image data
		SOIL_free_image_data(image);

		// store width, height and channels
		image_values[texture] = datas{ width, height };
	}
	else {
		printf("SOIL loading error: '%s' \n", SOIL_last_result());
	}
}


void TimerFunc(int value) {
	if (restart) {
		// reset global values
		score = time_now = 0;
		move_left = move_right = false;
		escape = game_start = game_end = false;
		rabbit_x = 0;
		remain_time = 35;		
		restart = false;
		int n = moon_cakes.size();
		for (int i = 0; i < n; ++i)
			moon_cakes.pop();
		glutPostRedisplay();
		glutTimerFunc(20, TimerFunc, 1);
		return;
	}
	if (!game_start || game_end || paused) {
		glutTimerFunc(20, TimerFunc, 1);
		return;
	}
	if ((time_now += 20) == max_time) {
		game_end = true;
		game_start = false;
		glutTimerFunc(20, TimerFunc, 1);
	}
	// moon_cakes drop
	for (int i = moon_cakes.size(); i > 0; i--) {
		point front = moon_cakes.front();
		front.y -= moon_cake_step + score * 0.00007;
		float buttom = front.y - moon_cake_size;
		if (buttom < -0.2 && abs(front.x - rabbit_x) < rabbit_size)
			score++;
		else if (front.y - moon_cake_size > -0.5)
			moon_cakes.push(front);
		moon_cakes.pop();
	}

	glutPostRedisplay();
	glutTimerFunc(20, TimerFunc, 1);
}

void Timer_moon_cake_control(int value) {
	if (!game_start || game_end || paused) {
		glutTimerFunc(500, Timer_moon_cake_control, 2);
		return;
	}
	// add moon_cakes until its number reaches the maximum
	if (moon_cakes.size() < moon_cake_num)
		moon_cakes.push(point{ (GLfloat)rand() / (GLfloat)(RAND_MAX / 0.96f) - 0.48f , 0.45f });
	glutPostRedisplay();
	glutTimerFunc(500, Timer_moon_cake_control, 2);
}

void Timer_rabbit_control(int value) {
	if (!game_start || game_end || paused) {
		glutTimerFunc(10, Timer_rabbit_control, 3);
		return;
	}
	// move to left
	if (move_left && rabbit_x > -0.45)
		rabbit_x -= rabbit_step;
	// move to right
	if (move_right && rabbit_x < 0.45)
		rabbit_x += rabbit_step;

	glutPostRedisplay();
	glutTimerFunc(10, Timer_rabbit_control, 3);
}

void Timer_remain_time_control(int value) {
	if (!game_start || game_end || paused) {
		glutTimerFunc(1000, Timer_remain_time_control, 4);
		return;
	}
	remain_time -= 1;
	glutTimerFunc(1000, Timer_remain_time_control, 4);
}
void SpecialPress(int key, int x, int y) {
	switch (key) {
			// press left button
		case GLUT_KEY_LEFT: 
			move_left = true;
			break;
			// press right button
		case GLUT_KEY_RIGHT: 
			move_right = true;
			break;
		default: 
			break;
	}
}

void SpecialRelease(int key, int x, int y) {
	switch (key) {
			// release left button
		case GLUT_KEY_LEFT: 
			move_left = false;
			break;
			// release right button
		case GLUT_KEY_RIGHT: 
			move_right = false;
			break;
		default: 
			break;
	}
}

void KeyboardPress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: 
			exit(0);
		case ' ':
			game_start = true;
			break;
		case 'p':
			paused = !paused;
			break;
		case 'P':
			paused = !paused;
			break;
		case 'R':
			if (!game_end)
				break;
			restart = true;
			game_start = false;
			game_end = false;
			break;
		case 'r':
			if (!game_end)
				break;
			restart = true;
			game_start = false;
			game_end = false;
			break;
		default: 
			break;
	}
}

/***************************************************************************************************************/

void init() {
	// set seed of 'rand()'
	std::srand(std::time(NULL));

	// enable 2D texture
	glEnable(GL_TEXTURE_2D);

	// bind texture
	set_texture(rabbit_left_texture, "./Images/Rabbit_left.png");
	set_texture(rabbit_right_texture, "./Images/Rabbit_right.png");
	set_texture(moon_cake_texture, "./Images/Single_moon_cake.png");
	set_texture(start_texture, "./Images/start.png");
	set_texture(score_texture, "./Images/score.png");
	set_texture(number_texture, "./Images/numbers.png");
	set_texture(remain_time_texture, "./Images/timer.png");
	set_texture(paused_texture, "./Images/paused.png");
	set_texture(restart_texture, "./Images/restart.png");

	// set texture parameters
	glBindTexture(GL_TEXTURE_2D, rabbit_left_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, rabbit_right_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// register timer functions
	glutTimerFunc(20, TimerFunc, 1);
	glutTimerFunc(500, Timer_moon_cake_control, 2);
	glutTimerFunc(10, Timer_rabbit_control, 3);
	glutTimerFunc(1000, Timer_remain_time_control, 4);

	// register keyboard functions
	glutSpecialFunc(SpecialPress);
	glutSpecialUpFunc(SpecialRelease);
	glutKeyboardFunc(KeyboardPress);
	//glutKeyboardUpFunc(KeyboardRelease);

	// reset global values
	score = time_now = 0;
	move_left = move_right = false;
	escape = game_start = game_end = false;
	rabbit_x = 0;
	remain_time = 35;
}

void reshape(int w, int h) {
	float aspectRatio = (float)w / (float)h;
	float newWidth, newHeight;

	// Set viewport size
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-0.5 * aspectRatio, 0.5 * aspectRatio, -0.5, 0.5); 
	glMatrixMode(GL_MODELVIEW);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (game_end) {
		int number_longSide = std::max(image_values[number_texture].width / 10, image_values[number_texture].height);
		float number_xSize = number_size * image_values[number_texture].width / 10 / number_longSide;
		float number_ySize = number_size * image_values[number_texture].height / number_longSide;
		int score_longSide = std::max(image_values[score_texture].width, image_values[score_texture].height);
		float score_xSize = score_size * image_values[score_texture].width / score_longSide;
		float score_ySize = score_size * image_values[score_texture].height / score_longSide;
		int restart_longSide = std::max(image_values[restart_texture].width, image_values[restart_texture].height);
		float restart_xSize = 0.75 * image_values[restart_texture].width / restart_longSide;
		float restart_ySize = 0.75 * image_values[restart_texture].height / restart_longSide;
		glBindTexture(GL_TEXTURE_2D, score_texture);
		glBegin(GL_QUADS);
		// print "Your score :"
		glTexCoord2f(0.0, 0.0); glVertex2f(-score_xSize / 5 - 0.2, score_y + 0.1- score_ySize / 5);
		glTexCoord2f(1.0, 0.0); glVertex2f(score_xSize / 5 - 0.2, score_y + 0.1 - score_ySize / 5);
		glTexCoord2f(1.0, 1.0); glVertex2f(score_xSize / 5 - 0.2, score_y + 0.1 + score_ySize / 5);
		glTexCoord2f(0.0, 1.0); glVertex2f(-score_xSize / 5 - 0.2, score_y + 0.1 + score_ySize / 5);
		glEnd();
		// print score
		glBindTexture(GL_TEXTURE_2D, number_texture);
		glBegin(GL_QUADS);
		draw_number(score / 100, -number_x / 5 + 0.015, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		draw_number((score / 10) % 10, 0.0 / 5 + 0.015, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		draw_number(score % 10, number_x / 5 + 0.015, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		glEnd();
		//print restart
		glBindTexture(GL_TEXTURE_2D, restart_texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-restart_xSize / 4 - 0.15, score_y - 0.45 - restart_ySize / 4);
		glTexCoord2f(1.0, 0.0); glVertex2f(restart_xSize / 4 - 0.15, score_y - 0.45 - restart_ySize / 4);
		glTexCoord2f(1.0, 1.0); glVertex2f(restart_xSize / 4 - 0.15, score_y - 0.45 + restart_ySize / 4);
		glTexCoord2f(0.0, 1.0); glVertex2f(-restart_xSize / 4 - 0.15, score_y - 0.45 + restart_ySize / 4);
		glEnd();
	}
	else if (game_start) {
		int rabbit_longSide = std::max(image_values[rabbit_left_texture].width, image_values[rabbit_left_texture].height);
		float rabbit_xSize = rabbit_size * image_values[rabbit_left_texture].width / rabbit_longSide;
		float rabbit_ySize = rabbit_size * image_values[rabbit_left_texture].height / rabbit_longSide;
		int moon_cake_longSide = std::max(image_values[moon_cake_texture].width, image_values[moon_cake_texture].height);
		float moon_cake_xSize = moon_cake_size * image_values[moon_cake_texture].width / moon_cake_longSide;
		float moon_cake_ySize = moon_cake_size * image_values[moon_cake_texture].height / moon_cake_longSide;
		int score_longSide = std::max(image_values[score_texture].width, image_values[score_texture].height);
		float score_xSize = score_size * image_values[score_texture].width / score_longSide;
		float score_ySize = score_size * image_values[score_texture].height / score_longSide;
		int number_longSide = std::max(image_values[number_texture].width / 10, image_values[number_texture].height);
		float number_xSize = number_size * image_values[number_texture].width / 10 / number_longSide;
		float number_ySize = number_size * image_values[number_texture].height / number_longSide;
		int remain_time_longSide = std::max(image_values[remain_time_texture].width, image_values[remain_time_texture].height);
		float remain_time_xSize = remain_time_size * image_values[remain_time_texture].width / remain_time_longSide;
		float remain_time_ySize = remain_time_size * image_values[remain_time_texture].height / remain_time_longSide;
		glBindTexture(GL_TEXTURE_2D, score_texture);
		glBegin(GL_QUADS);
		// print "Your score :"
		glTexCoord2f(0.0, 0.0); glVertex2f(-score_xSize / 5 - 0.2, score_y + 0.1 - score_ySize / 5);
		glTexCoord2f(1.0, 0.0); glVertex2f(score_xSize / 5 - 0.2, score_y + 0.1 - score_ySize / 5);
		glTexCoord2f(1.0, 1.0); glVertex2f(score_xSize / 5 - 0.2, score_y + 0.1 + score_ySize / 5);
		glTexCoord2f(0.0, 1.0); glVertex2f(-score_xSize / 5 - 0.2, score_y + 0.1 + score_ySize / 5);
		glEnd();
		// print score
		glBindTexture(GL_TEXTURE_2D, number_texture);
		glBegin(GL_QUADS);
		draw_number(score / 100, -number_x / 5 + 0.015, number_y +0.35, number_xSize / 8, number_ySize / 6);
		draw_number((score / 10) % 10, 0.0 / 5 + 0.015, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		draw_number(score % 10, number_x / 5 + 0.015, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		glEnd();
		// timer icon
		glBindTexture(GL_TEXTURE_2D, remain_time_texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-remain_time_xSize / 3 + 0.4, 0.35 - remain_time_ySize / 3);
		glTexCoord2f(1.0, 0.0); glVertex2f(remain_time_xSize / 3 + 0.4, 0.35 - remain_time_ySize / 3);
		glTexCoord2f(1.0, 1.0); glVertex2f(remain_time_xSize / 3 + 0.4, 0.35 + remain_time_ySize / 3);
		glTexCoord2f(0.0, 1.0); glVertex2f(-remain_time_xSize / 3 + 0.4, 0.35 + remain_time_ySize / 3);
		glEnd();
		// print remain_time
		glBindTexture(GL_TEXTURE_2D, number_texture);
		glBegin(GL_QUADS);
		draw_number(remain_time / 100, -number_x / 5 + 0.55, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		draw_number((remain_time / 10) % 10, 0.0 / 5 + 0.55, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		draw_number(remain_time % 10, number_x / 5 + 0.55, number_y + 0.35, number_xSize / 8, number_ySize / 6);
		glEnd();
		// rabbit
		glBindTexture(GL_TEXTURE_2D, rabbit_right_texture);
		if (move_left)
			glBindTexture(GL_TEXTURE_2D, rabbit_left_texture);
		glBegin(GL_QUADS);
		float x = rabbit_x;
		glTexCoord2f(0.0, 0.0); glVertex2f(x - rabbit_xSize, rabbit_y - rabbit_ySize);
		glTexCoord2f(1.0, 0.0); glVertex2f(x + rabbit_xSize, rabbit_y - rabbit_ySize);
		glTexCoord2f(1.0, 1.0); glVertex2f(x + rabbit_xSize, rabbit_y + rabbit_ySize);
		glTexCoord2f(0.0, 1.0); glVertex2f(x - rabbit_xSize, rabbit_y + rabbit_ySize);
		glEnd();
		// moon_cakes
		glBindTexture(GL_TEXTURE_2D, moon_cake_texture);
		glBegin(GL_QUADS);
		for (int i = moon_cakes.size(); i > 0; i--) {
			point front = moon_cakes.front();
			float x = front.x;
			float y = front.y;
			moon_cakes.push(front);
			moon_cakes.pop();
			glTexCoord2f(0.0, 0.0); glVertex2f(x - moon_cake_xSize, y - moon_cake_ySize);
			glTexCoord2f(1.0, 0.0); glVertex2f(x + moon_cake_xSize, y - moon_cake_ySize);
			glTexCoord2f(1.0, 1.0); glVertex2f(x + moon_cake_xSize, y + moon_cake_ySize);
			glTexCoord2f(0.0, 1.0); glVertex2f(x - moon_cake_xSize, y + moon_cake_ySize);
		}
		glEnd();

	}
	else {
		glBindTexture(GL_TEXTURE_2D, start_texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-0.16, -0.1 + 0.3);
		glTexCoord2f(1.0, 0.0); glVertex2f(0.16, -0.1 + 0.3);
		glTexCoord2f(1.0, 1.0); glVertex2f(0.16, 0.1 + 0.3);
		glTexCoord2f(0.0, 1.0); glVertex2f(-0.16, 0.1 + 0.3);
		glEnd();
		int paused_longSide = std::max(image_values[paused_texture].width, image_values[paused_texture].height);
		float paused_xSize = 0.75 * image_values[paused_texture].width / paused_longSide;
		float paused_ySize = 0.75 * image_values[paused_texture].height / paused_longSide;
		glBindTexture(GL_TEXTURE_2D, paused_texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-paused_xSize / 3, -paused_ySize / 3 - 0.4);
		glTexCoord2f(1.0, 0.0); glVertex2f(paused_xSize / 3, -paused_ySize / 3 - 0.4);
		glTexCoord2f(1.0, 1.0); glVertex2f(paused_xSize / 3, paused_ySize / 3 - 0.4);
		glTexCoord2f(0.0, 1.0); glVertex2f(-paused_xSize / 3, paused_ySize / 3 - 0.4);
		glEnd();
	}
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("moon_cake Catcher");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}