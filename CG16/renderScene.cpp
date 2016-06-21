/* Лянге Дмитрий Сергеевич, БПИ122, КДЗ2.
28.03.2016. VS2013.
Сделан Вариант 1.
1) Салют (активировать/выключить - "1")
2) Поющий фонтан (активировать/выключить - "2")
3) Костер (выключить/активировать - "3")
5) Дождь (активировать/выключить - "5")
Также на сцене присутствует штурмовик, его бластер, робот и основание фонтана в виде маяка. (и 2 крутящихся тора)


2) Добавлены клавиши (и выведена информация о них): число частиц огня, цвет частиц огня, размер частиц огня
3) Стрелками клавиатуры управляется направление и скорость поворота объекта


Дополнительно:
1) Реализован skybox
2) Прямой источник света (за спиной пользователя при первичном появлении на сцене),
направление регулируется кнопками "-" и "+". Включить/выключить на клавишу "L";
Точечный источник света. Включить/выключить на клавишу "K";
3) Туман (плотность регулируется клавишами 'I' и 'O', включение/выключение - 'U')
4) Перемещение камеры ('W','A','S','D'), перемещение объекта
5) На экран выведена информация об авторе работе, об управляющих клавишах и о контроле за основной струей фонтана
*/

#include "common_header.h"

#include "win_OpenGLApp.h"

#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"

#include "flyingCamera.h"

#include "freeTypeFont.h"

#include "skybox.h"

#include "spotLight.h"
#include "dirLight.h"
#include "pointLight.h"

#include "material.h"


#include "assimp_model.h"

#include "heightmap.h"

#include "static_geometry.h"

#include "particle_system_tf.h"


CVertexBufferObject vboSceneObjects;
UINT uiVAOSceneObjects;

CFreeTypeFont ftFont;

CSkybox sbMainSkybox;
CFlyingCamera cCamera;

// свет из дальней галактики
CDirectionalLight dlStar;

// свет огня
CPointLight plFire;

CMaterial matShiny;
CAssimpModel amModels[5];

CMultiLayeredHeightmap hmWorld;

int iTorusFaces;

bool bDisplayNormals = false; // Do not display normals by default

//системы частиц
//салют
CParticleSystemTransformFeedback salute;
//снярад
CParticleSystemTransformFeedback bullet;
//фонтан 
//основная струя
CParticleSystemTransformFeedback fountain;
//остальные струи
CParticleSystemTransformFeedback fountain1;
CParticleSystemTransformFeedback fountain2;
//огонь
CParticleSystemTransformFeedback fire;
//дым
CParticleSystemTransformFeedback smoke;
//пепел
CParticleSystemTransformFeedback ash;

namespace FogParameters {
	float fDensity = 0.03f;
	float fStart = 10.0f;
	float fEnd = 20.0f;
	glm::vec4 vFogColor = glm::vec4(0.1f, 0.1f, 0.1f, .2f);
	int iFogEquation = 0; // 0 = linear, 1 = exp, 2 = exp2
	bool showFog = false;
};

boolean moveObject = false;
float moveAngle = 0.0;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

/*-----------------------------------------------

Name:    InitScene

Params:  lpParam - Pointer to anything you want.

Result:  Initializes OpenGL features that will
         be used.

/*---------------------------------------------*/

void InitScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if(!PrepareShaderPrograms())
	{
		PostQuitMessage(0);
		return;
	}
	
	LoadAllTextures();

	vboSceneObjects.CreateVBO();
	glGenVertexArrays(1, &uiVAOSceneObjects); // Create one VAO
	glBindVertexArray(uiVAOSceneObjects);

	vboSceneObjects.BindVBO();

	iTorusFaces = GenerateTorus(vboSceneObjects, 7.0f, 2.0f, 20, 20);
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


	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	// Here we load font with pixel size 32 - this means that if we print with size above 32, the quality will be low
	ftFont.LoadSystemFont("arial.ttf", 32);
	ftFont.SetShaderProgram(&spFont2D);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 30.0f, 100.0f), glm::vec3(0.0f, 30.0f, 99.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.01f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D');

	// меняем skybox (сохраним старый на случай реализации переключения skybox-ов)
	// sbMainSkybox.LoadSkybox("data\\skyboxes\\bluefreeze\\", "bluefreeze_front.jpg", "bluefreeze_back.jpg", "bluefreeze_right.jpg", "bluefreeze_left.jpg", "bluefreeze_top.jpg", "bluefreeze_top.jpg");
	sbMainSkybox.LoadSkybox("data\\skyboxes\\ame_nebula\\", "purplenebula_ft.tga", "purplenebula_bk.tga", "purplenebula_rt.tga", "purplenebula_lf.tga", "purplenebula_up.tga", "purplenebula_dn.tga");

	// создаем прямой источник света из дальней галактики
	dlStar = CDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(sqrt(2.0f)/2, sqrt(2.0f)/2, -20.f), 0.5f, 0);

	plFire = CPointLight(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10.0f, 5.5f, 0.0f), 0.15f, 0.3f, 0.1f, 0.00004f);

	// Добавляем сюжетные модели
	amModels[0].LoadModelFromFile("data\\models\\BB8_New\\bb8.obj");
	amModels[1].LoadModelFromFile("data\\models\\Stormtrooper\\Stormtrooper.obj");	
	amModels[2].LoadModelFromFile("data\\models\\E-11_Blaster_Rifle\\E-11_Blaster_Rifle.obj");
	amModels[3].LoadModelFromFile("data\\models\\lighthouse\\Models and Textures\\lighthouse.obj");
	amModels[4].LoadModelFromFile("data\\models\\Fireplace\\Obj\\fireplace.obj");

	
	CAssimpModel::FinalizeVBO();
	CMultiLayeredHeightmap::LoadTerrainShaderProgram();
	//сменяем heightMap
	//hmWorld.LoadHeightMapFromImage("data\\worlds\\world_like_in_21th.bmp");
	hmWorld.LoadHeightMapFromImage("data\\worlds\\venus.bmp");

	matShiny = CMaterial(1.0f, 32.0f);
	

	salute.InitalizeParticleSystem();
	salute.SetGeneratorProperties(
		glm::vec3(-20.0f, 60.f, 80.0f), // Where the particles are generated
		glm::vec3(-10, 1, -10), // Minimal velocity
		glm::vec3(10, 10, 10), // Maximal velocity
		glm::vec3(0, -10, 0), // Gravity force applied to particles
		glm::vec3(0.0f, 1.f, 1.0f), // Color (light blue)
		3.f, // Minimum lifetime in seconds
		5.0f, // Maximum lifetime in seconds
		3.f, // Rendered size
		5.0f, // Spawn every 0.05 seconds
		30); // And spawn 30 particles
	salute.particleType = 0;

	bullet.InitalizeParticleSystem();
	bullet.SetGeneratorProperties(
		glm::vec3(-20.0f, 0.f, 80.0f), // Where the particles are generated
		glm::vec3(0, 12, 0), // Minimal velocity
		glm::vec3(0, 12, 0), // Maximal velocity
		glm::vec3(0, 0, 0), // Gravity force applied to particles
		glm::vec3(0.0f, 1.f, 1.0f), // Color (light blue)
		5.f, // Minimum lifetime in seconds
		5.0f, // Maximum lifetime in seconds
		2.f, // Rendered size
		5.0f, // Spawn every 0.05 seconds
		1); // And spawn 30 particles
	bullet.particleType = 0;

	fountain.InitalizeParticleSystem();
	fountain.SetGeneratorProperties(
		glm::vec3(-50.0f, 20.5f, 0.0f), // Where the particles are generated
		glm::vec3(-3, 6, 3), // Minimal velocity
		glm::vec3(3, 8, 3), // Maximal velocity
		glm::vec3(0, -10, 0), // Gravity force applied to particles
		glm::vec3(0.0f, 0.5f, 1.0f), // Color (light blue)
		1.5f, // Minimum lifetime in seconds
		3.0f, // Maximum lifetime in seconds
		0.75f, // Rendered size
		0.05f, // Spawn every 0.05 seconds
		30); // And spawn 30 particles
	fountain.particleType = 1;

	fountain1.InitalizeParticleSystem();
	fountain1.SetGeneratorProperties(
		glm::vec3(-50.0f, 20.5f, 0.0f), // Where the particles are generated
		glm::vec3(-2, 0, -2), // Minimal velocity
		glm::vec3(2, 15, 2), // Maximal velocity
		glm::vec3(0, -10, 0), // Gravity force applied to particles
		glm::vec3(0.0, 1.0, 1.0), // Color (light blue)
		5.0f, // Minimum lifetime in seconds
		7.0f, // Maximum lifetime in seconds
		0.25f, // Rendered size
		0.20f, // Spawn every 0.05 seconds
		50); // And spawn 30 particles
	fountain1.particleType = 1;

	fountain2.InitalizeParticleSystem();
	fountain2.SetGeneratorProperties(
		glm::vec3(-50.0f, 20.5f, 0.0f), // Where the particles are generated
		glm::vec3(2, 3, 2), // Minimal velocity
		glm::vec3(2, 4, 2), // Maximal velocity
		glm::vec3(0, -10, 0), // Gravity force applied to particles
		glm::vec3(0.0, 1.0, 1.0), // Color (light blue)
		5.0f, // Minimum lifetime in seconds
		7.0f, // Maximum lifetime in seconds
		0.25f, // Rendered size
		0.20f, // Spawn every 0.05 seconds
		50); // And spawn 30 particles
	fountain2.particleType = 1;

	fire.InitalizeParticleSystem();
	fire.SetGeneratorProperties(
		glm::vec3(-10.0f, 7.5f, 0.0f), // Where the particles are generated
		glm::vec3(-2, 0, -2), // Minimal velocity
		glm::vec3(2, 5, 2), // Maximal velocity
		glm::vec3(0, -1, 0), // Gravity force applied to particles
		glm::vec3(1.0f, 0.5f, 0.0f), // Color (red)
		1.5f, // Minimum lifetime in seconds
		3.0f, // Maximum lifetime in seconds
		0.75f, // Rendered size
		0.02f, // Spawn every 0.02 seconds
		30); // And spawn 30 particles
	fire.particleType = 2;

	smoke.InitalizeParticleSystem();
	smoke.SetGeneratorProperties(
		glm::vec3(-10.0f, 14.5f, 0.0f), // Where the particles are generated
		glm::vec3(-1, 0, -1), // Minimal velocity
		glm::vec3(1, 7, 1), // Maximal velocity
		glm::vec3(0, 3, 0), // Gravity force applied to particles
		glm::vec3(.4f, .8f, .9f), // Color (light blue)
		1.5f, // Minimum lifetime in seconds
		5.0f, // Maximum lifetime in seconds
		2.5f, // Rendered size
		0.3f, // Spawn every 0.05 seconds
		10); // And spawn 30 particles
	smoke.particleType = 2;

	ash.InitalizeParticleSystem();
	ash.SetGeneratorProperties(
		glm::vec3(-10.0f, 50.f, -50.0f), // Where the particles are generated
		glm::vec3(-10, 0, -10), // Minimal velocity
		glm::vec3(10, -100, 10), // Maximal velocity
		glm::vec3(0, -1, 0), // Gravity force applied to particles
		glm::vec3(0.0f, 0.5f, 1.0f), // Color (light blue)
		1.5f, // Minimum lifetime in seconds
		3.0f, // Maximum lifetime in seconds
		0.5f, // Rendered size
		0.05f, // Spawn every 0.02 seconds
		30); // And spawn 30 particles
	ash.particleType = 3;
}

