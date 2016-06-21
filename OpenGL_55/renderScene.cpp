#include "common_header.h"

#include "win_OpenGLApp.h"

#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"

#include "flyingCamera.h"

#include "skybox.h"

#include "spotLight.h"
#include "dirLight.h"
#include "pointLight.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "objModel.h"

#define NUMTEXTURES 13

CVertexBufferObject vboSceneObjects, vboCubeInd, vboCube;
UINT uiVAOs[2];

CTexture tTextures[NUMTEXTURES];
CFlyingCamera cCamera;

CSkybox skybox;

//CDirectionalLight dlSun;

CSpotLight sLight;
CDirectionalLight dLight;
CPointLight pLight;
CObjModel mdlHouse;

// My models
CObjModel mdlOldBoat;

/*
	Климаков Антон, OpenGL_5, Visual Studio 2013.
	Реализованы три источника, фотокуб, модели, скайбокс.
*/

#include "static_geometry.h"


// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Prepare all scene objects

	vboSceneObjects.CreateVBO();
	glGenVertexArrays(2, uiVAOs);
	glBindVertexArray(uiVAOs[0]);

	vboSceneObjects.BindVBO();

	AddSceneObjects(vboSceneObjects);

	vboSceneObjects.UploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

	glBindVertexArray(uiVAOs[1]);

	vboCube.CreateVBO();

	vboCube.BindVBO();
	AddCube(vboCube);
	vboCube.UploadDataToGPU(GL_STATIC_DRAW);

	vboCubeInd.CreateVBO();
	// Bind indices
	vboCubeInd.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
	vboCubeInd.AddData(&iCubeindices, sizeof(iCubeindices));
	vboCubeInd.UploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3) + sizeof(glm::vec2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	if(!PrepareShaderPrograms())
	{
		PostQuitMessage(0);
		return;
	}
	// Load textures

	string sTextureNames[] = {"water.jpg", "c1.jpg", "c2.jpg", "c3.jpg", "c4.jpg", "c5.jpg", "c6.jpg", "c7.jpg", "c8.jpg",	"c9.jpg", "c10.jpg", "c11.jpg", "c12.jpg" };

	FOR(i, NUMTEXTURES)
	{
		tTextures[i].LoadTexture2D("data\\textures\\"+sTextureNames[i], true);
		tTextures[i].SetFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 10.0f, 120.0f), glm::vec3(0.0f, 10.0f, 119.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	// Loading custom skybox
	skybox.LoadSkybox("data\\skyboxes\\TropicalSunnyDay\\", "TropicalSunnyDayFront2048.png", "TropicalSunnyDayBack2048.png", "TropicalSunnyDayLeft2048.png", "TropicalSunnyDayRight2048.png", "TropicalSunnyDayUp2048.png", "TropicalSunnyDayDown2048.png");
	//dlSun = CDirectionalLight(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(sqrt(2.0f) / 2, - sqrt(2.0f) / 2, 0), 1.0f);
	
	// Lights
	dLight = CDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 1.0f);
	sLight = CSpotLight(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-35.0f, 7.0f, 50.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1, 30.0f, 0.018f);
	pLight = CPointLight(glm::vec3(0.4f, 0.8f, 0.8f), glm::vec3(25.0f, 7.0f, 50.0f), 0.14f, 0.31f, 0.008f, 0.00008f);

	// Models
	mdlHouse.LoadModel("data\\models\\house\\house.obj", "house.mtl");
	mdlOldBoat.LoadModel("data\\models\\boat\\OldBoat.obj", "OldBoat.mtl");
}

float fGlobalAngle;
float fTextureContribution = 0.5f;
bool change;

