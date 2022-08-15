/*
Student Information
Student ID:
Student Name:
*/
#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/freeglut/freeglut.h"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "Dependencies/assimp/Importer.hpp"
#include "Dependencies/assimp/scene.h"
#include "Dependencies/assimp/postprocess.h"

#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Model.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

GLuint vaoID[4];
GLuint vboID[4];
GLuint eboID[4];

Shader shader;
Shader skyboxShader;
Shader Assimpshader;

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

float xMove = 0.0f;
float zMove = 0.0f;

glm::vec3 cameraPosition = {0.0f, 2.0f, 5.0f};
glm::vec3 cameraFront = {0.0f, 0.0f, -1.0f};
glm::vec3 cameraUp = {0.0f, 1.0f, 0.0f};
glm::vec3 cameraRight = { 1.0f, 0.0f, 0.0f };

glm::vec3 worldFront = { 0.0f, 0.0f, -1.0f };
glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
glm::vec3 worldRight = { 1.0f, 0.0f, 0.0f };

float fov = 45.0f;

bool firstMouse = true;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLfloat lastX = SCR_WIDTH / 2.0;
GLfloat lastY = SCR_HEIGHT / 2.0;
glm::mat4 SCrotate = glm::mat4(1.0f);

glm::vec3 intensity = {4.0f, 4.0f, 4.0f};

float craftX[3] = {-50.0f,0.0f,50.0f};

float dist = -50.0f;

int rockOR[2000];

GLuint point = 0;

int special = 0;

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

Model earth;
Model rock;
Model spacecraft;
//Models spacecraft("CourseProjectMaterials/object/planet.obj");

Texture skyboxTexture;
Texture earthTexture[2];
Texture rockTexture[4];
Texture craftTexture[2];
Texture spacecraftTexture[3];

void craftData(){
    craftTexture[0].setupTexture("CourseProjectMaterials/texture/ringTexture.bmp");
    craftTexture[1].setupTexture("CourseProjectMaterials/texture/redTexture.jpg");
}

