#pragma warning(disable:4996)

/*
	Выполнил Самохвалов Кирилл. Visual Studio 2013. OpenGL_2.
	Решенные задачи:
	
	добавить на сцену несколько объектов разных размеров, в т.ч.  октаэдр, состоящий из двух пирамид ( до 3 баллов);
	Все объекты должны иметь разные текстуры. При этом плато не должно иметь видимых "швов" (до 2 баллов);
	объекты должны перемещаться по сцене (до 2 баллов).

	Управление движением(поворотом) объектов достигается нажатиями на клавиши влево/вправо.
*/


#include "common_header.h"

#include "win_OpenGLApp.h"

#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* One VBO, where all static data are stored now,
in this tutorial vertex is stored as 3 floats for
position and 2 floats for texture coordinate */

CVertexBufferObject vboSceneObjects;
UINT uiVAO; // And one VAO

CShader shVertex, shFragment;
CShaderProgram spMain;

CTexture tGold, tTable, tPyramid, tPyramid1;

#include "static_geometry.h"

// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void initScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	vboSceneObjects.createVBO();
	glGenVertexArrays(1, &uiVAO); // Create one VAO
	glBindVertexArray(uiVAO);

	vboSceneObjects.bindVBO();

	// Add cube to VBO

	FOR(i, 36)
	{
		vboSceneObjects.addData(&vCubeVertices[i], sizeof(glm::vec3));
		vboSceneObjects.addData(&vCubeTexCoords[i%6], sizeof(glm::vec2));
	}

	// Add pyramid to VBO

	FOR(i, 12)
	{
		vboSceneObjects.addData(&vPyramidVertices[i], sizeof(glm::vec3));
		vboSceneObjects.addData(&vPyramidTexCoords[i%3], sizeof(glm::vec2));
	}
		
	FOR(i, 6)
	{
		vboSceneObjects.addData(&vGround[i], sizeof(glm::vec3));
		vCubeTexCoords[i] *= 5.0f;
		vboSceneObjects.addData(&vCubeTexCoords[i%6], sizeof(glm::vec2));
	}

	// Add my pyramid to VBO

	FOR(i, 12)
	{
		vboSceneObjects.addData(&vPyramid3My[i], sizeof(glm::vec3));
		vboSceneObjects.addData(&vPyramidTexCoords[i % 3], sizeof(glm::vec2));
	}

	// Add ground to VBO

	vboSceneObjects.uploadDataToGPU(GL_STATIC_DRAW);
	
	// Vertex positions start on zero index, and distance between two consecutive is sizeof whole
	// vertex data (position and tex. coord)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates start right after positon, thus on (sizeof(glm::vec3)) index,
	// and distance between two consecutive is sizeof whole vertex data (position and tex. coord)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));

	// Load shaders and create shader program

	shVertex.loadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shFragment.loadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

	spMain.createProgram();
	spMain.addShaderToProgram(&shVertex);
	spMain.addShaderToProgram(&shFragment);

	spMain.linkProgram();
	spMain.useProgram();

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	// Finally, load our texture

	tGold.loadTexture2D("data\\textures\\golddiag.jpg", true);
	tGold.setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	tPyramid.loadTexture2D("data\\textures\\pyramid.jpg", true);
	tPyramid.setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	tPyramid1.loadTexture2D("data\\textures\\pyramid1.jpg", true);
	tPyramid1.setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	tTable.loadTexture2D("data\\textures\\table.jpg", true);
	tTable.setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	glEnable(GL_TEXTURE_2D);
}

float fRotationAngleCube = 0.0f, fRotationAnglePyramid = 0.0f;
float fCubeRotationSpeed = 0.0f, fPyramidRotationSpeed = 0.0f;
const float PIover180 = 3.1415f/180.0f;

void displayTextureFiltersInfo()
{
	char buf[255];
	string sInfoMinification[] = 
	{
		"Nearest",
		"Bilinear"
	};
	string sInfoMagnification[] =
	{
		"Nearest",
		"Bilinear",
		"Nearest on closest mipmap",
		"Bilinear on closest mipmap",
		"Trilinear"
	};
	sprintf(buf, "Mag. Filter: %s, Min. Filter: %s", sInfoMinification[tTable.getMagnificationFilter()].c_str(), 
																	sInfoMagnification[tTable.getMinificationFilter()-2].c_str());
	SetWindowText(appMain.hWnd, buf);
}