void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spMain.UseProgram();

	glm::mat4 mModelMatrix, mView;

	// Set spotlight parameters

	glm::vec3 vCameraDir = glm::normalize(cCamera.vView-cCamera.vEye);

	oglControl->ResizeOpenGLViewportFull();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spMain.UseProgram();
	sLight.SetUniformData(&spMain, "sLight");
	dLight.SetUniformData(&spMain, "dLight");

	pLight.SetUniformData(&spMain, "pLight");

	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("gSamplers[0]", 0);
	spMain.SetUniform("gSamplers[1]", 1);
	spMain.SetUniform("vEyePosition", cCamera.vEye);
	spMain.SetUniform("fTexture[0]", 1.0f);
	spMain.SetUniform("fTexture[1]", fTextureContribution);
	spMain.SetUniform("nT", 1);
	


	mView = cCamera.Look();
	spMain.SetUniform("matrices.viewMatrix", &mView);

	mModelMatrix = glm::translate(glm::mat4(1.0f), cCamera.vEye);
	
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mView*mModelMatrix)));

	//CDirectionalLight dlSun2 = dlSun;

	// We set full ambient for skybox, so that its color isn't affected by directional light

	//dlSun2.fAmbient = 1.0f;
	//dlSun2.vColor = glm::vec3(1.0f, 1.0f, 1.0f);
	//dlSun2.SetUniformData(&spMain, "sunLight");

	// Rendering skybox

	skybox.RenderSkybox();
	glBindVertexArray(uiVAOs[1]);
	glEnable(GL_DEPTH_TEST);

	float PI = float(atan(1.0)*4.0);

	// Render cube

	glm::mat4 mModelToCamera;

	// Changing
	if (change) {
		fTextureContribution -= 0.005f;
		if (fTextureContribution < 0.0f) {
			fTextureContribution = 0.0f;
			change = false;
		}
	}
	else {
		fTextureContribution += 0.005f;
		if (fTextureContribution > 10.0f) {
			fTextureContribution = 2.0f;
			change = true;
		}
	}

	FOR(i, 6) { // because of 6 faces

		tTextures[i * 2 + 1].BindTexture();
		tTextures[i * 2 + 2].BindTexture(1);

		spMain.SetUniform("fTexture[0]", 1.0f - fTextureContribution);
		spMain.SetUniform("nT", 2);

		glEnable(GL_CULL_FACE);
		glm::vec3 vPos2 = glm::vec3(-40.0f, 15.0f, 50.0f);
		mModelMatrix = glm::mat4(1.0f);
		mModelMatrix = glm::translate(mModelMatrix, vPos2);
		mModelMatrix = glm::scale(mModelMatrix, glm::vec3(18.0f, 18.0f, 18.0f));
		spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
		spMain.SetUniform("matrices.modelMatrix", mModelMatrix);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(GLuint)));
		glDisable(GL_CULL_FACE);

	}

	spMain.SetUniform("fTexture[0]", 1.0f);
	spMain.SetUniform("nT", 1);

	glBindVertexArray(uiVAOs[0]);

	//dlSun.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0f));

	// Render ground

	tTextures[0].BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Render house

	glm::vec3 vPos = glm::vec3(-50.0f, 0.0f, 50.0f);
	mModelMatrix = glm::translate(glm::mat4(1.0), vPos);
	mModelMatrix = glm::scale(mModelMatrix, glm::vec3(8.0f, 8.0, 8.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("Refl", 0);
	mdlHouse.RenderModel();

	// Render boats

	vPos = glm::vec3(30.0f, 0.0f, 50.0f);
	mModelMatrix = glm::translate(glm::mat4(1.0), vPos);
	mModelMatrix = glm::rotate(mModelMatrix, -PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
	mModelMatrix = glm::scale(mModelMatrix, glm::vec3(1.0f, 1.0, 1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("Refl", 0); 
	mdlOldBoat.RenderModel();

	vPos = glm::vec3(-100.0f, 0.0f, 50.0f);
	mModelMatrix = glm::translate(glm::mat4(1.0), vPos);
	mModelMatrix = glm::rotate(mModelMatrix, -PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
	mModelMatrix = glm::scale(mModelMatrix, glm::vec3(1.0f, 1.0, 1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("Refl", 1); 
	mdlOldBoat.RenderModel();

	cCamera.Update();

	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);
	fGlobalAngle += appMain.sof(1.0f);
	oglControl->SwapBuffers();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	skybox.DeleteSkybox();
	spMain.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();

	glDeleteVertexArrays(2, uiVAOs);
	vboSceneObjects.DeleteVBO();
	vboCubeInd.DeleteVBO();
	vboCube.DeleteVBO();
}