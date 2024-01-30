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

GLfloat xy(GLfloat t, GLfloat u)
{
    //return sin(t)*cos(u);
    return t*u;
}


void creatingpoints()
{
    GLfloat start = -1.0f;
    GLfloat end = 1.f;
    GLfloat step = 0.01f;
    
    for (GLfloat i = start; i < end; i += step) {
        for (GLfloat j = start; j < end; j += step) {
            vertex v;

            //Location
            v.x = i;   
            v.y = j;
            v.z = xy(i,j);

            //Color
            v.r = 0;
            v.g = 0;
            v.b = 1;

            //Pushing the vertex into vector
            vertices.push_back(v);
        }
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
    std::cout << source << std::endl;
    GLuint shader = glCreateShader(type);
    std::cout << "Shader created" << std::endl;
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

GLuint createshaderprogram(const std::string& vertexShaderSource,std::string& fragmentShaderSource, unsigned int &transformLoc)
{   
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, readFile("vertexshader.vert").c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, readFile("fragmentshader.frag").c_str());

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    transformLoc = glGetUniformLocation(shaderProgram, "transform");

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
    while (file >> p.x >> p.y >> p.z >> p.r >> p.g >> p.b)
    {
        points.push_back(p);
    }
}

void Binding(GLuint shaderProgram)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Creating VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //Creating VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(point), points.data(), GL_DYNAMIC_DRAW);

    //GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    //GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    //glEnableVertexAttribArray(posAttrib);
    //glEnableVertexAttribArray(colAttrib);
    
    //Setting up position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(point), (void*)0);

    //Setting up color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(point), (void*)(3*sizeof(point)));
    
    glBindVertexArray(0);
    glBindVertexArray(1);
}

void camera(GLuint shaderProgram, unsigned int &transformLoc)
{
    const float radius = 10.0f;
    float camX = 0;
    float camZ = 10;
    float camY = 0;

     glm::mat4 view = glm::lookAt(glm::vec3(camX, camY, camZ), 
                                glm::vec3(0.0f, 0.0f, 0.0f), 
                                glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.f/600.f, 0.1f, 100.0f);
    
    glm::mat4 model = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0)),
        glm::radians(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // create transformations
    glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    transform = glm::translate(transform, glm::vec3(camX, 0, camZ));
    transform = glm::rotate(transform, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

unsigned int transformLoc_;

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
    GLuint shaderProgram = createshaderprogram(vertexShaderSource, fragmentShaderSource, transformLoc_);
    
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
        Binding(shaderProgram);
        
        // input
        processInput(window);

        camera(shaderProgram, transformLoc_);
        
        //Draw points
        glDrawArrays(GL_TRIANGLES, 0, points.size());

        //glDrawElements(GL_TRIANGLES, vectorofindex.size(), GL_UNSIGNED_INT, 0);
        
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