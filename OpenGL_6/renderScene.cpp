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

#include "framebuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUMTEXTURES 5
#define FOG_EQUATION_LINEAR		0
#define FOG_EQUATION_EXP		1
#define FOG_EQUATION_EXP2		2

/*
	OpenGL_6,  лимаков јнтон, Visual Studio 2013
		–еализовано:
            Ц3D-объекты на сцену  (до 2 баллов)
            ЦN точечных источников освещени€ (N > 2), можно разного цвета  (до 2 баллов)
			-включение/ отключение источников освещени€ (включены все, выключены все, включены некоторые) (до 2 баллов)
		ƒополнительное управление:
			1 - включение / выключение 1-ого источника
			2 - включение / выключение 2-ого источника
			3 - включение / выключение 3-ого источника
			5 - включение всех источников
			6 - выключение всех источников
*/


/* One VBO, where all static data are stored now,
in this tutorial vertex is stored as 3 floats for
position, 2 floats for texture coordinate and
3 floats for normal vector. */

CVertexBufferObject vboSceneObjects, vboShadowMapQuad;
UINT uiVAOs[1]; // Only one VAO now

CFlyingCamera cCamera;

CSkybox sbMainSkybox;
CObjModel mdlHouse;
CObjModel mdlBoat;
CDirectionalLight dlSun;
CPointLight plLight1;
CPointLight plLight2;
CPointLight plLight3;

CFramebuffer fboShadowMap;
bool bShadowsOn = true;
bool bDisplayShadowMap = true;
int iShadowMapTextureSize = 1024;
int torusFaces;
// 3 objects
int renderVert[3];

float fRotationVertAngleCube = 0.0f, fRotationHorAngleCube = 0.0f;
float fCubeVertRotationSpeed = 0.0f, fCubeHorRotationSpeed = 0.0f;
float torusAngleX = 1.0f, torusAngleY = 1.0f, torusAngleZ = 1.0f, torusSumX = 0.0f, torusSumY = 0.0f, torusSumZ = 0.0f;
float torusAngle = 0.0f;

//три точечных источника, переменные показывают, включены ли источники
bool plLight1IsWorking = true;
bool plLight2IsWorking = true;
bool plLight3IsWorking = true;

UINT uiVAOShadowMapQuad;

