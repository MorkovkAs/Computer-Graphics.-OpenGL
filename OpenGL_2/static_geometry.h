#pragma once

#include "vertexBufferObject.h"

extern glm::vec3 vCubeVertices[36];
extern glm::vec2 vCubeTexCoords[6];
extern glm::vec3 vGround[6];
extern glm::vec3 vPyramidVertices[16];
extern glm::vec2 vPyramidTexCoords[3];
extern glm::vec3 vPyramid3My[12];

int generateTorus(CVertexBufferObject &vboDest, float fRadius, float fTubeRadius, int iSubDivAround, int iSubDivTube);