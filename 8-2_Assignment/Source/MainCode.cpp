#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Paddle 
{
public:
	float x, y;
	float width, height;
	float speed;

	Paddle(float startX, float startY, float w, float h)
		:x(startX), y(startY), width(w), height(h), speed(0.02f) {}

	void moveLeft() { 
		x -= speed; 
	}

	void moveRight() {
		x += speed; 
	}

	void drawPaddle() {
		glColor3f(0.2f, 0.8f, 0.3f);
		glBegin(GL_QUADS);
		glVertex2f(x - width / 2, y - height / 2);
		glVertex2f(x + width / 2, y - height / 2);
		glVertex2f(x + width / 2, y + height / 2);
		glVertex2f(x - width / 2, y + height / 2);
		glEnd();
	}


};

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int hitPoint = 0;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			float intensity = 1.0f - (hitPoint * 0.2f);

			glColor3d(red * intensity, green * intensity, blue * intensity);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float dx;
	float dy;
	float speed = 0.03;

	Circle(double xx, double yy, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		red = r;
		green = g;
		blue = b;
		radius = rad;


		speed = 0.02f + (rand() % 10) * 0.002f;
		dx = (rand() % 2 == 0 ? 1 : -1) * speed * 0.45;
		dy = (rand() % 2 == 0 ? 1 : -1) * speed * 0.45;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return;

		float half = brk->width / 2.0f;

		if ((x + radius > brk->x - half && x - radius < brk->x + half) && (y + radius > brk->y - half && y - radius < brk->y + half))
		{
			if (brk->brick_type == REFLECTIVE) {
				// Simple bounce: reverse the bigger overlap axis
				float overlapX = min(x + radius - (brk->x - half),
					(brk->x + half) - (x - radius));
				float overlapY = min(y + radius - (brk->y - half),
					(brk->y + half) - (y - radius));

				if (overlapX < overlapY) {
					dx = -dx; // bounce horizontally
				}
				else {
					dy = -dy; // bounce vertically
				}
			}
			else if (brk->brick_type == DESTRUCTABLE) {
				brk->hitPoint++;
				if (brk->hitPoint >= 8) {
					brk->onoff = OFF;
				}

				// still bounce when hitting destructible
				float overlapX = min(x + radius - (brk->x - half),
					(brk->x + half) - (x - radius));
				float overlapY = min(y + radius - (brk->y - half),
					(brk->y + half) - (y - radius));

				if (overlapX < overlapY) {
					dx = -dx;
				}
				else {
					dy = -dy;
				}
			}
		}
	}

	void CheckCircleCollision(Circle& circ) {
		float dxC = x - circ.x;
		float dyC = y - circ.y;
		float distanceSq = dxC * dxC + dyC * dyC;
		float radiusSum = radius + circ.radius;

		if (distanceSq <= radiusSum * radiusSum) {
			// Simple elastic bounce: just swap velocities
			std::swap(dx, circ.dx);
			std::swap(dy, circ.dy);
			
			// On collision, change colors
			red = static_cast<float>(rand()) / RAND_MAX;
			green = static_cast<float>(rand()) / RAND_MAX;
			blue = static_cast<float>(rand()) / RAND_MAX;

			circ.red = static_cast<float>(rand()) / RAND_MAX;
			circ.green = static_cast<float>(rand()) / RAND_MAX;
			circ.blue = static_cast<float>(rand()) / RAND_MAX;
		}

	}

	bool CheckCollisionWithPaddle(Paddle& paddle) {
		// Simple bounding box vs circle check
		if (x + radius >= paddle.x - paddle.width / 2 &&
			x - radius <= paddle.x + paddle.width / 2 &&
			y - radius <= paddle.y + paddle.height / 2 &&
			y - radius >= paddle.y - paddle.height / 2) {

			// Bounce upward
			dy = fabs(dy);

			// Optionally change angle based on hit position
			float hitFactor = (x - paddle.x) / (paddle.width / 2);
			dx += hitFactor * 2.0f;

			return true;
		}
		return false;
	}


	int GetRandomDirection()
	{
		return (rand() % 7) + 1;
	}

	void MoveOneStep()
	{
		x += dx;
		y += dy;

		// bounce off screen borders
		if (x + radius > 1.0f || x - radius < -1.0f) {
			dx = -dx;
		}
		if (y + radius > 1.0f || y - radius < -1.0f) {
			dy = -dy;
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;
Paddle paddle(0, -0.9, 0.5f, 0.05f);


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Brick brick(REFLECTIVE, 0.75, -0.63, 0.1, 1, 1, 0);
	Brick brick2(DESTRUCTABLE, -0.8, 0.8, 0.2, 0, 1, 0);
	Brick brick3(DESTRUCTABLE, -0.7, -0.6, 0.3, 0, 1, 1);
	Brick brick4(REFLECTIVE, 0, 0.6, 0.4, 1, 0.5, 0.5);



	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		for (int i = 0; i < world.size(); i++) {
			for (int j = i + 1; j < world.size(); j++) {
				world[i].CheckCircleCollision(world[j]);
			}
		}

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);

			// Remove if below screen
			if (world[i].y - world[i].radius < -1.0f) {
				world.erase(world.begin() + i);
				i--;
				continue;
			}

			// Check paddle bounce
			world[i].CheckCollisionWithPaddle(paddle);

			world[i].MoveOneStep();
			world[i].DrawCircle();

		}

		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		paddle.drawPaddle();

		//Input
		//double mouseX, mouseY;
		//glfwGetCursorPos(window, &mouseX, &mouseY);

		// Assuming 800x600 window, convert mouseX to paddle position:
		//paddle.x = static_cast<float>(mouseX);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		paddle.moveLeft();
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		paddle.moveRight();
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = (rand() % 100) / 100.0;
		g = (rand() % 100) / 100.0;
		b = (rand() % 100) / 100.0;
		Circle B(0, 0, 0.05f, r, g, b);
		world.push_back(B);
	}
}