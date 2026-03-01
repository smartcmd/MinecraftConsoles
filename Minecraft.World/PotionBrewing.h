#pragma once
using namespace std;
class MobEffectInstance;

class PotionBrewing
{
public:
	static const int POTION_ID_SPLASH_DAMAGE = 32732;
	static const int POTION_ID_SPLASH_WEAKNESS = 32696;
	static const int POTION_ID_SPLASH_SLOWNESS = 32698;
	static const int POTION_ID_SPLASH_POISON = 32660;
	static const int POTION_ID_HEAL = 16341;
	static const int POTION_ID_SWIFTNESS = 16274;
	static const int POTION_ID_FIRE_RESISTANCE = 16307;

	static const bool SIMPLIFIED_BREWING = true;
	// 4J Stu - Made #define so we can use it to select const initialisation
#define _SIMPLIFIED_BREWING 1

	static const int BREWING_TIME_SECONDS = 20;

	static const int THROWABLE_BIT = 14;
	static const int THROWABLE_MASK = (1 << THROWABLE_BIT);

	static const wstring MOD_WATER;
	static const wstring MOD_SUGAR;
	static const wstring MOD_GHASTTEARS;
	static const wstring MOD_SPIDEREYE;
	static const wstring MOD_FERMENTEDEYE;
	static const wstring MOD_SPECKLEDMELON;
	static const wstring MOD_BLAZEPOWDER;
	static const wstring MOD_MAGMACREAM;
	static const wstring MOD_REDSTONE;
	static const wstring MOD_GLOWSTONE;
	static const wstring MOD_NETHERWART;
	static const wstring MOD_GUNPOWDER;
	static const wstring MOD_GOLDENCARROT;

	static const int BITS_FOR_MAX_NORMAL_EFFECT = 0xF;
	static const int BITS_FOR_DURATION = (1 << 5);
	static const int BITS_FOR_EXTENDED = (1 << 6);
	static const int BITS_FOR_NORMAL = (1 << 13);
	static const int BITS_FOR_SPLASH = (1 << 14);

private:
	typedef unordered_map<int, wstring> intStringMap;
	static intStringMap potionEffectDuration;
	static intStringMap potionEffectAmplifier;
	
public:
	static void staticCtor();

	static const int NUM_BITS = 15;
	
	// 4J Stu - Made public
	static const int BREW_MASK = 0x7fff;
private:
	static const int TOP_BIT = 0x4000;

	static bool isWrappedLit(int brew, int position);

public:
	static bool isLit(int brew, int position);

private:
	static int isBit(int brew, int position);
	static int isNotBit(int brew, int position);

public:
	static int getAppearanceValue(int brew);
	static int getColorValue(vector<MobEffectInstance *> *effects);
	static bool areAllEffectsAmbient(vector<MobEffectInstance *> *effects);

private:
	static unordered_map<int, int> cachedColors;

public:
	static int getColorValue(int brew, bool includeDisabledEffects);
	static int getSmellValue(int brew);

private:
	static const int DEFAULT_APPEARANCES[];

public:
	static int getAppearanceName(int brew);

private:
	static const int NO_COUNT = -1;
	static const int EQUAL_COUNT = 0;
	static const int GREATER_COUNT = 1;
	static const int LESS_COUNT = 2;

	static int constructParsedValue(bool isNot, bool hasMultiplier, bool isNeg, int countCompare, int valuePart, int multiplierPart, int brew);
	static int countOnes(int brew);
	static int parseEffectFormulaValue(const wstring &definition, int start, int end, int brew);

public:
	static vector<MobEffectInstance *> *getEffects(int brew, bool includeDisabledEffects);

#if !(_SIMPLIFIED_BREWING)
	static int boil(int brew);
	static int shake(int brew);
	static int stirr(int brew);
#endif

private:
	static int applyBrewBit(int currentBrew, int bit, bool isNeg, bool isNot, bool isRequired);

public:
	static int applyBrew(int currentBrew, const wstring &formula);
	static int setBit(int brew, int position, bool onOff);
	static int valueOf(int brew, int p1, int p2, int p3, int p4);
	static int valueOf(int brew, int p1, int p2, int p3, int p4, int p5);
	static wstring toString(int brew);
	//static void main(String[] args);
};