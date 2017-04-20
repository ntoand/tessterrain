#include "GLInclude.h"
#include "GLUtils.h"
#include "../Terrain.h"
#include "../Material.h"
#include "Mesh.h"
#include "Camera.h"

#include <sstream>
#include <iostream>
#include <vector>
using std::stringstream;
using std::string;
using std::cout;
using std::endl;

using namespace tessterrain;

GLFWwindow* window;
string title;

//timing related variables
float last_time=0, current_time =0;
//delta time
float dt = 0;

#define WIDTH 1024
#define HEIGHT 768

TessTerrain* tessTerrain = NULL;
vector<Mesh*> objects;

Camera* camera = NULL;

//camera / mouse
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstmouse = true;
bool usemouse = false;

void doMovement();

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //cout << key << endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //cout << "button: " << button << " action: " << action << endl;
    if (button == 0 && action == 1)
        usemouse = true;
    
    else if (button == 0 && action == 0)
        usemouse = false;
}


static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(yoffset);
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    
    if(!usemouse) {
        lastX = xpos;
        lastY = ypos;
        return;
    }
    
    if(firstmouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstmouse = false;
        cout << "first: " << lastX << " " << lastY << endl;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;
    
    camera->processMouseMovement(xoffset, yoffset);
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
	
}

void init_resources(string inifile)
{
    camera = new Camera();
    camera->position = glm::vec3(-102.684, 1940.91, 1665.38);
    camera->front = glm::vec3(0.749869, -0.437191, 0.496548);
    camera->ratio = 1.0*WIDTH/HEIGHT;
    camera->movementSpeed = 1000;
    camera->pitch = -25.9248;
    camera->yaw = 33.5117;
    camera->near = 10.0f;
    camera->far = 1000000.0f;
    
    tessTerrain = new TessTerrain();
    tessTerrain->init(inifile);
    tessTerrain->printInfo();
    tessTerrain->calViewportMatrix(WIDTH, HEIGHT);
    
    // create a sample object at origin
    Mesh* m = MeshUtils::sphere(100, 10, 10);
    m->moveTo(glm::vec3(0, 100, 0));
    objects.push_back(m);
}

void free_resources()
{
    if(tessTerrain)
        delete tessTerrain;
    if(camera)
        delete camera;
    for(int i = 0; i < objects.size(); i++)
        if(objects[i])
            delete objects[i];
}

void doMovement() {
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera->processKeyboard(FORWARD, dt);
    if(keys[GLFW_KEY_S])
        camera->processKeyboard(BACKWARD, dt);
    if(keys[GLFW_KEY_A])
        camera->processKeyboard(LEFT, dt);
    if(keys[GLFW_KEY_D])
        camera->processKeyboard(RIGHT, dt);
    if(keys[GLFW_KEY_I]) {
        cout << "pos: " << camera->position[0] << " " << camera->position[1] << " " << camera->position[2] << endl;
        cout << "front: " << camera->front[0] << " " << camera->front[1] << " " << camera->front[2] << endl;
        cout << "pitch: " << camera->pitch << " yaw: " << camera->yaw << endl;
        keys[GLFW_KEY_I] = false;
    }
    if(keys[GLFW_KEY_T]) {
        keys[GLFW_KEY_T] = false;
    }
    if(keys[GLFW_KEY_N]) {
        tessTerrain->nextDisplayMode();
        keys[GLFW_KEY_N] = false;
    }
}

void mainLoop()
{
    const int samples = 50;
    float time[samples];
    int index = 0;

    do{
        //timing related calcualtion
        last_time = current_time;
        current_time = glfwGetTime();
        dt = current_time-last_time;
        
        glfwPollEvents();
        doMovement();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // Dark blue background
        //glClearColor(1.0f,0.5f,0.5f,1.0f);
    
        // render terrain
        tessTerrain->render(camera);
        
        // render objects
        for(int i = 0; i < objects.size(); i++)
            if(objects[i])
                objects[i]->render(camera);

        glfwSwapBuffers(window);

        // Update FPS
        time[index] = glfwGetTime();
        index = (index + 1) % samples;

        if( index == 0 ) {
          float sum = 0.0f;
          for( int i = 0; i < samples-1 ; i++ )
            sum += time[i + 1] - time[i];
          float fps = samples / sum;

          stringstream strm;
          strm << title;
          strm.precision(4);
          strm << " (fps: " << fps << ")";
          glfwSetWindowTitle(window, strm.str().c_str());
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );
}

int main(int argc, char* argv[]) {
    
    string inifile = "testdata/tess/config.ini";
    if(argc == 2)
        inifile = string(argv[1]);

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

    glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, HEIGHT, "OpenGL window with GLFW", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// print GL info
	GLUtils::dumpGLInfo();

	// init resources
	init_resources(inifile);

	// Enter the main loop
	mainLoop();

	free_resources();

	// Close window and terminate GLFW
	glfwTerminate();

	// Exit program
	return EXIT_SUCCESS;
}
