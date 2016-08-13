#include "skygfx.h"
#include "d3d8.h"
#include "d3d8types.h"

/*
 * The following code is fairly close to the Xbox code.
 * If you don't like it, complain to R* Vienna.
 * The tweaking values look different on the real xbox somehow :/
 */


class RimPipe : public CustomPipe
{
	void CreateShaders(void);
	void LoadTweakingTable(void);
public:
	static void *vertexShader;
	static InterpolatedColor rampStart;
	static InterpolatedColor rampEnd;
	static InterpolatedFloat offset;
	static InterpolatedFloat scale;
	static InterpolatedFloat scaling;

	RimPipe(void);
	static RimPipe *Get(void);
	void Init(void);
	static void RenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	static void ShaderSetup(void);
	static void ObjectSetup(void);
	static void RenderMesh(RxD3D8InstanceData *inst);
};

void *RimPipe::vertexShader;
InterpolatedColor RimPipe::rampStart(Color(0.0f, 0.0f, 0.0f, 1.0f));
InterpolatedColor RimPipe::rampEnd(Color(1.0f, 1.0f, 1.0f, 1.0f));
InterpolatedFloat RimPipe::offset(0.5f);
InterpolatedFloat RimPipe::scale(1.5f);
InterpolatedFloat RimPipe::scaling(2.0f);

RxPipeline *&skinpipe = *AddressByVersion<RxPipeline**>(0x663CAC, 0x663CAC, 0x673DB0, 0x78A0D4, 0x78A0DC, 0x7890DC);

void neoRimPipeInit(void)
{
	RxPipelineNode *node;
	RimPipe::Get()->Init();
	node = RxPipelineFindNodeByName(skinpipe, "nodeD3D8SkinAtomicAllInOne.csl", NULL, NULL);
	*(void**)node->privateData = RimPipe::RenderCallback;
}

extern "C" {
__declspec(dllexport) void
AttachRimPipeToRwObject(RwObject *obj)
{
	if(RwObjectGetType(obj) == rpATOMIC)
		RimPipe::Get()->Attach((RpAtomic*)obj);
	else if(RwObjectGetType(obj) == rpCLUMP)
		RpClumpForAllAtomics((RpClump*)obj, CustomPipe::setatomicCB, RimPipe::Get());
}
}

RimPipe::RimPipe(void)
{
	rwPipeline = NULL;
}

RimPipe*
RimPipe::Get(void)
{
	static RimPipe rimpipe;
	return &rimpipe;
}

void
RimPipe::Init(void)
{
	CreateRwPipeline();
	SetRenderCallback(RenderCallback);
	LoadTweakingTable();
	CreateShaders();
}

void
RimPipe::CreateShaders(void)
{
	HRSRC resource;
	RwUInt32 *shader;
	resource = FindResource(dllModule, MAKEINTRESOURCE(IDR_RIMVS), RT_RCDATA);
	shader = (RwUInt32*)LoadResource(dllModule, resource);
	RwD3D9CreateVertexShader(shader, &vertexShader);
	assert(RimPipe::vertexShader);
	FreeResource(shader);
}

void
RimPipe::LoadTweakingTable(void)
{

	char *path;
	FILE *dat;
	path = getpath("neo\\rimTweakingTable.dat");
	if(path == NULL){
		MessageBox(NULL, "Couldn't load 'neo\\rimTweakingTable.dat'", "Error", MB_ICONERROR | MB_OK);
		exit(0);
	}
	dat = fopen(path, "r");
	neoReadWeatherTimeBlock(dat, &rampStart);
	neoReadWeatherTimeBlock(dat, &rampEnd);
	neoReadWeatherTimeBlock(dat, &offset);
	neoReadWeatherTimeBlock(dat, &scale);
	neoReadWeatherTimeBlock(dat, &scaling);
	fclose(dat);
}

//
// Rendering
//

