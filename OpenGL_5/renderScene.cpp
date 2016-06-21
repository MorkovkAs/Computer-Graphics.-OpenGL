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

#include "objModel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUMTEXTURES 5
#define FOG_EQUATION_LINEAR		0
#define FOG_EQUATION_EXP		1
#define FOG_EQUATION_EXP2		2

/* 
Климаков Антон, семинар 16, OpenGL_4. Visual Studio 2013, 05.03.2016.
Реализована смена дня и ночи, автоматическое/ручное включение света (точечный, прожектор), появление/исчезновение тумана.
В качестве бонуса лодку можно потопить (после утопления, лодки обычно не всплывают, поэтому вернуть на поверхность ее можно лишь перезагрузив проект).
Управление:
L - автоматическое переключение света день/ночь
J - точечный источник включение/выключение
K - прожектор включение/выключение
H - утопить лодку
F - изменение тумана включение/выключение
WASD - камера
*/

CVertexBufferObject vboSceneObjects, vboCubeInd, vboCube;
UINT uiVAOs[1]; // Only one VAO now

CTexture tTextures[NUMTEXTURES];
CFlyingCamera cCamera;

CSkybox sbMainSkybox;
CObjModel mdlBoat;
CDirectionalLight dlSun;
CSpotLight slFlashLight;
CPointLight plLight;

#include "static_geometry.h"

namespace FogParameters
{
	float fDensity = 0.04f;
	float fStart = 10.0f;
	float fEnd = 75.0f;
	glm::vec4 vFogColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	int iFogEquation = FOG_EQUATION_EXP; // 0 = linear, 1 = exp, 2 = exp2
};

float water_dawn = -0.2;
boolean plLightWorking = true;
// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Prepare all scene objects

	vboSceneObjects.CreateVBO();
	glGenVertexArrays(1, uiVAOs); // Create one VAO
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

	if(!PrepareShaderPrograms())
	{
		PostQuitMessage(0);
		return;
	}
	// Load textures

	string sTextureNames[] = {"water.jpg", "met_wall01a.jpg", "tower.jpg", "box.jpg", "ground.jpg"};

	FOR(i, NUMTEXTURES)
	{
		tTextures[i].LoadTexture2D("data\\textures\\"+sTextureNames[i], true);
		tTextures[i].SetFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 10.0f, 50.0f), glm::vec3(0.0f, 10.0f, 49.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	sbMainSkybox.LoadSkybox("data\\skyboxes\\TropicalSunnyDay\\", "TropicalSunnyDayFront2048.png", "TropicalSunnyDayBack2048.png", "TropicalSunnyDayLeft2048.png", "TropicalSunnyDayRight2048.png", "TropicalSunnyDayUp2048.png", "TropicalSunnyDayDown2048.png");


	dlSun = CDirectionalLight(glm::vec3(0.13f, 0.13f, 0.13f), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 1.0f);

	// Creating spotlight, position and direction will get updated every frame, that's why zero vectors
	slFlashLight = CSpotLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1, 10.0f, 0.02f);
	plLight = CPointLight(glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.0f, 7.1f + water_dawn, 0.0f), 0.15f, 0.3f, 0.007f, 0.002f);

	//mdlBoat.LoadModel("data\\models\\house\\house.obj", "house.mtl");
	mdlBoat.LoadModel("data\\models\\boat\\OldBoat.obj", "OldBoat.mtl");	

}

