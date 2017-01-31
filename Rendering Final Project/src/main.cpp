#include "SceneShader.h"
#include <iostream>

double mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float	rotate_x = 0.0,
		rotate_y = 0.0,
		translate_z = 1.0f;

#define maxZoom 1.2f
#define minZoom 0.3f
#define minRotate 0
#define maxRotate 45

GLFWwindow* window;
SceneShader shader;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			case(GLFW_KEY_ESCAPE):
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
			case (GLFW_KEY_N):
				std::cout << "Recompiling Shaders... ";
				shader.buildShaders();
				std::cout << "DONE" << std::endl;
				break;
			default:
				break;
		}
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		mouse_old_x = x;
		mouse_old_y = y;
	}
}

void motion(GLFWwindow* w, double x, double y)
{
	double dx = (x - mouse_old_x), dy = (y - mouse_old_y);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
	{
		rotate_x += (float)(dy * 0.5f);
		rotate_y += (float)(dx * 0.5f);

		// restricts the vertical viewing angle of the fire
		if (rotate_x < minRotate) 
			rotate_x = minRotate;
		else if (rotate_x > maxRotate) 
			rotate_x = maxRotate;
	}
	mouse_old_x = x;
	mouse_old_y = y;
}

void scroll(GLFWwindow* w, double x, double y)
{
	translate_z += (float) ((x - y) * 0.03f);
	if (translate_z < minZoom) 
		translate_z = minZoom;
	else if (translate_z > maxZoom) 
		translate_z = maxZoom;
}

void errorCallback(int error, const char* description)
{
	std::cout << "GLFW ERROR " << error << ": " << description << std::endl;
}

void startGlew()
{
	if (!gladLoadGL())
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major); // get integer (only if gl version > 3)
	glGetIntegerv(GL_MINOR_VERSION, &minor); // get dot integer (only if gl version > 3)
	printf("OpenGL on %s %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
	printf("GLSL version supported %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("GL version major, minor: %i.%i\n", major, minor);
}

int main(int argc, char**argv)
{
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwSetErrorCallback(errorCallback);
	
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_SAMPLES, 16);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Volumetric Fire", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse);
	glfwSetCursorPosCallback(window, motion);
	glfwSetScrollCallback(window, scroll);

	glfwSwapInterval(1);

	startGlew();

	shader.startup();

	float time = 0.f;
	while (!glfwWindowShouldClose(window))
	{
		GLfloat backgroundColor[] = { 0.f, 0.f, 0.5f };
		const GLfloat zero = 1.0f;

		glClearBufferfv(GL_COLOR, 0, backgroundColor);
		glClearBufferfv(GL_DEPTH, 0, &zero);
		glEnable(GL_DEPTH_TEST);

		shader.setRotationY(rotate_y);
		shader.setRotationX(rotate_x);
		shader.setZTranslation(translate_z);

		shader.render(time);

		glDisable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		time += 0.03f;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
