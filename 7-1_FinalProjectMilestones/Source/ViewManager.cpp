///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include <iostream>
#include <algorithm>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// keeps track of the movement's increase/decrease in speed
	float movementSpeedMultiplier = 0.0f;

	// keeps track of the sensitivity increase/decrease
	float rotationSensitivity = 0.0f;

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// this callabck is used to receive mouse scrolling events
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Wheel_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse) {
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	float xOffset = (xMousePos - gLastX) * rotationSensitivity;
	float yOffset = (gLastY - yMousePos) * rotationSensitivity;

	gLastX = xMousePos;
	gLastY = yMousePos;

	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// adjusted speed
	float adjustedSpeed = gDeltaTime * movementSpeedMultiplier;

	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// if the camera object is null, exit the method
	if (g_pCamera == NULL) {
		return;
	}

	// process camera moving forward and backward
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS) {
		// press W for forward movement
		g_pCamera->ProcessKeyboard(FORWARD, adjustedSpeed);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS) {
		// press S for backward movement
		g_pCamera->ProcessKeyboard(BACKWARD, adjustedSpeed);
	}

	// process camera moving left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS) {
		// press A for left movement
		g_pCamera->ProcessKeyboard(LEFT, adjustedSpeed);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS) {
		// press D for right movement
		g_pCamera->ProcessKeyboard(RIGHT, adjustedSpeed);
	}

	// process camera moving up and down
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS) {
		// press Q for upward movement
		g_pCamera->ProcessKeyboard(UP, adjustedSpeed);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS) {
		// press E for downward movement
		g_pCamera->ProcessKeyboard(DOWN, adjustedSpeed);
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS) {
		// press P for downward movement
		bOrthographicProjection = false;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS) {
		// press O for downward movement
		bOrthographicProjection = true;
	}
}

/***********************************************************
 *  Mouse_Scroll_Wheel_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is moved posiively/negatively in
 *  the vertical direction.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Wheel_Callback(GLFWwindow* window, double xOffset, double yOffset) {

	// incrmenting the movementSpeedMultiplier
	movementSpeedMultiplier += yOffset * 0.1f;

	// applying the movementSpeedMultiplier under the min and max 
	// increment and decrement conditions
	movementSpeedMultiplier = std::max(0.1f, std::min(5.0f, movementSpeedMultiplier));

	// incrmenting the rotationSensitivity
	rotationSensitivity += yOffset * 0.1f;

	// applying the rotationSensitivity under the min and max 
	// increment and decrement conditions
	rotationSensitivity = std::max(0.1f, std::min(5.0f, movementSpeedMultiplier));
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 ortho;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();


	// define the current projection matrix
	if (bOrthographicProjection) {
		float orthoScale = 10.0f;
		float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
		projection = glm::ortho(-orthoScale * aspect, orthoScale * aspect, -orthoScale, orthoScale);
	}
	else {
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}