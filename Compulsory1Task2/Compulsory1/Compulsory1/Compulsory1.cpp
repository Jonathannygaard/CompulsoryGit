#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <windows.h>
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void processInput(GLFWwindow* window);

struct vertex
{
    GLfloat x,y,z,r,g,b;
};
std::vector<vertex> vertices;

struct point
{
    GLfloat x,y,z;
    GLfloat r,g,b;
};
std::vector<point> points;

GLuint VBO, VAO;


// settings
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

GLfloat y(GLfloat t)
{
return sin(t)-t/10; //function y = sin(y)
};

GLfloat dy(GLfloat t)
{
    return cos(t); //function y' = cos(y)
}

GLfloat x(GLfloat t)
{
    return cos(t)-t/10; //function x = cos(x)
}


void creatingpoints()
{
    GLfloat start = -10.0f;
    GLfloat end = 10.f;
    GLfloat step = 0.001f;
    
    for (GLfloat i = start; i < end; i += step)
    {
        int scale = 2;
        vertex v;
        
        //Location
        v.x = x(i)/scale;
        v.y = y(i)/scale;
        v.z = (i/10)/scale;

        //Color
        v.r = dy(i) >= 0 ? 0 : 1;
        v.g = dy(i) >= 0 ? 1 : 0;
        v.b = 0;

        //Pushing the vertex into vector
        vertices.push_back(v);
    }

    std::cout << "Points created " << vertices.size() << std::endl;
}

void creatingfile()
{
    std::ofstream file("data.txt");
    if(!file)
    {
        std::cout<< "Error creating file" << "\n";
        return;
    }

    file << vertices.size() << '\n';
    for (GLint i = 0; i <= vertices.size()-1; ++i) {
        file << vertices[i].x  << " " << vertices[i].y<< " "  << vertices[i].z
        << " " << vertices[i].r << " "<<  vertices[i].g<< " " << vertices[i].b << "\n";
    }
    file.close();
}

std::string readFile(const std::string& filename)
{
    std::ifstream file(filename);
    if(!file)
    {
        std::cout<< "Error reading shader file" << "\n";
        return "";
    }
    if (!file.is_open())
    {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (content.empty())
    {
        std::cerr << "The file is empty: " << filename << std::endl;
        return "";
    }
    
    return content;
}

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, nullptr, buffer);
        std::cout << "Error: Shader compilation failed: " << buffer << std::endl;
    }

    return shader;
}

GLuint createshaderprogram(const std::string& vertexShaderSource,std::string& fragmentShaderSource)
{   
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, readFile("vertexshader.vert").c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, readFile("fragmentshader.frag").c_str());

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        char buffer[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, buffer);
        std::cout << "Error: Shader linking failed: " << buffer << std::endl;
    }

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    return shaderProgram;
}

void binding ()
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
 // ------------------------------------------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glBindVertexArray(0);
  
}

void ReadData()
{
    std::ifstream file("data.txt");
    if(!file)
    {
        std::cout<< "Error reading file" << "\n";
        return;
    }

    std::string line;

    point p;
    std::getline(file, line);
    std::getline(file, line);
    std::cout << "Reading data" << std::endl;
    while (file >> p.x >> p.y >> p.z >> p.r >> p.g >> p.b)
    {
        points.push_back(p);
    }
}


void Display(GLuint shaderProgram)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Creating VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //Creating VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(point), &points[0], GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");

    //Setting up position attribute
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(point), (void*)0);
    

    //Setting up color attribute
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(point), (void*)(3*sizeof(float)));

    glBindVertexArray(0);
    glBindVertexArray(1);
}

void camera()
{
    // Camera position
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);

    // Camera target
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

    // Camera direction
    glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

    // Up vector
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Camera right
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

    // Camera up
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    // View matrix
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);
}

int main()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // to make this portable for other Devices/ operating system MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graph window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::string vertexfile = "vertexshader.vert";
    std::string fragmentfile = "fragmentshader.frag";
    std::string vertexShaderSource = readFile(vertexfile);
    std::string fragmentShaderSource = readFile(fragmentfile);
    GLuint shaderProgram = createshaderprogram(vertexShaderSource, fragmentShaderSource);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::mat4(1.f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        // glm::lookAt(
        // glm::vec3(0.0f, 0.0f, 3.0f),
        // glm::vec3(0.0f, 0.0f, 0.0f),  
        // glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.f/600.f, 0.1f, 100.0f);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); 

    
    creatingpoints();
    creatingfile();
    ReadData();
    
    glEnable(GL_DEPTH_TEST);
    
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader program linking error: " << infoLog << std::endl;
    }
    

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        //Update vertex binding
        binding();
        camera();
        
        Display(shaderProgram);
        
        //Draw points
        glDrawArrays(GL_LINE_STRIP, 0, points.size());
        
        // input
        processInput(window);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL error: " << err << std::endl;
    }

    glDeleteBuffers(1,&VBO);
    glDeleteVertexArrays(1,&VAO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{   
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    std::cout << " windows resized with " << width << " Height " << height << std::endl;
}