#include "static_geometry.h"

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

	renderVert[0] = AddSceneObjects(vboSceneObjects);
	renderVert[1] = AddCube(vboSceneObjects);
	renderVert[2] = GenerateTorus(vboSceneObjects, 1.4f, 0.3f, 100, 100);


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
	LoadAllTextures();
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 10.0f, 120.0f), glm::vec3(0.0f, 10.0f, 119.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.001f);
	cCamera.SetMovingKeys('W', 'S', 'A', 'D', VK_SPACE);

	sbMainSkybox.LoadSkybox("data\\skyboxes\\jajlands1\\", "jajlands1_ft.jpg", "jajlands1_bk.jpg", "jajlands1_lf.jpg", "jajlands1_rt.jpg", "jajlands1_up.jpg", "jajlands1_dn.jpg");

	dlSun = CDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(sqrt(2.0f) / 2, -sqrt(2.0f) / 2, 0), 1.0f, 0);
	plLight1 = CPointLight(glm::vec3(0.9f, 0.4f, 0.4f), glm::vec3(66.0f, 20.1f, 35.0f), 0.15f, 0.3f, 0.007f, 0.002f);
	plLight2 = CPointLight(glm::vec3(0.5f, 0.2f, 0.6f), glm::vec3(10.0f, 15.1f, -80.0f), 0.15f, 0.3f, 0.007f, 0.002f);
	plLight3 = CPointLight(glm::vec3(0.1f, 0.7f, 0.3f), glm::vec3(-70.0f, 17.1f, 35.0f), 0.15f, 0.3f, 0.007f, 0.002f);

	mdlHouse.LoadModel("data\\models\\house\\house.obj", "house.mtl");
	mdlBoat.LoadModel("data\\models\\boat\\OldBoat.obj", "OldBoat.mtl");

	glm::vec3 vShadowMapQuad[] =
	{
		glm::vec3(0.75f, 1.0f, 0.0f),
		glm::vec3(0.75f, 0.5f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.5f, 0.0f)
	};

	glm::vec2 vShadowMapQuadTC[] =
	{
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f)
	};

	vboShadowMapQuad.CreateVBO();
	FOR(i, 4)
	{
		vboShadowMapQuad.AddData(&vShadowMapQuad[i], sizeof(glm::vec3));
		vboShadowMapQuad.AddData(&vShadowMapQuadTC[i], sizeof(glm::vec2));
	}
	glGenVertexArrays(1, &uiVAOShadowMapQuad);
	glBindVertexArray(uiVAOShadowMapQuad);

	vboShadowMapQuad.BindVBO();
	vboShadowMapQuad.UploadDataToGPU(GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (void*)sizeof(glm::vec3));

	fboShadowMap.CreateFramebufferWithTexture(iShadowMapTextureSize, iShadowMapTextureSize);
	fboShadowMap.AddDepthBuffer();
	fboShadowMap.SetFramebufferTextureFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_NEAREST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	torusAngle += 0.005f;
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	// This values set the darkness of whole scene (direction of light), that's why such name of variable :D
	static float fAngleOfDarkness = 45.0f;

	glm::mat4 mDepthBiasMVP, mModelMatrix, mView;
	const float PI = float(atan(1.0)*4.0);

	if (bShadowsOn) // So if the shadows are on
	{
		// We are going to render scene from the light's point of view
		fboShadowMap.BindFramebuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		spShadowMapper.UseProgram();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//spShadowMapper.UseProgram();

		// Because we have a directional light, we just set it high enough (vLightPos) so that it sees all objects on scene
		// We also create orthographics projection matrix for the purposes of rendering shadows
		const float fRangeX = 300, fRangeY = 300, fMinZ = 0.05f, fMaxZ = 1500;
		//const float fRangeX = 150, fRangeY = 150, fMinZ = 0.125f, fMaxZ = 512;
		glm::mat4 mPROJ = glm::ortho<float>(-fRangeX, fRangeX, -fRangeY, fRangeY, fMinZ, fMaxZ);
		glm::vec3 vLightPos = -dlSun.vDirection*256.0f;
		glm::mat4 mViewFromLight = glm::lookAt(vLightPos, glm::vec3(0, 0, 0), glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), dlSun.vDirection));//glm::vec3(0, 1, 0));


		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
			);

		// Calculate depth bias matrix to calculate shadow coordinates in shader programs
		mDepthBiasMVP = biasMatrix * mPROJ * mViewFromLight;

		// Rendering the scene to shadowmap
		glBindVertexArray(uiVAOs[0]);

		glm::mat4 depthMVP;
		tTextures[3].BindTexture();
		mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(16.0f, 16.0f, 16.0f));
		mModelMatrix = glm::translate(mModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
		mModelMatrix = glm::rotate(mModelMatrix, fRotationVertAngleCube, glm::vec3(1.0f, 0.0f, 0.0f));
		mModelMatrix = glm::rotate(mModelMatrix, fRotationHorAngleCube, glm::vec3(0.0f, 1.0f, 0.0f));
		depthMVP = mPROJ * mViewFromLight * mModelMatrix;
		spShadowMapper.SetUniform("depthMVP", depthMVP);
		glDrawArrays(GL_TRIANGLES, renderVert[0], renderVert[1]);

		tTextures[2].BindTexture();
		mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(16.0f, 16.0f, 16.0f));
		mModelMatrix = glm::translate(mModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
		mModelMatrix = glm::rotate(mModelMatrix, torusAngle, glm::vec3(1.0f, 0.0f, 1.0f));
		depthMVP = mPROJ * mViewFromLight * mModelMatrix;
		spShadowMapper.SetUniform("depthMVP", depthMVP);
		glDrawArrays(GL_TRIANGLES, renderVert[0] + renderVert[1], renderVert[2]);

		FOR(i, 3)
		{
			glm::vec3 vPos = glm::vec3(0.0f, 0.0f, -10.0f);
			mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(8.0f, 8.0f, 8.0f));
			mModelMatrix = glm::rotate(mModelMatrix, i*120.0f*PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			mModelMatrix = glm::translate(mModelMatrix, vPos);
			depthMVP = mPROJ * mViewFromLight * mModelMatrix;
			spShadowMapper.SetUniform("depthMVP", depthMVP);
			mdlHouse.RenderModel();
		}

		FOR(i, 3)
		{
			glm::vec3 vPos = glm::vec3(50.0f, 0.0f, -80.0f);
			mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
			mModelMatrix = glm::rotate(mModelMatrix, i*120.0f*PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			mModelMatrix = glm::translate(mModelMatrix, vPos);
			depthMVP = mPROJ * mViewFromLight * mModelMatrix;
			spShadowMapper.SetUniform("depthMVP", depthMVP);
			mdlBoat.RenderModel();
		}

		// Now the shadows are rendered, we can go back to normal rendering
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mModelMatrix = glm::translate(glm::mat4(1.0f), cCamera.vEye);
	oglControl->ResizeOpenGLViewportFull();

	spSkybox.UseProgram();

	spSkybox.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spSkybox.SetUniform("matrices.viewMatrix", cCamera.Look());

	spSkybox.SetUniform("vColor", glm::vec4(1, 1, 1, 1));
	spSkybox.SetUniform("gSampler", 0);
	spSkybox.SetUniform("matrices.modelMatrix", mModelMatrix);
	spSkybox.SetUniform("matrices.normalMatrix", mModelMatrix);

	sbMainSkybox.RenderSkybox();

	spMain.UseProgram();

	spMain.SetUniform("matrices.projMatrix", oglControl->GetProjectionMatrix());
	spMain.SetUniform("gSampler", 0);
	mView = cCamera.Look();
	spMain.SetUniform("matrices.viewMatrix", &mView);

	spMain.SetUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	spMain.SetUniform("matrices.modelMatrix", glm::mat4(1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::mat4(1.0f));

	glBindVertexArray(uiVAOs[0]);

	//static float fAngle = 45.0f;
	// You can play with direction of light with '+' and '-' key
	if (Keys::Key(VK_ADD))fAngleOfDarkness += appMain.sof(90);
	if (Keys::Key(VK_SUBTRACT))fAngleOfDarkness -= appMain.sof(90);

	//if (fAngleOfDarkness < -90.0f) fAngleOfDarkness = -90.0f;
	//if (fAngleOfDarkness > 90.0f) fAngleOfDarkness = 90.0f;

	// Set the directional vector of light
	dlSun.vDirection = glm::vec3(-sin(fAngleOfDarkness*PI / 180.0f), cos(-fAngleOfDarkness*PI / 180.0f), 0.0f);
	dlSun.SetUniformData(&spMain, "sunLight");

	spMain.SetUniform("matrices.depthBiasMVP", mDepthBiasMVP);
	// Bind shadow map to 5th texture unit (index is 5, if we count from 1 it's 6th)
	// You can use whichever texture unit you want, I used 5 because of 5 textures in terrain and 6th first available
	fboShadowMap.BindFramebufferTexture(5, false);
	spMain.SetUniform("shadowMap", 5);
	spMain.SetUniform("bShadowsOn", bShadowsOn ? 1 : 0);

	//3 точечных источника
	plLight1.SetUniformData(&spMain, "pointLight1");
	plLight2.SetUniformData(&spMain, "pointLight2");
	plLight3.SetUniformData(&spMain, "pointLight3");

	spMain.UseProgram();

	int renderTag = 0;
	
	// Render ground
	tTextures[0].BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, renderVert[0]);
	renderTag += renderVert[0];

	//render cube
	tTextures[3].BindTexture();
	mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(16.0f, 16.0f, 16.0f));
	mModelMatrix = glm::translate(mModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
	mModelMatrix = glm::rotate(mModelMatrix, fRotationVertAngleCube, glm::vec3(1.0f, 0.0f, 0.0f));
	mModelMatrix = glm::rotate(mModelMatrix, fRotationHorAngleCube, glm::vec3(0.0f, 1.0f, 0.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	glDrawArrays(GL_TRIANGLES, renderTag, renderVert[1]);
	renderTag += renderVert[1];
	
	//render torus
	tTextures[2].BindTexture();
	mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(16.0f, 16.0f, 16.0f));
	mModelMatrix = glm::translate(mModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
	mModelMatrix = glm::rotate(mModelMatrix, torusAngle, glm::vec3(1.0f, 0.0f, 1.0f));
	spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
	spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
	glDrawArrays(GL_TRIANGLES, renderTag, renderVert[2]);

	//render houses
	FOR(i, 3)
	{
		glm::vec3 vPos = glm::vec3(0.0f, 0.0f, -10.0f);
		mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(8.0f, 8.0f, 8.0f));
		mModelMatrix = glm::rotate(mModelMatrix, i*120.0f*PI/180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		mModelMatrix = glm::translate(mModelMatrix, vPos);
		spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
		spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
		mdlHouse.RenderModel();
	}

	FOR(i, 3)
	{
		glm::vec3 vPos = glm::vec3(50.0f, 0.0f, -80.0f);
		mModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
		mModelMatrix = glm::rotate(mModelMatrix, i*120.0f*PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		mModelMatrix = glm::translate(mModelMatrix, vPos);
		spMain.SetUniform("matrices.normalMatrix", glm::transpose(glm::inverse(mModelMatrix)));
		spMain.SetUniform("matrices.modelMatrix", &mModelMatrix);
		mdlBoat.RenderModel();
	}

	if (bDisplayShadowMap)
	{
		// Display shadow map
		spShadowMapRender.UseProgram();
		glBindVertexArray(uiVAOShadowMapQuad);
		fboShadowMap.BindFramebufferTexture();
		spShadowMapRender.SetUniform("samplerShadow", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	cCamera.Update();

	// A little interaction for user

	if (Keys::Onekey(VK_UP))fCubeVertRotationSpeed -= appMain.sof(600.0f);
	if (Keys::Onekey(VK_DOWN))fCubeVertRotationSpeed += appMain.sof(600.0f);
	if (Keys::Onekey(VK_LEFT))fCubeHorRotationSpeed -= appMain.sof(600.0f);
	if (Keys::Onekey(VK_RIGHT))fCubeHorRotationSpeed += appMain.sof(600.0f);

	fRotationVertAngleCube += appMain.sof(fCubeVertRotationSpeed);
	fRotationHorAngleCube += appMain.sof(fCubeHorRotationSpeed);

	if (Keys::Onekey('R'))bShadowsOn = !bShadowsOn;
	if (Keys::Onekey('M'))bDisplayShadowMap = !bDisplayShadowMap;

	if(Keys::Onekey(VK_ESCAPE))PostQuitMessage(0);
	oglControl->SwapBuffers();

	if (Keys::Onekey('1')) //точечный источник света 1, ручное управление
	if (plLight1IsWorking == true)
		plLight1.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight1IsWorking = false;
	else
		plLight1.vColor = glm::vec3(0.9f, 0.4f, 0.4f), plLight1IsWorking = true;

	if (Keys::Onekey('2')) //точечный источник света 2, ручное управление
	if (plLight2IsWorking == true)
		plLight2.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight2IsWorking = false;
	else
		plLight2.vColor = glm::vec3(0.5f, 0.2f, 0.6f), plLight2IsWorking = true;

	if (Keys::Onekey('3')) //точечный источник света 3, ручное управление
	if (plLight3IsWorking == true)
		plLight3.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight3IsWorking = false;
	else
		plLight3.vColor = glm::vec3(0.1f, 0.7f, 0.3f), plLight3IsWorking = true;

	if (Keys::Onekey('5')) {//включение всех
		plLight1.vColor = glm::vec3(0.9f, 0.4f, 0.4f), plLight1IsWorking = true;
		plLight2.vColor = glm::vec3(0.5f, 0.2f, 0.6f), plLight2IsWorking = true;
		plLight3.vColor = glm::vec3(0.1f, 0.7f, 0.3f), plLight3IsWorking = true;
	}

	if (Keys::Onekey('6')) {//выключение всех
		plLight1.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight1IsWorking = false;
		plLight2.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight2IsWorking = false;
		plLight3.vColor = glm::vec3(0.0f, 0.0f, 0.0f), plLight3IsWorking = false;
	}	
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].DeleteTexture();
	sbMainSkybox.DeleteSkybox();
	spMain.DeleteProgram();
	spColor.DeleteProgram();
	spShadowMapper.DeleteProgram();
	spShadowMapRender.DeleteProgram();
	spSkybox.DeleteProgram();
	FOR(i, NUMSHADERS)shShaders[i].DeleteShader();

	glDeleteVertexArrays(1, uiVAOs);
	vboSceneObjects.DeleteVBO();
	vboShadowMapQuad.DeleteVBO();
	mdlHouse.DeleteModel();
}