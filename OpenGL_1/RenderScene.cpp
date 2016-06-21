#include "common_header.h"

#include "win_OpenGLApp.h"

#include "shaders.h"

/*
Климаков Антон БПИ 121(2), семинар 13: OpenGL1, 4.02.2016
Реализовано:
1) изменен цвет фона
2) добавлены закрашенные фигуры
3) добавлен контур ракеты
*/

float fTriangle[9]; // Data to render triangle (3 vertices, each has 3 floats)
float fQuad[12]; // Data to render quad using triangle strips (4 vertices, each has 3 floats)
float fDuck[54];
float fMore[36];
float fTriangleColor[9];
float fQuadColor[12];
float fDuckColor[54];
float fMoreColor[36];

UINT uiVBO[8];
UINT uiVAO[4];

CShader shVertex, shFragment;
CShaderProgram spMain;

// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//Новый цвет фона
	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);

	// Setup triangle vertices
	fTriangle[0] = -0.9f; fTriangle[1] = 0.7f; fTriangle[2] = 0.0f;
	fTriangle[3] = -0.7f; fTriangle[4] = 0.9f; fTriangle[5] = 0.0f;
	fTriangle[6] = -0.5f; fTriangle[7] = 0.7f; fTriangle[8] = 0.0f;

	// Setup triangle color

	fTriangleColor[0] = 1.0f; fTriangleColor[1] = 0.0f; fTriangleColor[2] = 0.0f;
	fTriangleColor[3] = 0.0f; fTriangleColor[4] = 1.0f; fTriangleColor[5] = 0.0f;
	fTriangleColor[6] = 0.0f; fTriangleColor[7] = 0.0f; fTriangleColor[8] = 1.0f;
 
	// Setup quad vertices
 
	fQuad[0] = 0.7f; fQuad[1] = 0.9f; fQuad[2] = 0.0f;
	fQuad[3] = 0.7f; fQuad[4] = 0.6f; fQuad[5] = 0.0f;
	fQuad[6] = 0.9f; fQuad[7] = 0.9f; fQuad[8] = 0.0f;
	fQuad[9] = 0.9f; fQuad[10] = 0.6f; fQuad[11] = 0.0f;

	// Setup quad color

	fQuadColor[0] = 1.0f; fQuadColor[1] = 0.0f; fQuadColor[2] = 0.0f;
	fQuadColor[3] = 0.0f; fQuadColor[4] = 1.0f; fQuadColor[8] = 0.0f;
	fQuadColor[6] = 0.0f; fQuadColor[7] = 0.0f; fQuadColor[5] = 1.0f;
	fQuadColor[9] = 1.0f; fQuadColor[10] = 1.0f; fQuadColor[11] = 0.0f;

	// Setup duck vertices

	fDuck[0] = 0.0f; fDuck[1] = 0.8f;  fDuck[2] = 0.0f;
	fDuck[3] = -0.2f; fDuck[4] = 0.4f;  fDuck[5] = 0.0f;
	fDuck[6] = -0.2f; fDuck[7] = -0.3f;  fDuck[8] = 0.0f;
	fDuck[9] = -0.3f; fDuck[10] = -0.5f; fDuck[11] = 0.0f;
	fDuck[12] = -0.3f; fDuck[13] = -0.9f; fDuck[14] = 0.0f;
	fDuck[15] = -0.2f; fDuck[16] = -0.9f; fDuck[17] = 0.0f;
	fDuck[18] = -0.2f; fDuck[19] = -0.7f; fDuck[20] = 0.0f;
	fDuck[21] = -0.05f; fDuck[22] = -0.7f; fDuck[23] = 0.0f;
	fDuck[24] = -0.05f; fDuck[25] = -0.9f; fDuck[26] = 0.0f;
	fDuck[27] = 0.05f; fDuck[28] = -0.9f; fDuck[29] = 0.0f;
	fDuck[30] = 0.05f; fDuck[31] = -0.7f; fDuck[32] = 0.0f;
	fDuck[33] = 0.2f; fDuck[34] = -0.7f; fDuck[35] = 0.0f;
	fDuck[36] = 0.2f; fDuck[37] = -0.9f; fDuck[38] = 0.0f;
	fDuck[39] = 0.3f; fDuck[40] = -0.9f; fDuck[41] = 0.0f;
	fDuck[42] = 0.3f; fDuck[43] = -0.5f; fDuck[44] = 0.0f;
	fDuck[45] = 0.2f; fDuck[46] = -0.3f; fDuck[47] = 0.0f;
	fDuck[48] = 0.2f; fDuck[49] = 0.4f; fDuck[50] = 0.0f;
	fDuck[51] = 0.0f; fDuck[52] = 0.8f;  fDuck[53] = 0.0f;

	// Setup duck color

	fDuckColor[0] = 1.0f; fDuckColor[1] = 0.0f;  fDuckColor[2] = 0.0f;
	fDuckColor[3] = 1.0f; fDuckColor[4] = 1.0f;  fDuckColor[5] = 0.0f;
	fDuckColor[6] = 1.0f; fDuckColor[7] = 0.0f;  fDuckColor[8] = 1.0f;
	fDuckColor[9] = 1.0f; fDuckColor[10] = 0.0f; fDuckColor[11] = 0.0f;
	fDuckColor[12] = 1.0f; fDuckColor[13] = 1.0f; fDuckColor[14] = 0.0f;
	fDuckColor[15] = 1.0f; fDuckColor[16] = 0.0f; fDuckColor[17] = 1.0f;
	fDuckColor[18] = 1.0f; fDuckColor[19] = 0.0f; fDuckColor[20] = 0.0f;
	fDuckColor[21] = 1.0f; fDuckColor[22] = 1.0f; fDuckColor[23] = 0.0f;
	fDuckColor[24] = 1.0f; fDuckColor[25] = 0.0f; fDuckColor[26] = 1.0f;
	fDuckColor[27] = 1.0f; fDuckColor[28] = 0.0f; fDuckColor[29] = 0.0f;
	fDuckColor[30] = 1.0f; fDuckColor[31] = 1.0f; fDuckColor[32] = 0.0f;
	fDuckColor[33] = 0.2f; fDuckColor[34] = -0.7f; fDuckColor[35] = 0.0f;
	fDuckColor[36] = 0.2f; fDuckColor[37] = -0.9f; fDuckColor[38] = 0.0f;
	fDuckColor[39] = 0.3f; fDuckColor[40] = -0.9f; fDuckColor[41] = 0.0f;
	fDuckColor[42] = 0.3f; fDuckColor[43] = -0.5f; fDuckColor[44] = 0.0f;
	fDuckColor[45] = 0.2f; fDuckColor[46] = -0.3f; fDuckColor[47] = 0.0f;
	fDuckColor[48] = 0.2f; fDuckColor[49] = -0.3f; fDuckColor[50] = 0.0f;
	fDuckColor[51] = 1.0f; fDuckColor[52] = 0.0f;  fDuckColor[53] = 0.0f;

	// Setup duck vertices

	fMore[0] = 0.0f; fMore[1] = 0.0f;  fMore[2] = 0.0f;
	fMore[3] = 0.5f; fMore[4] = 0.0f;  fMore[5] = 0.0f;
	fMore[6] = 0.5f; fMore[7] = 0.5f;  fMore[8] = 0.0f;

	fMore[9] = 0.0f; fMore[10] = 0.0f; fMore[11] = 0.0f;
	fMore[12] = 0.0f; fMore[13] = 0.5f; fMore[14] = 0.0f;
	fMore[15] = -0.5f; fMore[16] = 0.5f; fMore[17] = 0.0f;

	fMore[18] = 0.0f; fMore[19] = 0.0f; fMore[20] = 0.0f;
	fMore[21] = -0.5f; fMore[22] = 0.0f; fMore[23] = 0.0f;
	fMore[24] = -0.5f; fMore[25] = -0.5f; fMore[26] = 0.0f;

	fMore[27] = 0.0f; fMore[28] = 0.0f; fMore[29] = 0.0f;
	fMore[30] = 0.0f; fMore[31] = -0.5f; fMore[32] = 0.0f;
	fMore[33] = 0.5f; fMore[34] = -0.5f; fMore[35] = 0.0f;

	for (int i = 0; i < 36; i++) {
		fMore[i] = fMore[i] * 0.4 - 0.6f;
	}

	// Setup duck color

	fMoreColor[0] = 1.0f; fMoreColor[1] = 1.0f;  fMoreColor[2] = 1.0f;
	fMoreColor[3] = 1.0f; fMoreColor[4] = 1.0f;  fMoreColor[5] = 1.0f;
	fMoreColor[6] = 1.0f; fMoreColor[7] = 1.0f;  fMoreColor[8] = 1.0f;
	fMoreColor[9] = 1.0f; fMoreColor[10] = 0.0f; fMoreColor[11] = 0.0f;
	fMoreColor[12] = 1.0f; fMoreColor[13] = 1.0f; fMoreColor[14] = 0.0f;
	fMoreColor[15] = 1.0f; fMoreColor[16] = 0.0f; fMoreColor[17] = 1.0f;
	fMoreColor[18] = 1.0f; fMoreColor[19] = 1.0f; fMoreColor[20] = 1.0f;
	fMoreColor[21] = 1.0f; fMoreColor[22] = 1.0f; fMoreColor[23] = 1.0f;
	fMoreColor[24] = 1.0f; fMoreColor[25] = 1.0f; fMoreColor[26] = 1.0f;
	fMoreColor[27] = 1.0f; fMoreColor[28] = 0.0f; fMoreColor[29] = 0.0f;
	fMoreColor[30] = 1.0f; fMoreColor[31] = 1.0f; fMoreColor[32] = 0.0f;
	fMoreColor[33] = 0.2f; fMoreColor[34] = -0.7f; fMoreColor[35] = 0.0f;

	glGenVertexArrays(4, uiVAO); // Generate two VAOs, one for triangle and one for quad
	glGenBuffers(8, uiVBO); // And four VBOs

	// Setup whole triangle
	glBindVertexArray(uiVAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangle, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangleColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Setup whole quad
	glBindVertexArray(uiVAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), fQuad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[3]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), fQuadColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Setup whole quad
	glBindVertexArray(uiVAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[4]);
	glBufferData(GL_ARRAY_BUFFER, 54 * sizeof(float), fDuck, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[5]);
	glBufferData(GL_ARRAY_BUFFER, 54 * sizeof(float), fDuckColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Setup whole quad
	glBindVertexArray(uiVAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[6]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(float), fMore, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[7]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(float), fMoreColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Load shaders and create shader program

	shVertex.LoadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shFragment.LoadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

	spMain.CreateProgram();
	spMain.AddShaderToProgram(&shVertex);
	spMain.AddShaderToProgram(&shFragment);

	spMain.LinkProgram();
	spMain.UseProgram();
}

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	// We just clear color
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(uiVAO[1]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(uiVAO[2]);
	glDrawArrays(GL_LINE_STRIP, 0, 18);

	glBindVertexArray(uiVAO[3]);
	glDrawArrays(GL_TRIANGLES, 0, 12);

	oglControl->SwapBuffersM();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	spMain.DeleteProgram();

	shVertex.DeleteShader();
	shFragment.DeleteShader();
}