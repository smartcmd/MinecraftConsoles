#pragma once
using namespace std;

class AABB;
class Recipy;
class Object;

typedef std::vector<AABB*> AABBList;
typedef std::vector<Recipy *> RecipyList;
typedef std::vector<Object *> ObjectList;

#define MAX_PATH_SIZE 256
#define PI (3.141592654f)
#define HALF_PI (1.570796327f)

enum ByteOrder
{
	BIGENDIAN,
	LITTLEENDIAN,

#if defined(__PS3__) || defined(_XBOX)
	LOCALSYTEM_ENDIAN = BIGENDIAN,
#else
	LOCALSYTEM_ENDIAN = LITTLEENDIAN,
#endif
};
enum EDefaultSkins
{
	eDefaultSkins_ServerSelected,
	eDefaultSkins_Skin0,
	eDefaultSkins_Skin1,
	eDefaultSkins_Skin2,
	eDefaultSkins_Skin3,
	eDefaultSkins_Skin4,
	eDefaultSkins_Skin5,
	eDefaultSkins_Skin6,
	eDefaultSkins_Skin7,
	eDefaultSkins_Skin8,
	eDefaultSkins_Skin9,
	eDefaultSkins_Skin10,
	eDefaultSkins_Skin11,
	eDefaultSkins_Skin12,
	eDefaultSkins_Skin13,
	eDefaultSkins_Skin14,
	eDefaultSkins_Skin15,
	eDefaultSkins_Skin16,
	eDefaultSkins_Skin17,

	eDefaultSkins_Count,
};