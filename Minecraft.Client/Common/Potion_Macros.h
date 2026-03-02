#pragma once

// 4J-JEV:
// All functional potions need bit-13 set.

#define MASK_REGENERATION		0x2001
#define MASK_SPEED				0x2002
#define MASK_FIRE_RESISTANCE	0x2003
#define MASK_POISON				0x2004
#define MASK_INSTANTHEALTH		0x2005
#define MASK_NIGHTVISION		0x2006
#define MASK_INVISIBILITY		0x200E
#define MASK_WEAKNESS			0x2008
#define MASK_STRENGTH			0x2009
#define MASK_SLOWNESS			0x200A
#define MASK_INSTANTDAMAGE		0x200C

#define MASK_TYPE_AWKWARD		0x0010

#define MASK_SPLASH				0x4000
#define MASK_BIT13				0x2000

#define MASK_LEVEL2				0x0020
#define MASK_EXTENDED			0x0040
#define MASK_LEVEL2EXTENDED		0x0060

#define MACRO_POTION_IS_REGENERATION(aux)		((aux & 0x200F)									== MASK_REGENERATION)
#define MACRO_POTION_IS_SPEED(aux)				((aux & 0x200F)									== MASK_SPEED)
#define MACRO_POTION_IS_FIRE_RESISTANCE(aux)	((aux & 0x200F)									== MASK_FIRE_RESISTANCE)
#define MACRO_POTION_IS_INSTANTHEALTH(aux)		((aux & 0x200F)									== MASK_INSTANTHEALTH)
#define MACRO_POTION_IS_NIGHTVISION(aux)		((aux & 0x200F)									== MASK_NIGHTVISION)
#define MACRO_POTION_IS_INVISIBILITY(aux)		((aux & 0x200F)									== MASK_INVISIBILITY)
#define MACRO_POTION_IS_WEAKNESS(aux)			((aux & 0x200F)									== MASK_WEAKNESS)
#define MACRO_POTION_IS_STRENGTH(aux)			((aux & 0x200F)									== MASK_STRENGTH)
#define MACRO_POTION_IS_SLOWNESS(aux)			((aux & 0x200F)									== MASK_SLOWNESS)
#define MACRO_POTION_IS_POISON(aux)				((aux & 0x200F)									== MASK_POISON)
#define MACRO_POTION_IS_INSTANTDAMAGE(aux)		((aux & 0x200F)									== MASK_INSTANTDAMAGE)
#define MACRO_POTION_IS_NIGHTVISION(aux)		((aux & 0x200F)									== MASK_NIGHTVISION)
#define MACRO_POTION_IS_INVISIBILITY(aux)		((aux & 0x200F)									== MASK_INVISIBILITY)

#define MACRO_POTION_IS_SPLASH(aux)				((aux & MASK_SPLASH)							== MASK_SPLASH)
#define MACRO_POTION_IS_BOTTLE(aux)				((aux & MASK_SPLASH)							== 0)

#define MACRO_POTION_IS_AKWARD(aux)				((aux & MASK_TYPE_AWKWARD)						== MASK_TYPE_AWKWARD)

#define MACRO_POTION_IS_REGULAR(aux)			((aux & (MASK_LEVEL2EXTENDED))					== 0)
#define MACRO_POTION_IS_LEVEL2(aux)				((aux & (MASK_LEVEL2 ))							== MASK_LEVEL2)
#define MACRO_POTION_IS_EXTENDED(aux)			((aux & (MASK_EXTENDED))						== (MASK_EXTENDED))
#define MACRO_POTION_IS_LEVEL2EXTENDED(aux)		((aux & (MASK_LEVEL2EXTENDED))					== (MASK_LEVEL2EXTENDED))


#define MACRO_MAKEPOTION_AUXVAL(potion_type, potion_strength, potion_effect)	(potion_type | potion_strength | potion_effect)

// The potion brewing creates high aux values with redundant high bits, so use this to bring the aux val into ranges that match our macros
// 4J-JEV: 0x2000 == bit-13; Used to stop netherwart "resetting" functional potions.
#define NORMALISE_POTION_AUXVAL(aux) (aux & (MASK_BIT13 | MASK_SPLASH | 0xFF))