/*-----------------------------------------------

Name:    RenderScene

Params:  lpParam - Pointer to anything you want.

Result:  Renders whole scene.

/*---------------------------------------------*/
//параметры салюта
int saluteParticlesAmount = 0;
int bulletParticlesAmount = 0;
glm::vec3 saluteForm = glm::vec3(10, 10, 10);

//параметры фонтана
double waterParticleSize = 0.25;
glm::vec3 waterParticleColour = glm::vec3(1.0f, 0.5f, 0.0f);
int waterParticlesAmount = 0;
int water1ParticlesAmount = 0;
glm::vec3 coloursArray[] = { glm::vec3(1.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(1.0, 0.0, 1.0), glm::vec3(1.0, 1.0, 1.0) };
glm::vec3 speedArray[] = { glm::vec3(2, 15, 2), glm::vec3(10, 100, 10), glm::vec3(6, 15, 6)};
glm::vec3 currentSpeed = glm::vec3(2, 4, 2);

boolean isDirectDisplayed = true;
boolean isPointDisplayed = true;

boolean isSaluteDisplayed = false;
boolean isFountainDisplayed = false;
boolean isFireDisplayed = true;
boolean isShotDisplayed = false;
boolean isRainDisplayed = false;

//параметры огня
int fireParticlesAmount = 30;
//параметры дыма
int smokeParticlesAmount = 10;

//параметры пепла
int ashParticlesAmount = 0;

float vertRotation = 0.0f;
float horRotation = 0.0f;
float fVertAngle = 0.0f;
float fHorAngle = 0.0f;

void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;
	oglControl->ResizeOpenGLViewportFull();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	spMain.UseProgram();

	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("matrices.viewMatrix", cCamera.Look());

	spMain.SetUniform("gSampler", 0);

	spMain.SetUniform("fogParams.iEquation", FogParameters::iFogEquation);
	spMain.SetUniform("fogParams.vFogColor", FogParameters::vFogColor);
	spMain.SetUniform("fogParams.showFog", FogParameters::showFog);

	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0));
	spMain.SetUniform("vColor", glm::vec4(1, 1, 1, 1));

	if (FogParameters::iFogEquation == 0){
		spMain.SetUniform("fogParams.fStart", FogParameters::fStart);
		spMain.SetUniform("fogParams.fEnd", FogParameters::fEnd);
	}
	spMain.SetUniform("fogParams.fDensity", FogParameters::fDensity);

	// This values will set the darkness of whole scene, that's why such name of variable :D
	static float fAngleOfDarkness = 45.0f;
	// You can play with direction of light with '+' and '-' key
	if(Keys::Key(VK_ADD))fAngleOfDarkness += appMain.sof(90);
	if(Keys::Key(VK_SUBTRACT))fAngleOfDarkness -= appMain.sof(90);

	plFire.SetUniformData(&spMain, "pointLight1");

	// Set the directional vector of light
	dlStar.vDirection = glm::vec3(-sin(fAngleOfDarkness*3.1415f/180.0f), -cos(fAngleOfDarkness*3.1415f/180.0f), -20.0f);

	
	//управление туманом
	// клавиши регулировки света
	if (Keys::Onekey('L')) {
		if (dlStar.vColor == glm::vec3(.1f, .1f, .1f)) {
			dlStar.vColor = glm::vec3(1.f, 1.f, 1.f);
			isDirectDisplayed = true;
		} else {
			dlStar.vColor = glm::vec3(.1f, .1f, .1f);
			isDirectDisplayed = false;
		}		
	}

	dlStar.iSkybox = 1;
	dlStar.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("matrices.modelMatrix", glm::translate(glm::mat4(1.0), cCamera.vEye));
	sbMainSkybox.RenderSkybox();

	dlStar.iSkybox = 0;
	dlStar.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("matrices.modelMatrix", glm::translate(glm::mat4(1.0), cCamera.vEye));
	sbMainSkybox.RenderSkybox();

	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0));

	spMain.SetUniform("vEyePosition", cCamera.vEye);
	matShiny.SetUniformData(&spMain, "matActive");
	
	// Render a robot
	if (moveObject)
	{
		moveAngle += appMain.sof(1.0f);

	}

	CAssimpModel::BindModelsVAO();

	glm::mat4 mModel = glm::translate(glm::mat4(1.0), glm::vec3(-20.0f, 2.5f, 80.f));
	mModel = glm::scale(mModel, glm::vec3(0.1f, 0.1f, 0.1f));

	mModel = glm::translate(glm::mat4(1.0), glm::vec3(-20.0f + 60 * sin(moveAngle), 2.5f + 40 * sin(2 * moveAngle), 80.0f + 20 * sin(moveAngle)));
	mModel = glm::scale(mModel, glm::vec3(.1f, .1f, .1f));

	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[0].RenderModel();

	// ... and a stormtrooper

	mModel = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, 2.5f, 0));
	mModel = glm::scale(mModel, glm::vec3(5.5f, 5.5f, 5.5f));

	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[1].RenderModel();

	// ... and a gun

	mModel = glm::translate(glm::mat4(1.0), glm::vec3(5.0f, 11.5f, 1.f));
	mModel = glm::scale(mModel, glm::vec3(3.5f, 3.5f, 3.5f));

	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[2].RenderModel();

	// ... and a tower

	mModel = glm::translate(glm::mat4(1.0), glm::vec3(-50.0f, 4.0f, 0.0f));
	mModel = glm::scale(mModel, glm::vec3(1.5f, 1.5f, 1.5f));

	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[3].RenderModel();

	// ... and a fireplace

	mModel = glm::translate(glm::mat4(1.0), glm::vec3(-10.0f, 4.0f, 0.0f));
	mModel = glm::scale(mModel, glm::vec3(1.f, 1.f, 1.f));

	spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
	amModels[4].RenderModel();

	// Render 3 rotated tori to create interesting object

	tTextures[5].BindTexture();
	glBindVertexArray(uiVAOSceneObjects);
	

	FOR(i, 2)
	{
		glm::vec3 vCenter = glm::vec3(-40+i*40, 10, -20);
		mModel = glm::translate(glm::mat4(1.0), vCenter);
		if (i == 0) {
			mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
			mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);

		mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.01f, 0.0f, 0.0f));
		if (i == 0) {
			mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
			mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		mModel = glm::rotate(mModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);

		mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.00f, 0.01f, 0.0f));
		if (i == 0) {
			mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
			mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		mModel = glm::rotate(mModel, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		spMain.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		glDrawArrays(GL_TRIANGLES, 0, iTorusFaces*3);
	}

	// Now we're going to render terrain

	hmWorld.SetRenderSize(300.0f, 35.0f, 300.0f);
	CShaderProgram* spTerrain = CMultiLayeredHeightmap::GetShaderProgram();

	spTerrain->UseProgram();

	spTerrain->SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spTerrain->SetUniform("matrices.viewMatrix", cCamera.Look());

	spTerrain->SetUniform("vEyePosition", cCamera.vEye);
	matShiny.SetUniformData(spTerrain, "matActive");

	// We bind all 5 textures - 3 of them are textures for layers, 1 texture is a "path" texture, and last one is
	// the places in heightmap where path should be and how intense should it be
	FOR(i, 5)
	{
		char sSamplerName[256];
		sprintf_s(sSamplerName, "gSampler[%d]", i);//_s
		tTextures[i].BindTexture(i);
		spTerrain->SetUniform(sSamplerName, i);
	}

	// ... set some uniforms
	spTerrain->SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", glm::mat4(1.0));
	spTerrain->SetUniform("vColor", glm::vec4(1, 1, 1, 1));

	dlStar.SetUniformData(spTerrain, "sunLight");

	// ... and finally render heightmap
	hmWorld.RenderHeightmap();

	if(bDisplayNormals)
	{
		spNormalDisplayer.UseProgram();
		spNormalDisplayer.SetUniform("fNormalLength", 1.0f);
		spNormalDisplayer.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
		spNormalDisplayer.SetUniform("matrices.viewMatrix", cCamera.Look());

		CAssimpModel::BindModelsVAO();

		// ... Render the robot again

		glm::mat4 mModel = glm::translate(glm::mat4(1.0), glm::vec3(-20.0f, 17.5f, 80.f));
		mModel = glm::scale(mModel, glm::vec3(0.1f, 0.1f, 0.1f));
		mModel = glm::translate(glm::mat4(1.0), glm::vec3(-20.0f + 60 * sin(moveAngle), 2.5f + 40 * sin(2 * moveAngle), 80.0f + 20 * sin(moveAngle)));


		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[0].RenderModel(GL_POINTS);

		// ... and the stormtrooper again

		mModel = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, 17.5f, 0));
		mModel = glm::scale(mModel, glm::vec3(5.5f, 5.5f, 5.5f));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[1].RenderModel(GL_POINTS);

		// ... and the gun again

		mModel = glm::translate(glm::mat4(1.0), glm::vec3(5.0f, 26.5f, 1.f));
		mModel = glm::scale(mModel, glm::vec3(3.5f, 3.5f, 3.5f));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[2].RenderModel(GL_POINTS);

		// ... and a tower again

		mModel = glm::translate(glm::mat4(1.0), glm::vec3(-50.0f, 4.0f, 0.0f));
		mModel = glm::scale(mModel, glm::vec3(1.5f, 1.5f, 1.5f));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[3].RenderModel(GL_POINTS);

		// ... and a fireplace again

		mModel = glm::translate(glm::mat4(1.0), glm::vec3(-10.0f, 4.0f, 0.0f));
		mModel = glm::scale(mModel, glm::vec3(1.f, 1.f, 1.f));

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
		amModels[4].RenderModel(GL_POINTS);

		glBindVertexArray(uiVAOSceneObjects);

		FOR(i, 2)
		{
			glm::vec3 vCenter = glm::vec3(-40+i*40, 10, -20);
			mModel = glm::translate(glm::mat4(1.0), vCenter);
			mModel = glm::translate(glm::mat4(1.0), vCenter + glm::vec3(0.01f, 0.0f, 0.0f));
			if (i == 0) {
				mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);

			mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.01f, 0.0f, 0.0f));
			if (i == 0) {
				mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			mModel = glm::rotate(mModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);

			mModel = glm::translate(glm::mat4(1.0), vCenter+glm::vec3(0.00f, 0.01f, 0.0f));
			mModel = glm::translate(glm::mat4(1.0), vCenter + glm::vec3(0.01f, 0.0f, 0.0f));
			if (i == 0) {
				mModel = glm::rotate(mModel, fVertAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				mModel = glm::rotate(mModel, fHorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			mModel = glm::rotate(mModel, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", mModel);
			glDrawArrays(GL_POINTS, 0, iTorusFaces*3);
		}

		spNormalDisplayer.SetModelAndNormalMatrix("matrices.modelMatrix", "matrices.normalMatrix", hmWorld.GetScaleMatrix());
		hmWorld.RenderHeightmapForNormals();
	}

	tTextures[6].BindTexture(); 

	
	// выводим салют
	salute.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	salute.iNumToGenerate = saluteParticlesAmount;
	salute.vGenColor = coloursArray[rand() % 7];
	saluteForm[0] = 5 + rand() % 20;
	saluteForm[1] = 20 + rand() % 200;
	saluteForm[2] = 5 + rand() % 20;
	salute.vGenVelocityRange = saluteForm;
	salute.UpdateParticles(appMain.sof(1.0f));
	salute.RenderParticles();

	// выводим снаряд
	bullet.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	bullet.iNumToGenerate = bulletParticlesAmount;
	bullet.UpdateParticles(appMain.sof(1.0f));
	bullet.RenderParticles();

	// выводим фонтан
	fountain.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	fountain.fGenSize = waterParticleSize;
	fountain.vGenColor = waterParticleColour;
	fountain.iNumToGenerate = waterParticlesAmount;
	fountain.UpdateParticles(appMain.sof(1.0f));
	fountain.RenderParticles();

	// выводим фонтан1
	fountain1.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	fountain1.vGenColor = coloursArray[rand() % 7];
	fountain1.vGenVelocityRange = speedArray[rand() % 3];
	fountain1.iNumToGenerate = water1ParticlesAmount;
	fountain1.UpdateParticles(appMain.sof(1.0f));
	fountain1.RenderParticles();

	// выводим фонтан2
	fountain2.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	currentSpeed[1] += 0.01;
	fountain2.vGenVelocityRange = currentSpeed;
	fountain2.vGenVelocityMin = currentSpeed;
	fountain2.iNumToGenerate = water1ParticlesAmount;
	fountain2.UpdateParticles(appMain.sof(1.0f));
	fountain2.RenderParticles();

	// выводим огонь
	fire.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);	
	fire.iNumToGenerate = fireParticlesAmount;
	fire.UpdateParticles(appMain.sof(1.0f));
	fire.RenderParticles();

	// выводим дым над фонтаном
	smoke.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	smoke.iNumToGenerate = smokeParticlesAmount;
	smoke.UpdateParticles(appMain.sof(1.0f));
	smoke.RenderParticles();

	// выводим пепел с неба
	ash.SetMatrices(oglControl->GetProjectionMatrix(), cCamera.vEye, cCamera.vView, cCamera.vUp);
	ash.UpdateParticles(appMain.sof(1.0f));
	ash.iNumToGenerate = ashParticlesAmount;	
	ash.RenderParticles();

	cCamera.Update();

	// Print something over scene
	
	spFont2D.UseProgram();
	glDisable(GL_DEPTH_TEST);
	spFont2D.SetUniform("matrices.projMatrix", oglControl->GetOrthoMatrix());

	int w = oglControl->GetViewportWidth(), h = oglControl->GetViewportHeight();
	
	spFont2D.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	ftFont.PrintFormatted(20, h-30, 20, "FPS: %d", oglControl->GetFPS());
	ftFont.PrintFormatted(20, h - 80, 20, "Particles (all systems): %d", 
		salute.GetNumParticles() +
		bullet.GetNumParticles() +
		fountain.GetNumParticles() + 
		fountain1.GetNumParticles() +
		fountain2.GetNumParticles() +
		fire.GetNumParticles() +
		smoke.GetNumParticles() +
		ash.GetNumParticles()
		);

	/*ftFont.PrintFormatted(20, h-110, 20, "Specular Intensity: %.2f (Press 'Q' and 'E' to change)", matShiny.fSpecularIntensity);
	if(Keys::Key('Q'))matShiny.fSpecularIntensity -= appMain.sof(0.2f);
	if(Keys::Key('E'))matShiny.fSpecularIntensity += appMain.sof(0.2f);

	ftFont.PrintFormatted(20, h-140, 20, "Specular Power: %.2f (Press 'Z' and 'C' to change)", matShiny.fSpecularPower);
	if(Keys::Key('Z'))matShiny.fSpecularPower -= appMain.sof(8.0f);
	if(Keys::Key('C'))matShiny.fSpecularPower += appMain.sof(8.0f);*/

	/*ftFont.PrintFormatted(20, h-100, 20, "Displaying Normals: %s (Press 'N' to toggle)", bDisplayNormals ? "Yes" : "Nope");
	if(Keys::Onekey('N'))bDisplayNormals = !bDisplayNormals;*/

	ftFont.PrintFormatted(20, h - 130, 20, "Displaying Fog: %s (Press 'U' to toggle)", FogParameters::showFog ? "Yes" : "No");
	//управление туманом
	if (Keys::Onekey('U'))FogParameters::showFog = !FogParameters::showFog;
	if (Keys::Key('I'))FogParameters::fDensity += appMain.sof(0.01f);
	if (Keys::Key('O'))FogParameters::fDensity -= appMain.sof(0.01f);

	ftFont.PrintFormatted(20, h - 160, 20, "Direct Light: %s (Press 'L' to toggle)", isDirectDisplayed ? "Yes" : "No");
	ftFont.PrintFormatted(20, h - 180, 20, "Point Light from fire: %s (Press 'K' to toggle)", isPointDisplayed ? "Yes" : "No");

	ftFont.PrintFormatted(20, h - 220, 20, "Displaying Salute: %s (Press '1' to toggle)", isSaluteDisplayed ? "Yes" : "No");
	ftFont.PrintFormatted(20, h - 240, 20, "Displaying Fountain: %s (Press '2' to toggle)", isFountainDisplayed ? "Yes" : "No");
	ftFont.PrintFormatted(20, h - 260, 20, "Displaying Fire: %s (Press '3' to toggle)", isFireDisplayed ? "Yes" : "No");
	//ftFont.PrintFormatted(20, h - 300, 20, "Displaying Salute: %s (Press '4' to toggle)", isShotDisplayed ? "Yes" : "No");
	ftFont.PrintFormatted(20, h - 280, 20, "Displaying Rain: %s (Press '5' to toggle)", isRainDisplayed ? "Yes" : "No");


	ftFont.PrintFormatted(20, h - 330, 20, "Fountain main jet particles size: %.2f (Press 'R' and 'T' to change)", waterParticleSize);
	if (Keys::Onekey('R'))waterParticleSize += 0.05;
	if (Keys::Onekey('T') && waterParticleSize > 0.05)waterParticleSize -= 0.05;

	ftFont.PrintFormatted(20, h-350, 20, "Fountain main jet particles colour (Press 'F' to change randomly");
	ftFont.PrintFormatted(20, h-370, 20, "and 'G' to change back to the original colour)");
	if (Keys::Onekey('F')){
		waterParticleColour = glm::vec3(
			(static_cast<double>(rand()) / (RAND_MAX)), 
			(static_cast<double>(rand()) / (RAND_MAX)), 
			(static_cast<double>(rand()) / (RAND_MAX)));
	}
	if (Keys::Onekey('G'))waterParticleColour = glm::vec3(1.0f, 0.5f, 0.0f);
	

	ftFont.PrintFormatted(20, h - 390, 20, "Fountain main jet particles amount: %d (Press 'V' and 'B' to change)", waterParticlesAmount);
	if (Keys::Onekey('V')) {
		if (isFountainDisplayed)waterParticlesAmount += 5;
	}
	if (Keys::Onekey('B') && waterParticlesAmount > 5){
		if (isFountainDisplayed)waterParticlesAmount -= 5;
	}
	
	//включение/выключение систем частиц
	//салют
	if (Keys::Onekey('1')){
		if (saluteParticlesAmount == 0) {
			saluteParticlesAmount = 20;
			bulletParticlesAmount = 1;
			isSaluteDisplayed = true;
		}
		else {
			saluteParticlesAmount = 0;
			bulletParticlesAmount = 0;
			isSaluteDisplayed = false;
		}
	}
	//фонтан
	if (Keys::Onekey('2')){
		if (waterParticlesAmount == 0) {
			waterParticlesAmount = 40;
			water1ParticlesAmount = 40;
			currentSpeed[1] = 4;
			isFountainDisplayed = true;
		} else {
			waterParticlesAmount = 0;
			water1ParticlesAmount = 0;
			isFountainDisplayed = false;
		}
	}
	//огонь и дым
	if (Keys::Onekey('3')){
		if (fireParticlesAmount == 0) {
			fireParticlesAmount = 30;
			smokeParticlesAmount = 10;
			if (!plFire.switchedOn) {
				plFire.Switch();
			}
			isFireDisplayed = true;
			isPointDisplayed = true;
		}
		else {
			fireParticlesAmount = 0;
			smokeParticlesAmount = 0;
			if (plFire.switchedOn) {
				plFire.Switch();
			}
			isFireDisplayed = false;
			isPointDisplayed = false;
		}
	}
	//выстрел
	if (Keys::Onekey('4')){
		/*isShotDisplayed = true;
		isShotDisplayed = false;*/
	}
	//пепел
	if (Keys::Onekey('5')){
		if (ashParticlesAmount == 0) {
			ashParticlesAmount = 40;
			isRainDisplayed = true;
		}
		else {
			ashParticlesAmount = 0;
			isRainDisplayed = false;
		}
	}
	

	ftFont.PrintFormatted(20, h - 430, 20, "Use ArrowUp and ArrowDown to rotate the torus object vertically;");
	ftFont.PrintFormatted(20, h - 450, 20, "Use ArrowLeft and ArrowRight to rotate the torus object horizontally;");

	//управление объектом
	if (Keys::Onekey(VK_UP))vertRotation -= appMain.sof(50.0f);
	if (Keys::Onekey(VK_DOWN))vertRotation += appMain.sof(50.0f);
	if (Keys::Onekey(VK_LEFT))horRotation -= appMain.sof(50.0f);
	if (Keys::Onekey(VK_RIGHT))horRotation += appMain.sof(50.0f);

	fVertAngle += appMain.sof(vertRotation);
	fHorAngle += appMain.sof(horRotation);
	
	ftFont.PrintFormatted(20, h - 500, 20, "Robot moving: %s (Press 'M' to toggle)", moveObject ? "Yes" : "No");
	if (Keys::Onekey('M')) {
		moveObject = !moveObject;
		moveAngle = 0;
	}

	ftFont.PrintFormatted(20, h - 550, 20, "KDZ2 by Lyange Dmitry");

	//управление точечным светом
	if (Keys::Onekey('K')) {
		plFire.Switch();
		isPointDisplayed = !isPointDisplayed;
	}

	glEnable(GL_DEPTH_TEST);	
	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);

	oglControl->SwapBuffers();
}

/*-----------------------------------------------

Name:    ReleaseScene

Params:  lpParam - Pointer to anything you want.

Result:  Releases OpenGL scene.

/*---------------------------------------------*/

void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	sbMainSkybox.DeleteSkybox();

	spMain.DeleteProgram();
	spOrtho2D.DeleteProgram();
	spFont2D.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();
	ftFont.DeleteFont();

	glDeleteVertexArrays(1, &uiVAOSceneObjects);
	vboSceneObjects.DeleteVBO();

	hmWorld.ReleaseHeightmap();
	CMultiLayeredHeightmap::ReleaseTerrainShaderProgram();
}