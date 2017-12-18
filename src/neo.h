extern RwTexDictionary *neoTxd;

void RwToD3DMatrix(void *d3d, RwMatrix *rw);
void MakeProjectionMatrix(void *d3d, RwCamera *cam, float nbias = 0.0f, float fbias = 0.0f);
enum {
	LOC_combined    = 0,
	LOC_world       = 4,
	LOC_tex         = 8,
	LOC_eye         = 12,
	LOC_directDir   = 13,
	LOC_ambient     = 15,
	LOC_matCol      = 16,
	LOC_directCol   = 17,
	LOC_lightDir    = 18,
	LOC_lightCol    = 22,

	LOC_directSpec  = 26,	// for carpipe
	LOC_reflProps   = 27,

	LOC_gloss2	= 28,
	LOC_gloss3	= 29,

	LOC_rampStart   = 36,	// for rim pipe
	LOC_rampEnd     = 37,
	LOC_rim         = 38,
	LOC_viewVec     = 41,
};

#define NUMWEATHERS 7	// 4 for III

struct Color
{
	float r, g, b, a;
	Color(void) {}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};

class InterpolatedValue
{
public:
	virtual void Read(char *s, int line, int field) = 0;
};

class InterpolatedFloat : public InterpolatedValue
{
public:
	float data[24][NUMWEATHERS];
	float curInterpolator;
	float curVal;

	InterpolatedFloat(float init);
	void Read(char *s, int line, int field);
	float Get(void);
};

class InterpolatedColor : public InterpolatedValue
{
public:
	Color data[24][NUMWEATHERS];
	float curInterpolator;
	Color curVal;

	InterpolatedColor(const Color &init);
	void Read(char *s, int line, int field);
	Color Get(void);
};

class InterpolatedLight : public InterpolatedColor
{
public:
	InterpolatedLight(const Color &init) : InterpolatedColor(init) {}
	void Read(char *s, int line, int field);
};

void neoReadWeatherTimeBlock(FILE *file, InterpolatedValue *interp);

// DISCLAIMER: This is all much too complicated.
// I wanted to copy Vienna's code as much as possible but
// now everything is a mess. Fuck!

class CustomPipe
{
public:
	bool canUse;
	RxPipeline *rwPipeline;
	RxD3D8AllInOneRenderCallBack renderCB;
	RxD3D8AllInOneRenderCallBack originalRenderCB;

	CustomPipe(void);
	void CreateRwPipeline(void);
	void SetRenderCallback(RxD3D8AllInOneRenderCallBack);
	void Attach(RpAtomic *atomic);
	static RpAtomic *setatomicCB(RpAtomic *atomic, void *data);
};

void UploadZero(int loc);
void UploadLightColor(RpLight *light, int loc);
void UploadLightDirection(RpLight *light, int loc);
void UploadLightDirectionInv(RpLight *light, int loc);

void neoGlossPipeInit(void);
void neoRimPipeInit(void);

void hookWaterDrops(void);
void neoInit(void);

class NeoWorldPipe : public CustomPipe
{
	void CreateShaders(void);
	void LoadTweakingTable(void);
public:
	//bool isActive;
	bool modulate2x;
	int setMaterial;
	int setMaterialColor;
	int modulateMaterial;
	int lightingEnabled;
//	int lastColor;
//	float lastAmbient;
//	float lastDiffuse;
	bool usePixelShader;
	void *pixelShader;
	InterpolatedFloat lightmapBlend;

	NeoWorldPipe(void);
	void Attach(RpAtomic *atomic);
	static NeoWorldPipe *Get(void);
	void Init(void);

	void RenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	void RenderObjectSetup(RwUInt32 flags);
	void RenderMeshSetUp(RxD3D8InstanceData *inst);
	void RenderMeshCombinerSetUp(RxD3D8InstanceData *inst, RwUInt32 flags);
	void RenderMeshCombinerTearDown(void);
	void RenderMesh(RxD3D8InstanceData *inst, RwUInt32 flags);
};

class GlossPipe : public CustomPipe
{
	void CreateShaders(void);

