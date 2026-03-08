#include "stdafx.h"
#include "UI.h"
#include "../Minecraft.World/StringHelpers.h"
#include "UIScene_Credits.h"

#define CREDIT_ICON -2

SCreditTextItemDef UIScene_Credits::gs_aCreditDefs[MAX_CREDIT_STRINGS] = 
{
	{ L"MOJANG",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_CREDITS_ORIGINALDESIGN,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Markus Persson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_CREDITS_PMPROD,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Kaplan",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_CREDITS_RESTOFMOJANG,		NO_TRANSLATED_STRING,eMediumText },																						
	{ L"%ls",											IDS_CREDITS_LEADPC,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Jens Bergensten",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_JON_KAGSTROM,		NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_CEO,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Carl Manneh",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_DOF,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Lydia Winters",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_WCW,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Karin Severinsson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_CUSTOMERSUPPORT,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Marc Watson",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																					
	{ L"%ls",											IDS_CREDITS_DESPROG,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Aron Nieminen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																					
	{ L"%ls",											IDS_CREDITS_CHIEFARCHITECT,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Frisk",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_CODENINJA,			NO_TRANSLATED_STRING,eLargeText },
	{ L"%ls",											IDS_CREDITS_TOBIAS_MOLLSTAM,	NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_OFFICEDJ,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Kristoffer Jelbring",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_DEVELOPER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Leonard Axelsson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%ls",											IDS_CREDITS_BULLYCOORD,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Jakob Porser",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_ARTDEVELOPER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Junkboy",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_EXPLODANIM,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Mattis Grahm",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%ls",											IDS_CREDITS_CONCEPTART,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Henrik Petterson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																																												
	{ L"%ls",											IDS_CREDITS_CRUNCHER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Patrick Geuder",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%ls",											IDS_CREDITS_MUSICANDSOUNDS,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Rosenfeld (C418)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line	

// Added credit for horses
	{ L"Developers of Mo' Creatures:",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"John Olarte (DrZhark)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kent Christian Jensen",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Dan Roque",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line	


	{ L"4J Studios",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"%ls",											IDS_CREDITS_PROGRAMMING,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Paddy Burns",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Richard Reavy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Stuart Ross",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"James Vaughan",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Mark Hughes",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"Harry Gordon",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"Thomas Kronberg",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	
#ifdef _XBOX
	{ L"Ian le Bruce",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Andy West",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Gordon McLean",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif

#ifdef __PSVITA__
// 4J-PB - Aaron didn't want to be in the credits	{ L"Aaron Puzey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Dawson",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif

	{ L"%ls",											IDS_CREDITS_ART,				NO_TRANSLATED_STRING,eLargeText },
	{ L"David Keningale",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#ifdef _XBOX
	{ L"Pat McGovern",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif
	{ L"Alan Redmond",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#ifdef _XBOX
	{ L"Julian Laing",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },

	{ L"Caitlin Goodale",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Scott Sutherland",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif	
	{ L"Chris Reeves",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kate Wright",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Michael Hansen",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#ifdef _XBOX
	{ L"Kate Flavell",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif
	{ L"Donald Robertson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jamie Keddie",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Thomas Naylor",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Brian Lindsay",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Hannah Watts",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Rebecca O'Neil",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },

	{ L"%ls",											IDS_CREDITS_QA,					NO_TRANSLATED_STRING,eLargeText },
	{ L"Steven Gary Woodward",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#ifdef _XBOX
	{ L"Richard Black",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#endif
	{ L"George Vaughan",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_CREDITS_SPECIALTHANKS,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Chris van der Kuyl",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Roni Percy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Anne Clarke",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Anthony Kent",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
#ifdef _XBOX
 // credits are in the XUI file
#elif defined(__PS3__)
// font credits
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_DYNAFONT,					NO_TRANSLATED_STRING,eLargeText },

#elif defined(__ORBIS__)
// font credits
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_DYNAFONT,					NO_TRANSLATED_STRING,eLargeText },

#elif defined(_DURANGO)
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"Xbox LIVE Arcade Team",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																						
	{ L"%s",											IDS_CREDITS_LEADPRODUCER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Roger Carpenter",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_PRODUCER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Stuart Platt",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Riccardo Lenzi",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_LEADTESTER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Bill Brown (Insight Global)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Brandon McCurry (Insight Global)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Hakim Ronaque, Joe Dunavant",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Paul Loynd, Jeffery Stephens",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Rial Lerum (Xtreme Consulting Group Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_DESIGNTEAM,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Craig Leigh",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_DEVELOPMENTTEAM,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Scott Guest",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jeff \"Dextor\" Blazier",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Yukie Yamaguchi",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jason Hewitt",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_RELEASEMANAGEMENT,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Isaac Aubrey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jordan Forbes",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Josh Mulanax",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Shogo Ishii (TekSystems)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tyler Keenan (Xtreme Consulting Group Inc)",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Joshua Bullard (TekSystems)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	
	{ L"GTO-E Compliance",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Dominic Gara",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"James Small",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },

	{ L"%s",											IDS_CREDITS_EXECPRODUCER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Mark Coates",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Avi Ben-Menahem",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Earnest Yuen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },


	{ L"%s",											IDS_CREDITS_XBLADIRECTOR,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Ted Woolsey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_BIZDEV,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Cherie Lutz",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Peter Zetterberg",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_PORTFOLIODIRECTOR,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Chris Charla",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"%s",											IDS_CREDITS_PRODUCTMANAGER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel McConnell",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"%s",											IDS_CREDITS_MARKETING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Brandon Wells",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Michael Wolf",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"John Dongelmans",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"%s",											IDS_CREDITS_COMMUNITYMANAGER,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Alex Hebert",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_REDMONDLOC,			NO_TRANSLATED_STRING,eLargeText }, 
	{ L"Zeb Wedell",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Gabriella Mittiga (Pactera)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Scott Fielding (Global Studio Consulting)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Yong Zhao (Hisoft Envisage Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Shogo Ishii (Insight Global)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_EUROPELOC,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Gerard Dunne",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Ricardo Cordoba",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Magali Lucchini",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Malika Kherfi",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Lizzy Untermann",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Ian Walsh",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Alfonsina Mossello (Keywords International Ltd)",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Marika Mauri (Keywords International Ltd)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Nobuhiro Izumisawa (Keywords International Ltd)",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Sebastien Faucon (Keywords International Ltd)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Jose Manuel Martinez (Keywords International Ltd)",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Montse Garcia  (Keywords International Ltd)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"%s",											IDS_CREDITS_ASIALOC,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Takashi Sasaki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Changseon Ha",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Shinya Muto (Zip Global Corporation)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Hiroshi Hosoda (Zip Global Corporation)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Natsuko Kudo (Zip Global Corporation)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Yong-Hong Park (Zip Global Corporation)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"Yuko Yoshida (Zip Global Corporation)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText }, 
	{ L"%s",											IDS_CREDITS_USERRESEARCH,		NO_TRANSLATED_STRING,eLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"User Research Lead",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Tim Nichols",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"User Research Engineer",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Michael Medlock",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kristie Fisher",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%s",											IDS_CREDITS_MGSCENTRAL,			NO_TRANSLATED_STRING,eLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"Test Team Lead",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Dan Smith",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_MILESTONEACCEPT,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Justin Davis (VMC)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Microsoft Studios Sentient Development Team",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },																					
	{ L"Ellery Charlson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Frank Klier",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jason Ronald",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Cullen Waters",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Steve Jackson",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Barath Vasudevan",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Derek Mantey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Henry Sterchi",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Scott Fintel",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Soren Hannibal Nielsen",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Meetali Goel (Aditi)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Uladzimir Sadouski (Volt)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_SPECIALTHANKS,		NO_TRANSLATED_STRING,eLargeText },

	{ L"Allan Murphy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Allison Bokone",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Alvin Chen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Arthur Yung",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Brian Tyler",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Daniel Taylor",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Dave Reed",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Duoc Nguyen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Eric Voreis",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Evelyn Thomas",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jeff Braunstein",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jolynn Carpenter",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Justin Brown",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kareem Choudhry",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kevin Cogger",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kevin La Chapelle",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Luc Rancourt",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Matt Bronder",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Michael Siebert",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Mike Harsh",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Mike Sterling",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Nick Rapp",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Orr Keshet",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Paul Hellyar",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Peter Giffin",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Richard Moe",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Scott Selfon",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Stephane St-Michel",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Steve Spiller",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Steven Trombetta",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Theo Michel",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tina Lemire",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tom Miller",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Travis St. Onge",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },

	{ L"Brianna Witherspoon (Nytec Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jim Pekola (Xtreme Consulting Group Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Greg Hjertager",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Masha Reutovski (Nytec Inc)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Henry",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Matt Golz",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Gaffney (Volt)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jared Barnhill (Aditi)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Laura Hawkins",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"2nd Cavalry",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"GTO Bug Bash Team",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Oliver Miyashita",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kevin Salcedo",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Nick Bodenham",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Giggins",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Ben Board",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Peter Choi",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Andy Su (CompuCom Systems Inc.)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"David Boker ",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Josh Bliggenstorfer",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Paul Amer",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Louise Smith",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Karin Behland (Aquent LLC)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"John Bruno",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Phil Spencer",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"John Smith",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Christi Davisson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jacob Farley (Aditi)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chad Stringer (Collabera)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Rick Rispoli (Collabera)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Test by Experis",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },																						
	{ L"%s",											IDS_CREDITS_TESTMANAGER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Matt Brown",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Gavin Kennedy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_SRTESTLEAD,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Lloyd Bell",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tim Attuquayefio",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_TESTLEAD,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Byron R. Monzon",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Marta Alombro",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_SDET,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Valeriy Novytskyy",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_PROJECT,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Allyson Burk",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"David Scott",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"John Shearer",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_ADDITIONALSTE,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Chris Merritt",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kimberlee Lyles",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Eric Ranz",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Russ Allen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_TESTASSOCIATES,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Michael Arvat",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Josh Breese",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"April Culberson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jason Fox",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Clayton K. Hopper",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Matthew Howells",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Alan Hume",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jacob Martin",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kevin Lourigan",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tyler Lovemark",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_RISE_LUGO,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Ryan Naegeli",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Isaac Price",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Masha Reutovski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Brad Shockey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jonathan Tote",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Marc Williams",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Gillian Williams",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jeffrey Woito",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tyler Young",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jae Yslas",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Amanda Swalling",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Ben Dienes",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Kent",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Dustin Lukas",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Emily Lovering",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Nick Fowler",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	// EVEN MORE CREDITS
	{ L"Test by Lionbridge",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																						
	{ L"%s",											IDS_CREDITS_TESTMANAGER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Blazej Zawadzki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_TESTLEAD,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Jakub Garwacki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Kamil Lahti",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Mariusz Gelnicki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Karol Falak",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Lukasz Watroba",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"%s",											IDS_CREDITS_PROJECT,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Artur Grochowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Grzegorz Kohorewicz",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Lukasz Derewonko",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Celej",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"Senior Test Engineers",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Jakub Rybacki",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Mateusz Szymanski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Arkadiusz Szczytowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Rafal Rawski",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"%s",											IDS_CREDITS_TESTASSOCIATES,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Adrian Klepacki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Aleksander Pietraszak",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"Arkadiusz Kala",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Arkadiusz Sykula",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Bartlomiej Kmita",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Malinowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jan Prejs",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jedrzej Kucharek",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Kamil Dabrowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Maciej Urlo",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Maciej Wygoda",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Marcin Piasecki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Marcin Piotrowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Marek Latacz",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Biernat",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Krupinski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Warchal",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Wascinski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Zbrzezniak",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Milosz Maciejewicz",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Pawel Kumanowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Przemyslaw Malinowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tomasz Dabrowicz",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tomasz Trzebiatowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Wojciech Kujawa",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"Blazej Kohorewicz",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Damian Mielnik",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Dariusz Nowakowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Dominik Rzeznicki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jacek Piotrowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Rybacki",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Wozniakowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jaroslaw Radzio",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Kamil Kaczor",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Karolina Szymanska",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Konrad Mady",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Krzysztof Galazka",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Ludwik Miszta",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Lukasz Kwiatkowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Marcin Krzysiak",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Mateusz Szymanski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Maslany",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michal Nyszka",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Norbert Jankowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Piotr Daszewski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Radoslaw Kozlowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tomasz Kalowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_SPECIALTHANKS,		NO_TRANSLATED_STRING,eLargeText },
	{ L"David Hickey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Sean Kellogg",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Adam Keating",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jerzy Tyminski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Paulina Sliwinska",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	
	{ L"Test by Shield",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																						
	{ L"GTO Shared Service Test Manager",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Natahri Felton",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Shield Test Lead",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Matt Giddings",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Shield IT Support",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"David Grant (Compucom Systems Inc)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Primary Team",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Alex Chen (CompuCom Systems Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Alex Hunte (CompuCom Systems Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Brian Boye (CompuCom Systems Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Bridgette Cummins (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Chris Carleson (Volt)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Christopher Hermey (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"David Hendrickson (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Ioana Preda (CompuCom Systems Inc)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jessica Jenkins (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Johnathan Ochs (CompuCom Systems Inc)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Michael Upham (CompuCom Systems Inc)",			NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Nicholas Johansson (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Nicholas Starner (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Torr Vickers (Volt)",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Victoria Bruder (CompuCom Systems Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																							

#elif defined(_WIN64)
#elif defined(__PSVITA__)
// font credits
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%ls",											IDS_DYNAFONT,					NO_TRANSLATED_STRING,eLargeText },

#endif

#ifndef _XBOX
// Miles & Iggy credits
	{ L"",													NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"",													CREDIT_ICON,					eCreditIcon_Iggy,eSmallText },	// extra blank line
	{ L"Uses Iggy.",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#ifdef __PS3__
	{ L"Copyright (C) 2009-2013 by RAD Game Tools, Inc.",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#else
	{ L"Copyright (C) 2009-2014 by RAD Game Tools, Inc.",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#endif
	{ L"",													NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"",													CREDIT_ICON,					eCreditIcon_Miles,eSmallText },	// extra blank line
	{ L"Uses Miles Sound System.",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#ifdef __PS3__
	{ L"Copyright (C) 1991-2013 by RAD Game Tools, Inc.",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#else
	{ L"Copyright (C) 1991-2014 by RAD Game Tools, Inc.",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#endif
#ifdef __PS3__
	{ L"",													NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"",													CREDIT_ICON,					eCreditIcon_Dolby,eSmallText },	// extra blank line
	{ L"Dolby and the double-D symbol",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"are trademarks of Dolby Laboratories.",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
#endif
#endif
};

UIScene_Credits::UIScene_Credits(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bAddNextLabel = false;

	// How many lines of text are in the credits?
	m_iNumTextDefs = MAX_CREDIT_STRINGS;

	// Are there any additional lines needed for the DLC credits?
	m_iNumTextDefs+=app.GetDLCCreditsCount();

	m_iCurrDefIndex = -1;

	// Add the first 20 Flash can cope with
	for(unsigned int i = 0; i < 20; ++i)
	{
		++m_iCurrDefIndex;

		// Set up the new text element.
		if ( gs_aCreditDefs[i].m_iStringID[0] == NO_TRANSLATED_STRING )
		{
			setNextLabel(gs_aCreditDefs[i].m_Text,gs_aCreditDefs[i].m_eType);
		}
		else // using additional translated string.
		{
			LPWSTR creditsString = new wchar_t[ 128 ];
			if(gs_aCreditDefs[i].m_iStringID[1]!=NO_TRANSLATED_STRING)
			{
				swprintf( creditsString, 128, gs_aCreditDefs[i].m_Text, app.GetString( gs_aCreditDefs[i].m_iStringID[0] ),  app.GetString( gs_aCreditDefs[i].m_iStringID[1] ) );	
			}
			else
			{
				swprintf( creditsString, 128, gs_aCreditDefs[i].m_Text, app.GetString( gs_aCreditDefs[i].m_iStringID[0] ) );	
			}
			setNextLabel(creditsString,gs_aCreditDefs[i].m_eType);
			delete [] creditsString;
		}
	}
}

wstring UIScene_Credits::getMoviePath()
{
	return L"Credits";
}

void UIScene_Credits::updateTooltips()
{
	ui.SetTooltips( m_iPad, -1, IDS_TOOLTIPS_BACK);
}

void UIScene_Credits::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
}

void UIScene_Credits::handleReload()
{
	// We don't allow this in splitscreen, so just go back
	navigateBack();
}

void UIScene_Credits::tick()
{
	UIScene::tick();

	if(m_bAddNextLabel)
	{
		m_bAddNextLabel = false;

		const SCreditTextItemDef* pDef;

		// Time to create next text item.
		++m_iCurrDefIndex;

		// Wrap back to start.
		if ( m_iCurrDefIndex >= m_iNumTextDefs )
		{
			m_iCurrDefIndex = 0;
		}

		if(m_iCurrDefIndex >= MAX_CREDIT_STRINGS)
		{
			app.DebugPrintf("DLC credit %d\n",m_iCurrDefIndex-MAX_CREDIT_STRINGS);
			// DLC credit
			pDef = app.GetDLCCredits(m_iCurrDefIndex-MAX_CREDIT_STRINGS);
		}
		else
		{
			// Get text def for this item.
			pDef = &( gs_aCreditDefs[ m_iCurrDefIndex ] );
		}

		// Set up the new text element.
		if(pDef->m_Text!=NULL) // 4J-PB - think the RAD logo ones aren't set up yet and are coming is as null
		{
			if ( pDef->m_iStringID[0] == CREDIT_ICON )
			{
				addImage((ECreditIcons)pDef->m_iStringID[1]);
			}			
			else // using additional translated string.
			{
				wstring sanitisedString = wstring(pDef->m_Text);

				// 4J-JEV: Some DLC credits contain copyright or registered symbols that are not rendered in some fonts.
				if ( !ui.UsingBitmapFont() )
				{
					sanitisedString = replaceAll(sanitisedString, L"\u00A9", L"(C)");
					sanitisedString = replaceAll(sanitisedString, L"\u00AE", L"(R)");
					sanitisedString = replaceAll(sanitisedString, L"\u2013", L"-");
				}

				LPWSTR creditsString = new wchar_t[ 128 ];
				if (pDef->m_iStringID[0]==NO_TRANSLATED_STRING)
				{
					ZeroMemory(creditsString, 128);
					memcpy( creditsString, sanitisedString.c_str(), sizeof(WCHAR) * sanitisedString.length() );
				}
				else if(pDef->m_iStringID[1]!=NO_TRANSLATED_STRING)
				{
					swprintf( creditsString, 128, sanitisedString.c_str(), app.GetString( pDef->m_iStringID[0] ),  app.GetString( pDef->m_iStringID[1] ) );	
				}
				else
				{
					swprintf( creditsString, 128, sanitisedString.c_str(), app.GetString( pDef->m_iStringID[0] ) );	
				}

				setNextLabel(creditsString,pDef->m_eType);
				delete [] creditsString;
			}
		}
	}
}

void UIScene_Credits::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %ls, pressed- %ls, released- %ls\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed && !repeat)
		{
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_Credits::setNextLabel(const wstring &label, ECreditTextTypes size)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (int)size;

	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = (m_iCurrDefIndex == (m_iNumTextDefs - 1));

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetNextLabel , 3 , value );
}

void UIScene_Credits::addImage(ECreditIcons icon)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (int)icon;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = (m_iCurrDefIndex == (m_iNumTextDefs - 1));

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcAddImage , 2 , value );	
}

void UIScene_Credits::handleRequestMoreData(F64 startIndex, bool up)
{
	m_bAddNextLabel = true;
}