float fGlobalAngle;
int countScene = 0;
float colorScene = 0.1f;
bool up = true;
bool autoSwitching = true;
bool fog = true;

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spMain.UseProgram();
	glm::mat4 mModelMatrix, mView;

	CPointLight plLight2 = plLight; //точечный источник света, на носу лодки
	plLight2.fAmbient = 1.0f;
	plLight.vPosition = glm::vec3(0.0f, 7.1f + water_dawn, 10.0f);
	plLight2.vColor = glm::vec3(1.0f, 1.0f, 1.0f);

	// Set spotlight parameters
	CSpotLight slFlashLight2 = slFlashLight; //прожектор в середине лодки, вертится автоматически на 360 градусов
	slFlashLight.vPosition = glm::vec3(-0.08f, 10.0f + water_dawn, 5.0f);
	slFlashLight.vDirection = glm::normalize(glm::vec3(cos(fGlobalAngle), -0.2f, sin(fGlobalAngle)));
	slFlashLight2.vColor = glm::vec3(1.0f, 1.0f, 1.0f);
	
	if (countScene < 20000 & up){ //светлеет, туман исчезает
		countScene++;
		if (fog) {
			if (FogParameters::iFogEquation == FOG_EQUATION_LINEAR)
			{
				FogParameters::fStart += appMain.sof(15.0f);
			}
			else
			{
				FogParameters::fDensity -= appMain.sof(0.01f);
			}
		}
	}		

	else if (countScene > -10000){//темнеет, туман появляется 
		countScene--;
		if (fog) {
			if (FogParameters::iFogEquation == FOG_EQUATION_LINEAR)
			{
				FogParameters::fStart -= appMain.sof(15.0f);
			}
			else
			{
				FogParameters::fDensity += appMain.sof(0.01f);
			}
		}
		up = false;
	}
	else
		up = true;

	colorScene = countScene / 20000.0f;
	if (colorScene < 0) //"цвет" солнца
		colorScene = 0.01f;

	if (autoSwitching) {//автоматическое включение/выключение света; если включено, то ручное управление светом (J, K) работать не будет
		if (colorScene < 0.35f) { //night, автоматически включается свет
			if (plLightWorking == false)
				plLight.vColor = glm::vec3(0.4f, 0.4f, 0.4f), plLightWorking = true;
			slFlashLight.bOn = 1;
		}
		else { //day, автоматически выключается свет
			if (plLightWorking == true)
				plLight.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLightWorking = false;
			slFlashLight.bOn = 0;
		}
	}


	dlSun = CDirectionalLight(glm::vec3(colorScene, colorScene, colorScene), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 1.0f);
		
	oglControl->ResizeOpenGLViewportFull();

	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("gSampler", 0);
	spMain.SetUniform("fogParams.iEquation", FogParameters::iFogEquation);
	spMain.SetUniform("fogParams.vFogColor", FogParameters::vFogColor);
	mView = cCamera.Look();
	spMain.SetUniform("matrices.viewMatrix", &mView);

	mModelMatrix = glm::translate(glm::mat4(1.0f), cCamera.vEye);
	
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mView*mModelMatrix)));

	if (FogParameters::iFogEquation == FOG_EQUATION_LINEAR)
	{
		spMain.SetUniform("fogParams.fStart", FogParameters::fStart);
		spMain.SetUniform("fogParams.fEnd", FogParameters::fEnd);
	}
	else
		spMain.SetUniform("fogParams.fDensity", FogParameters::fDensity);
		
	CDirectionalLight dlSun2 = dlSun;

	// We set full ambient for skybox, so that its color isn't affected by directional light

	dlSun2.fAmbient = 1.0f;
	dlSun2.vColor = glm::vec3(colorScene, colorScene, colorScene);
	dlSun2.SetUniformData(&spMain, "sunLight");
	plLight2.SetUniformData(&spMain, "pointLight");
	slFlashLight2.SetUniformData(&spMain, "spotLight");

	sbMainSkybox.RenderSkybox();

	glm::mat4 mModelToCamera;

	glBindVertexArray(uiVAOs[0]);
	dlSun.SetUniformData(&spMain, "sunLight");
	plLight.SetUniformData(&spMain, "pointLight");
	slFlashLight.SetUniformData(&spMain, "spotLight");

	//spMain.SetUniform("vColor", glm::vec4(colorScene, colorScene, colorScene, 1.0f));//0.3f, 0.3f, 0.3f, 1.0f));
	spMain.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0f));


	// Render ground
	tTextures[0].BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	FOR(i, 1)
	{
		glm::vec3 vPos = glm::vec3(0.0f, water_dawn, 0.0f);
		mModelMatrix = glm::translate(glm::mat4(1.0), vPos);
		mModelMatrix = glm::scale(mModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
		spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
		spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
		mdlBoat.RenderModel();
	}

	cCamera.Update();

	if (FogParameters::iFogEquation == FOG_EQUATION_LINEAR)
	{
		if (Keys::Key(VK_ADD))
			FogParameters::fStart += appMain.sof(15.0f);
		if (Keys::Key(VK_SUBTRACT))
			FogParameters::fStart -= appMain.sof(15.0f);

		if (Keys::Key(VK_PRIOR))
			FogParameters::fEnd += appMain.sof(15.0f);
		if (Keys::Key(VK_NEXT))
			FogParameters::fEnd -= appMain.sof(15.0f);
	}
	else
	{
		if (Keys::Key(VK_ADD))
			FogParameters::fDensity += appMain.sof(0.01f);
		if (Keys::Key(VK_SUBTRACT))
			FogParameters::fDensity -= appMain.sof(0.01f);
	}

	if (Keys::Onekey('F')) //есть ли туман
		fog = !fog;
	if (Keys::Key('H')) //тонет лодка
		water_dawn -= 0.001;

	if (Keys::Onekey('J')) //точечный источник света, ручное управление
		if (plLightWorking == true)
			plLight.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLightWorking = false;
		else
			plLight.vColor = glm::vec3(0.4f, 0.4f, 0.4f), plLightWorking = true;
	if (Keys::Onekey('K')) //прожектор, ручное управление
		slFlashLight.bOn = 1 - slFlashLight.bOn;
	if (Keys::Onekey('L')){ //автоматическое управление светом; если true, то ручное управление недоступно
		autoSwitching = !autoSwitching;
	}

	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);
	fGlobalAngle += appMain.sof(1.0f);
	oglControl->SwapBuffers();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	sbMainSkybox.DeleteSkybox();
	spMain.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();

	glDeleteVertexArrays(1, uiVAOs);
	vboSceneObjects.DeleteVBO();
	vboCubeInd.DeleteVBO();
	vboCube.DeleteVBO();
	mdlBoat.DeleteModel();
}