	static void RenderGloss(RxD3D8ResEntryHeader *header);
	RwTexture *GetGlossTex(RpMaterial *mat);
public:
	Color specular;
	//bool isActive;
//	uchar b2;
	RwTexDictionary *texdict;
	static void *vertexShader;
	static void *pixelShader;

	GlossPipe(void);
	static GlossPipe *Get(void);
	void Init(void);
	static void RenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	static void ShaderSetup(RwMatrix *world);
};

class NeoCarPipe : public CustomPipe
{
	void CreateShaders(void);
	void LoadTweakingTable(void);

	static void MakeScreenQuad(void);
	static void MakeQuadTexCoords(bool textureSpace);
	static void RenderReflectionScene(void);
public:
	static InterpolatedFloat fresnel;
	static InterpolatedFloat power;
	static InterpolatedLight diffColor;
	static InterpolatedLight specColor;
	static void *vertexShaderPass1;
	static void *vertexShaderPass2;
	// reflection map
	static RwCamera *reflectionCam;
	static RwTexture *reflectionMask;
	static RwTexture *reflectionTex;
	static RwIm2DVertex screenQuad[4];
	static RwImVertexIndex screenindices[6];

	static NeoCarPipe *Get(void);
	void Init(void);
	static void RenderEnvTex(void);
	static void SetupEnvMap(void);
	static void RenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	static void ShaderSetup(RwMatrix *world);
	static void DiffusePass(RxD3D8ResEntryHeader *header);
	static void SpecularPass(RxD3D8ResEntryHeader *header);
};

/*
 * neo water drops
 */

struct VertexTex2
{
	RwReal      x;
	RwReal      y;
	RwReal      z;
	RwReal      rhw;
	RwUInt32    emissiveColor;
	RwReal      u0;
	RwReal      v0;
	RwReal      u1;
	RwReal      v1;
};

class WaterDrop
{
public:
	float x, y, time;		// shorts on xbox (short float?)
	float size, uvsize, ttl;	// "
	uchar r;
	uchar g;
	uchar b;
	uchar alpha;
	bool active;
	bool fades;

	void Fade(void);
};

class WaterDropMoving
{
public:
	WaterDrop *drop;
	float dist;
};

class WaterDrops
{
public:
	enum {
		MAXDROPS = 2000,
		MAXDROPSMOVING = 700
	};

	// Logic

	static float ms_xOff, ms_yOff;	// not quite sure what these are
	static WaterDrop ms_drops[MAXDROPS];
	static int ms_numDrops;
	static WaterDropMoving ms_dropsMoving[MAXDROPSMOVING];
	static int ms_numDropsMoving;

	static bool ms_enabled;
	static bool ms_movingEnabled;

	static float ms_distMoved, ms_vecLen, ms_rainStrength;
	static RwV3d ms_vec;
	static RwV3d ms_lastAt;
	static RwV3d ms_lastPos;
	static RwV3d ms_posDelta;

	static int ms_splashDuration;
	static CPlaceable_III *ms_splashObject;

	static void Process(void);
	static void CalculateMovement(void);
	static void SprayDrops(void);
	static void MoveDrop(WaterDropMoving *moving);
	static void ProcessMoving(void);
	static void Fade(void);

	static WaterDrop *PlaceNew(float x, float y, float size, float time, bool fades, int R, int G, int B);
	static void NewTrace(WaterDropMoving*);
	static void NewDropMoving(WaterDrop*);
	// this has one more argument in VC: ttl, but it's always 2000.0
	static void FillScreenMoving(float amount, bool isBlood);
	static void FillScreen(int n);
	static void Clear(void);
	static void Reset(void);

	static void RegisterSplash(CPlaceable_III *plc, float distance);
	static bool NoRain(void);

	// Rendering

	static RwTexture *ms_maskTex;
	static RwTexture *ms_tex;
//	static RwRaster *ms_maskRaster;
	static RwRaster *ms_raster;
	static int ms_fbWidth, ms_fbHeight;
	static void *ms_vertexBuf;
	static void *ms_indexBuf;
	static VertexTex2 *ms_vertPtr;
	static int ms_numBatchedDrops;
	static int ms_initialised;

	static void InitialiseRender(RwCamera *cam);
	static void AddToRenderList(WaterDrop *drop);
	static void Render(void);
};