void skyboxData() {
    GLfloat skyboxVertices[] = {
            // Positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
      
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
      
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
       
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
      
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
      
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
    
    glGenVertexArrays(1, &vaoID[2]);
    glGenBuffers(1, &vboID[2]);
    glBindVertexArray(vaoID[2]);
    glBindBuffer(GL_ARRAY_BUFFER, vboID[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
    
    std::vector<const char*> skybox_faces;
    skybox_faces.push_back("skybox textures/right.bmp");
    skybox_faces.push_back("skybox textures/left.bmp");
    skybox_faces.push_back("skybox textures/bottom.bmp");
    skybox_faces.push_back("skybox textures/top.bmp");
    skybox_faces.push_back("skybox textures/back.bmp");
    skybox_faces.push_back("skybox textures/front.bmp");
    skyboxTexture.loadCubemap(skybox_faces);
}

void earthData() {
    earth = loadOBJ("CourseProjectMaterials/object/planet.obj");
    
    glGenVertexArrays(1, &vaoID[0]);
    glBindVertexArray(vaoID[0]);
    
    glGenBuffers(1, &vboID[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, earth.vertices.size() * sizeof(Vertex), &earth.vertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    glGenBuffers(1, &eboID[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, earth.indices.size() * sizeof(unsigned int), &earth.indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    
    earthTexture[0].setupTexture("CourseProjectMaterials/texture/earthTexture.bmp");
    earthTexture[1].setupTexture("CourseProjectMaterials/texture/earthNormal.bmp");
}

void rockData() {
    rock = loadOBJ("CourseProjectMaterials/object/rock.obj");
    
    glGenVertexArrays(1, &vaoID[1]);
    glBindVertexArray(vaoID[1]);
    
    glGenBuffers(1, &vboID[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    glGenBuffers(1, &eboID[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    
    rockTexture[0].setupTexture("CourseProjectMaterials/texture/rockTexture.bmp");
    rockTexture[1].setupTexture("CourseProjectMaterials/texture/golden.jpg");
    rockTexture[2].setupTexture("CourseProjectMaterials/texture/diamond.jpg");
}

void spacecraftData() {
    spacecraft = loadOBJ("CourseProjectMaterials/object/spacecraft.obj");

    glGenVertexArrays(1, &vaoID[3]);
    glBindVertexArray(vaoID[3]);

    glGenBuffers(1, &vboID[3]);
    glBindBuffer(GL_ARRAY_BUFFER, vboID[3]);
    glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glGenBuffers(1, &eboID[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    spacecraftTexture[0].setupTexture("CourseProjectMaterials/texture/spacecraftTexture.bmp");
    spacecraftTexture[1].setupTexture("CourseProjectMaterials/texture/golden.jpg");
    spacecraftTexture[2].setupTexture("CourseProjectMaterials/texture/diamond.jpg");
}

void initialRockOR() {
    for (int i = 0; i < 2000; i++) {
        rockOR[i] = (int)rand() % 30;

        if (rockOR[i] <= 27) {
            rockOR[i] = 0;
        }
        else if (rockOR[i] == 28) {
            rockOR[i] = 1;
            special++;
        }
        else {
            rockOR[i] = 2;
            special++;
        }
    }
}

glm::mat4* modelMatrices;
int amount = 2000;

void CreateRand_Model() {
    modelMatrices = new glm::mat4[amount];
    
    srand(glutGet(GLUT_ELAPSED_TIME) * 0.002);
    GLfloat radius = 50.0f;
    GLfloat offset = 1.0f;
    GLfloat displacement;
    for(GLuint i = 0; i < amount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        
        GLfloat angle = (GLfloat)i / (GLfloat)amount * 360.0f;
        
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat x = sin(angle) * radius + displacement;
        
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat y = displacement * 0.4f + 1;
        
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));
        
        GLfloat scale = (rand() % 10) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale / 3));
        
        GLfloat rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        
        modelMatrices[i] = model;
    }
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void sendDataToOpenGL()
{
	//TODO
    skyboxData();
    earthData();
    rockData();
    spacecraftData();
    CreateRand_Model();

    initialRockOR();

    //Models craft("CourseProjectMaterials/object/craft.obj");
     
    craftData();
	//Load objects and bind to VAO and VBO
	//Load textures
}

void dirLightOption() {
    //shader.setVec3("material. diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
    //shader.setVec3("material.specular", glm::vec3(0.3f, 0.3f, 0.3f));
    //shader.setVec3("material.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
    
    shader.setVec3("viewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
    shader.setFloat("material.shininess", 32.0f);
    shader.setVec3("dirLight.direction", -0.2f, 0.0f, -0.3f);
    shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    shader.setVec3("dirLight.intensity", intensity);
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader
    shader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
    skyboxShader.setupShader("SkyboxVertexShader.glsl", "SkyboxFragmentShader.glsl");
    Assimpshader.setupShader("AssimpVertexShader.glsl", "AssimpFragmentShader.glsl");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void pointLight() {
    shader.setVec3("pointLights[0].position", cameraPosition[0], cameraPosition[1], cameraPosition[2] - 10.0f);
    shader.setVec3("pointLights[0].ambient", 1.0f, 0.05f, 0.05f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09);
    shader.setFloat("pointLights[0].quadratic", 0.032);
}

bool collision(glm::vec4 vectorA, glm::vec4 vectorB)
{
    if (glm::distance(vectorA, vectorB) <= 20.0f)
        return true;
    else
        return false;
}

void paintSkybox() {
    glDepthMask(GL_FALSE);
    
    skyboxShader.use();
    
    dirLightOption();
    
    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
    
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, glm::vec3(400.0f, 400.0f, 400.0f));
    
    modelTransformMatrix = modelTransformMatrix * scaleMatrix;
    skyboxShader.setMat4("modelTransformMatrix", modelTransformMatrix);
    
    glm::mat4 viewTransformMatrix = glm::mat4(1.0f);
    viewTransformMatrix = glm::lookAt(cameraPosition,cameraPosition + cameraFront, cameraUp);
    skyboxShader.setMat4("viewTransformMatrix", viewTransformMatrix);
    
    glm::mat4 projTransformMatrix = glm::mat4(1.0f);
    projTransformMatrix = glm::perspective(glm::radians(fov), 1.3f, 0.1f, 1000.0f);
    skyboxShader.setMat4("projTransformMatrix", projTransformMatrix);
    
    
    glBindVertexArray(vaoID[2]);
    skyboxTexture.bind_cubemap(0);
    //shader.use();
    skyboxShader.setInt("skybox", 0);
    //shader.setInt("objNormal", 1);
    //shader.setInt("normalMapping_flag", 0);
    //glDrawElements(GL_TRIANGLES, skybox.indices.size(), GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void paintEarth() {
    shader.use();
    
    dirLightOption();
    pointLight();
    
    float timer = (float)glutGet(GLUT_ELAPSED_TIME) * 0.0005;
    
    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
    
    glm::mat4 rotateMatrix = glm::mat4(1.0f);
    rotateMatrix = glm::rotate(rotateMatrix, timer, glm::vec3(0.0, 1.0f, 0.0f));
    
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
    
    glm::mat4 translateMatrix = glm::mat4(1.0f);
    translateMatrix = glm::translate(translateMatrix, glm::vec3(0.0f, -5.0f, -200.0f));
    
    modelTransformMatrix = modelTransformMatrix * translateMatrix * scaleMatrix * rotateMatrix;
    shader.setMat4("modelTransformMatrix", modelTransformMatrix);
    
    glm::mat4 viewTransformMatrix = glm::mat4(1.0f);
    viewTransformMatrix = glm::lookAt(cameraPosition,cameraPosition + cameraFront, cameraUp);
    shader.setMat4("viewTransformMatrix", viewTransformMatrix);
    
    glm::mat4 projTransformMatrix = glm::mat4(1.0f);
    projTransformMatrix = glm::perspective(glm::radians(fov), 1.3f, 0.1f, 400.0f);
    shader.setMat4("projTransformMatrix", projTransformMatrix);
    
    
    glBindVertexArray(vaoID[0]);
    earthTexture[0].bind(0);
    earthTexture[1].bind(1);
    //shader.use();
    shader.setInt("objTexture", 0);
    shader.setInt("objNormal", 1);
    shader.setInt("normalMapping_flag", 1);
    glDrawElements(GL_TRIANGLES, earth.indices.size(), GL_UNSIGNED_INT, 0);
    
}

void paintRock() {
    shader.use();
    float timer = (float)glutGet(GLUT_ELAPSED_TIME) * 0.0002;
    float selftimer = (float)glutGet(GLUT_ELAPSED_TIME) * 0.003;;
    dirLightOption();
    pointLight();
    
    for(GLuint i = 0; i < amount; i++) {
        glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
        
        glm::mat4 rotateMatrix = glm::mat4(1.0f);
        rotateMatrix = glm::rotate(rotateMatrix, timer, glm::vec3(0.0, 1.0f, 0.0f));

        glm::mat4 selfrotateMatrix = glm::mat4(1.0f);
        selfrotateMatrix = glm::rotate(selfrotateMatrix, selftimer, glm::vec3(0.0, 1.0f, 0.0f));
        
        glm::mat4 scaleMatrix = glm::mat4(1.0f);
        scaleMatrix = glm::scale(scaleMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
        
        glm::mat4 translateMatrix = glm::mat4(1.0f);
        translateMatrix = glm::translate(translateMatrix, glm::vec3(0.0f, 0.0f, -200.0f));

        glm::mat4 collisionMatrix = modelTransformMatrix * translateMatrix * rotateMatrix * modelMatrices[i];
        glm::vec3 rockPos = collisionMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        bool get = collision(glm::vec4(cameraPosition+glm::vec3(0.0f, -2.0f, -5.0f), 1.0f), glm::vec4(rockPos, 1.0f));

        modelTransformMatrix = modelTransformMatrix * translateMatrix * rotateMatrix * modelMatrices[i] * scaleMatrix * selfrotateMatrix;

        shader.setMat4("modelTransformMatrix", modelTransformMatrix);
        
        glm::mat4 viewTransformMatrix = glm::mat4(1.0f);
        viewTransformMatrix = glm::lookAt(cameraPosition,cameraPosition + cameraFront, cameraUp);
        shader.setMat4("viewTransformMatrix", viewTransformMatrix);
        
        glm::mat4 projTransformMatrix = glm::mat4(1.0f);
        projTransformMatrix = glm::perspective(glm::radians(fov), 1.3f, 0.1f, 400.0f);
        shader.setMat4("projTransformMatrix", projTransformMatrix);
        
        
        glBindVertexArray(vaoID[1]);
        if (get == true) {
            if (rockOR[i] == 1) {
                point += 1;
                special--;
            }
            if (rockOR[i] == 2) {
                point += 2;
                special--;
            }
            rockOR[i] = 3;
        }

        rockTexture[rockOR[i]].bind(0);

        //shader.use();
        shader.setInt("objTexture", 0);

        shader.setInt("normalMapping_flag", 0);
        glDrawElements(GL_TRIANGLES, rock.indices.size(), GL_UNSIGNED_INT, 0);
        
        rockTexture[1].unbind();
    }
    
}

void paintSpcaecraft() {
    shader.use();

    dirLightOption();
    pointLight();

    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);

    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, glm::vec3(0.004f, 0.004f, 0.004f));

    glm::mat4 translateMatrix = glm::mat4(1.0f);
    translateMatrix = glm::translate(translateMatrix, glm::vec3(0.0f + xMove, 0.0f, -5.0f + zMove));

    modelTransformMatrix = modelTransformMatrix * translateMatrix * scaleMatrix  * SCrotate;
    shader.setMat4("modelTransformMatrix", modelTransformMatrix);

    cameraPosition = modelTransformMatrix * glm::vec4(0.0f, 500.0f, 1250.0f, 1.0f);
    worldFront = normalize(SCrotate * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
    worldRight = normalize(SCrotate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    glm::mat4 viewTransformMatrix = glm::mat4(1.0f);
    viewTransformMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    shader.setMat4("viewTransformMatrix", viewTransformMatrix);

    glm::mat4 projTransformMatrix = glm::mat4(1.0f);
    projTransformMatrix = glm::perspective(glm::radians(fov), 1.3f, 0.1f, 50.0f);
    shader.setMat4("projTransformMatrix", projTransformMatrix);


    glBindVertexArray(vaoID[3]);
    int i;
    if (special == 0) {
        i = 2;
    }
    else if (point > 50) {
        i = 1;
    }
    else {
        i = 0;
    }
    spacecraftTexture[i].bind(0);
    //shader.use();
    shader.setInt("objTexture", 0);
    shader.setInt("normalMapping_flag", 0);
    glDrawElements(GL_TRIANGLES, spacecraft.indices.size(), GL_UNSIGNED_INT, 0);
}

void paintCraft(float x, float z) {
    //Models craft("CourseProjectMaterials/object/craft.obj");
    
    Assimpshader.use();
    
    dirLightOption();
    pointLight();
    
    float timer = (float)glutGet(GLUT_ELAPSED_TIME) * 0.001;
    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);

    glm::mat4 rotateMatrix = glm::mat4(1.0f);
    rotateMatrix = glm::rotate(rotateMatrix, timer, glm::vec3(0.0, 1.0f, 0.0f));

    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 translateMatrix = glm::mat4(1.0f);
    translateMatrix = glm::translate(translateMatrix, glm::vec3(x, 0.0f, z));

    glm::mat4 collisionMatrix = modelTransformMatrix * translateMatrix;
    glm::vec3 craftPos = collisionMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool crash = collision(glm::vec4(cameraPosition + glm::vec3(0.0f, -2.0f, -5.0f), 1.0f), glm::vec4(craftPos, 1.0f));

    modelTransformMatrix = modelTransformMatrix * translateMatrix * rotateMatrix * scaleMatrix;
    Assimpshader.setMat4("model", modelTransformMatrix);

    glm::mat4 viewTransformMatrix = glm::mat4(1.0f);
    viewTransformMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    Assimpshader.setMat4("view", viewTransformMatrix);

    glm::mat4 projTransformMatrix = glm::mat4(1.0f);
    projTransformMatrix = glm::perspective(glm::radians(fov), 1.3f, 0.1f, 400.0f);
    Assimpshader.setMat4("projection", projTransformMatrix);


        //glBindVertexArray(vaoID[3]);
    craftTexture[0].bind(0);
    if (crash == true) {
        craftTexture[1].bind(0);
    }
    //shader.use();
    //Assimpshader.setInt("objTexture", 0);
    //Assimpshader.setInt("normalMapping_flag", 0);
}

void paintGL()  //always run
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    paintSkybox();
    

	//TODO:
    
    paintEarth();
    paintRock();
    paintSpcaecraft();
    
    //shader.use();
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Sets the mouse-button callback for the current window.	
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    lastX = xpos;

    GLfloat sensitivity = 0.05;
    xoffset *= sensitivity;

    yaw += xoffset;

    SCrotate = glm::rotate(SCrotate, glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 front;
    front.x = cos(glm::radians(yaw));
    front.y = 0.0f;
    front.z = sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
    glm::vec3 right;
    right.x = -sin(glm::radians(yaw));
    right.y = 0.0f;
    right.z = cos(glm::radians(yaw));
    cameraRight = glm::normalize(right);
	// Sets the cursor position callback for the current window
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_DOWN)
    {
        xMove = xMove - 2.0f * worldFront[0];
        zMove = zMove - 2.0f * worldFront[2];
    }
    if (key == GLFW_KEY_UP)
    {
        xMove = xMove + 2.0f * worldFront[0];
        zMove = zMove + 2.0f * worldFront[2];
    }
    if (key == GLFW_KEY_LEFT)
    {
        xMove = xMove - 2.0f * worldRight[0];
        zMove = zMove - 2.0f * worldRight[2];
    }
    if (key == GLFW_KEY_RIGHT)
    {
        xMove = xMove + 2.0f * worldRight[0];
        zMove = zMove + 2.0f * worldRight[2];
    }
	// Sets the Keyboard callback for the current window.
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                      
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

    Models craft1("CourseProjectMaterials/object/craft.obj");
    Models craft2("CourseProjectMaterials/object/craft.obj");
    Models craft3("CourseProjectMaterials/object/craft.obj");
    GLfloat x[3] = {30.0f, 0.0f, -30.0f};
    GLfloat dir[3] = { -0.2f, 0.2f, 0.2f };
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();
        for (int i = 0; i < 3; i++) {
             if (x[i] < -50.0f) {
                 dir[i] = 0.2f;
              }
             if (x[i] > 50.0f) {
                 dir[i] = -0.2f;
             }
             x[i] += dir[i];
        }
        paintCraft(x[0], -30.0f);
        craft1.Draw(Assimpshader);

        paintCraft(x[1], -80.0f);
        craft2.Draw(Assimpshader);

        paintCraft(x[2], -130.0f);
        craft3.Draw(Assimpshader);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






