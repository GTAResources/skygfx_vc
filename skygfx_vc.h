#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <rwcore.h>
#include <rwplcore.h>
#include <rpworld.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "resource.h"
#include "MemoryMgr.h"

struct MatFXEnv
{
	RwFrame *envFrame;
	RwTexture *envTex;
	float envCoeff;
	int envFBalpha;
	int pad;
	int effect;
};


struct MatFX
{
	MatFXEnv fx[2];
	int effects;
};

int initD3D9(void *d3d8device);
void RwD3D9SetVertexShader(void *shader);
RwBool RwD3D9CreateVertexShader(const RwUInt32 *function, void **shader);
void RwD3D9SetFVF(RwUInt32 fvf);
void RwD3D9SetVertexShaderConstant(RwUInt32 registerAddress, const void *constantData, RwUInt32 constantCount);

enum {
	LOC_world       = 0,
	LOC_worldIT     = 4,
	LOC_view        = 8,
	LOC_proj        = 12,
	LOC_eye         = 16,
	LOC_ambient     = 17,
	LOC_directDir   = 18,
	LOC_directDiff  = 19,
	LOC_directSpec  = 20,
	LOC_lights      = 21,

	LOC_matCol      = 29,
	LOC_surfProps   = 30,
};