// Renders whole scene.
// lpParam - Pointer to anything you want.
void renderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int iModelViewLoc = glGetUniformLocation(spMain.getProgramID(), "modelViewMatrix");
	int iProjectionLoc = glGetUniformLocation(spMain.getProgramID(), "projectionMatrix");
	glUniformMatrix4fv(iProjectionLoc, 1, GL_FALSE, glm::value_ptr(*oglControl->getProjectionMatrix()));

	glm::mat4 mModelView = glm::lookAt(glm::vec3(0, 12, 27), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 mCurrent;

	glBindVertexArray(uiVAO);

	// Texture binding - we set GL_ACTIVE_TEXTURE0, and then we tell fragment shader,
	// that gSampler variable will fetch data from GL_ACTIVE_TEXTURE0

	int iSamplerLoc = glGetUniformLocation(spMain.getProgramID(), "gSampler");
	glUniform1i(iSamplerLoc, 0);

	//1-ая текстура
	tGold.bindTexture(0);
	// Rendering of cube

	mCurrent = glm::translate(mModelView, glm::vec3(5.0f, 10.0f, 0.0f));
	mCurrent = glm::scale(mCurrent, glm::vec3(3.0f, 3.0f, 3.0f));
	mCurrent = glm::rotate(mCurrent, fRotationAngleCube, glm::vec3(-1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, glm::value_ptr(mCurrent));

	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	//2-ая текстура
	tPyramid1.bindTexture(0);
	// Rendering of pyramid

	mCurrent = glm::translate(mModelView, glm::vec3(-10.0f, 5.0f, 0.0f));
	mCurrent = glm::scale(mCurrent, glm::vec3(5.0f, 5.0f, 5.0f));
	mCurrent = glm::rotate(mCurrent, fRotationAnglePyramid, glm::vec3(0.0f, -1.0f, 0.0f));
	glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, glm::value_ptr(mCurrent));

	glDrawArrays(GL_TRIANGLES, 36, 12);

	//Добавляем еще одну пирамиду, перевернутую, чтобы получился тетраэдр
	mCurrent = glm::translate(mModelView, glm::vec3(-10.0f, 0.0f, 0.0f));
	mCurrent = glm::scale(mCurrent, glm::vec3(5.0f, -5.0f, 5.0f));
	mCurrent = glm::rotate(mCurrent, fRotationAnglePyramid, glm::vec3(0.0f, -1.0f, 0.0f));
	glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, glm::value_ptr(mCurrent));

	glDrawArrays(GL_TRIANGLES, 36, 12);

	//3-ая текстура
	tTable.bindTexture();
	// Render ground

	glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, glm::value_ptr(mModelView));
	glDrawArrays(GL_TRIANGLES, 48, 6);


	//Просто еще одна пирамида, со своими вершинами
	//4-ая текстура
	tPyramid.bindTexture(0);

	mCurrent = glm::translate(mModelView, glm::vec3(0.0f, 0.0f, 0.0f));
	mCurrent = glm::scale(mCurrent, glm::vec3(20.0f, 20.0f, 20.0f));
	mCurrent = glm::rotate(mCurrent, fRotationAnglePyramid, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, glm::value_ptr(mCurrent));

	glDrawArrays(GL_TRIANGLES, 54, 12);


	// A little interaction for user

	if(Keys::key(VK_UP))fCubeRotationSpeed -= appMain.sof(60.0f);
	if(Keys::key(VK_DOWN))fCubeRotationSpeed += appMain.sof(60.0f);
	if(Keys::key(VK_RIGHT))fPyramidRotationSpeed += appMain.sof(60.0f);
	if(Keys::key(VK_LEFT))fPyramidRotationSpeed -= appMain.sof(60.0f);

	fRotationAngleCube += appMain.sof(fCubeRotationSpeed);
	fRotationAnglePyramid += appMain.sof(fPyramidRotationSpeed);

	// If user presses escape, quit the application
	if(Keys::onekey(VK_ESCAPE))PostQuitMessage(0);

	// F1 and F2 change the texture filterings and set window text about that
	if(Keys::onekey(VK_F1))
	{
		tGold.setFiltering((tGold.getMagnificationFilter()+1)%2, tGold.getMinificationFilter());
		tTable.setFiltering((tTable.getMagnificationFilter()+1)%2, tTable.getMinificationFilter());
		displayTextureFiltersInfo();
	}
	if(Keys::onekey(VK_F2))
	{
		int iNewMinFilter = tTable.getMinificationFilter() == TEXTURE_FILTER_MIN_TRILINEAR ? TEXTURE_FILTER_MIN_NEAREST : tTable.getMinificationFilter()+1;
		tTable.setFiltering(tTable.getMagnificationFilter(), iNewMinFilter); 
		tGold.setFiltering(tGold.getMagnificationFilter(), iNewMinFilter);
		displayTextureFiltersInfo();
	}
	oglControl->swapBuffers();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void releaseScene(LPVOID lpParam)
{
	spMain.deleteProgram();

	shVertex.deleteShader();
	shFragment.deleteShader();

	vboSceneObjects.releaseVBO();
	glDeleteVertexArrays(1, &uiVAO);

	tGold.releaseTexture();
	tTable.releaseTexture();
}