void
RimPipe::ShaderSetup(void)
{
	D3DMATRIX worldMat, viewMat, projMat;

	RwD3D8GetTransform(D3DTS_WORLD, &worldMat);
	RwD3D8GetTransform(D3DTS_VIEW, &viewMat);
	RwD3D8GetTransform(D3DTS_PROJECTION, &projMat);
	RwD3D9SetVertexShaderConstant(LOC_world, (void*)&worldMat, 4);
	RwD3D9SetVertexShaderConstant(LOC_worldIT, (void*)&worldMat, 4);
	RwD3D9SetVertexShaderConstant(LOC_view, (void*)&viewMat, 4);
	RwD3D9SetVertexShaderConstant(LOC_proj, (void*)&projMat, 4);

	RwMatrix *camfrm = RwFrameGetLTM(RwCameraGetFrame((RwCamera*)((RwGlobals*)RwEngineInst)->curCamera));
	RwD3D9SetVertexShaderConstant(LOC_eye, (void*)RwMatrixGetPos(camfrm), 1);

	if(pAmbient)
		UploadLightColor(pAmbient, LOC_ambient);
	else
		UploadZero(LOC_ambient);
	if(pDirect){
		UploadLightColor(pDirect, LOC_directCol);
		UploadLightDirection(pDirect, LOC_directDir);
	}else{
		UploadZero(LOC_directCol);
		UploadZero(LOC_directDir);
	}
	for(int i = 0 ; i < 4; i++)
		if(pExtraDirectionals[i]){
			UploadLightDirection(pExtraDirectionals[i], LOC_lights+i*2);
			UploadLightColor(pExtraDirectionals[i], LOC_lights+1+i*2);
		}else{
			UploadZero(LOC_lights+i*2);
			UploadZero(LOC_lights+1+i*2);
		}

						{
							static bool keystate = false;
							if(GetAsyncKeyState(rimlightkey) & 0x8000){
								if(!keystate){
									keystate = true;
									rimlight = (rimlight+1)%2;
								}
							}else
								keystate = false;
						}
	// Rim values
	Color c = rampStart.Get();
	RwD3D9SetVertexShaderConstant(LOC_rampStart, (void*)&c, 1);
	c = rampEnd.Get();
	RwD3D9SetVertexShaderConstant(LOC_rampEnd, (void*)&c, 1);
	float f[4];
	f[0] = offset.Get();
	f[1] = scale.Get();
	f[2] = scaling.Get();
	if(!rimlight)
		f[2] = 0.0f;
	f[3] = 1.0f;
	RwD3D9SetVertexShaderConstant(LOC_rim, (void*)f, 1);
}

void
RimPipe::ObjectSetup(void)
{
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	RwD3D8SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwD3D9SetVertexShader(vertexShader);                                   // 9!
	RwD3D8SetPixelShader(NULL);                                            // 8!
}

//int textured = 1;

void
RimPipe::RenderMesh(RxD3D8InstanceData *inst)
{
						//	{
						//		static bool keystate = false;
						//		if(GetAsyncKeyState(VK_F4) & 0x8000){
						//			if(!keystate){
						//				keystate = true;
						//				textured = (textured+1)%2;
						//			}
						//		}else
						//			keystate = false;
						//	}

	RwD3D8SetStreamSource(0, inst->vertexBuffer, inst->stride);
	RwD3D9SetFVF(inst->vertexShader);				       // 9!
	RwRGBAReal color;
	RwRGBARealFromRwRGBA(&color, &inst->material->color);
	RwD3D9SetVertexShaderConstant(LOC_matCol, (void*)&color, 1);

	RwD3D8SetTexture(inst->material->texture, 0);
	// have to set these after the texture, RW sets texture stage states automatically
//	if(textured)
		RwD3D8SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//	else
//		RwD3D8SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	RwD3D8SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	RwD3D8SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);

	RwUInt32 tf = D3DCOLOR_ARGB(inst->material->color.alpha, 0xFF, 0xFF, 0xFF);
	if(tf == 0xFFFFFFFF){
		RwD3D8SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RwD3D8SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}else{
		RwD3D8SetRenderState(D3DRS_TEXTUREFACTOR, tf);
		RwD3D8SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RwD3D8SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		RwD3D8SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		RwD3D8SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		RwD3D8SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	}

	drawDualPass(inst);
}

void
RimPipe::RenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{
	ShaderSetup();

	RxD3D8ResEntryHeader *header = (RxD3D8ResEntryHeader*)&repEntry[1];
	RxD3D8InstanceData *inst = (RxD3D8InstanceData*)&header[1];

	ObjectSetup();
	for(int i = 0; i < header->numMeshes; i++){
		RenderMesh(inst);
		inst++;
	}
	RwD3D8SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RwD3D8SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	RwD3D8SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RwD3D8SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}
