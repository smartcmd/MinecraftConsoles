#include "stdafx.h"
#include <assert.h>
#include "XUI_HelpCredits.h"

SCreditTextItemDef CScene_Credits::gs_aCreditDefs[MAX_CREDIT_STRINGS] = 
{
	{ L"MOJANG",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%s",											IDS_CREDITS_ORIGINALDESIGN,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Markus Persson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%s",											IDS_CREDITS_PMPROD,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Kaplan",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%s",											IDS_CREDITS_RESTOFMOJANG,		NO_TRANSLATED_STRING,eMediumText },																						
	{ L"%s",											IDS_CREDITS_LEADPC,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Jens Bergensten",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_JON_KAGSTROM,		NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_CEO,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Carl Manneh",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_DOF,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Lydia Winters",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_WCW,				NO_TRANSLATED_STRING,eLargeText },
	{ L"Karin Severinsson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_CUSTOMERSUPPORT,	NO_TRANSLATED_STRING,eLargeText },
	{ L"Marc Watson",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																					
	{ L"%s",											IDS_CREDITS_DESPROG,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Aron Nieminen",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																					
	{ L"%s",											IDS_CREDITS_CHIEFARCHITECT,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Frisk",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_CODENINJA,			NO_TRANSLATED_STRING,eLargeText },
	{ L"%s",											IDS_CREDITS_TOBIAS_MOLLSTAM,	NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_OFFICEDJ,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Kristoffer Jelbring",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_DEVELOPER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Leonard Axelsson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_BULLYCOORD,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Jakob Porser",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_ARTDEVELOPER,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Junkboy",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_EXPLODANIM,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Mattis Grahm",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_CONCEPTART,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Henrik Petterson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																																												
	{ L"%s",											IDS_CREDITS_CRUNCHER,			NO_TRANSLATED_STRING,eLargeText },
	{ L"Patrick Geuder",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_MUSICANDSOUNDS,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Daniel Rosenfeld (C418)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line																						
	{ L"4J Studios",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
	{ L"%s",											IDS_CREDITS_PROGRAMMING,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Paddy Burns",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Richard Reavy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Stuart Ross",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"James Vaughan",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Mark Hughes",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"Thomas Kronberg",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																					
	{ L"Ian le Bruce",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Andy West",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Gordon McLean",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"%s",											IDS_CREDITS_ART,				NO_TRANSLATED_STRING,eLargeText },
	{ L"David Keningale",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Pat McGovern",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Alan Redmond",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Julian Laing",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Caitlin Goodale",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Scott Sutherland",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Chris Reeves",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kate Wright",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Michael Hansen",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Kate Flavell",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Donald Robertson",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jamie Keddie",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Thomas Naylor",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Brian Lindsay",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Hannah Watts",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Rebecca O'Neil",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"%s",											IDS_CREDITS_QA,					NO_TRANSLATED_STRING,eLargeText },
	{ L"Steven Gary Woodward",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Richard Black",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"George Vaughan",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"",												NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },	// extra blank line
	{ L"%s",											IDS_CREDITS_SPECIALTHANKS,		NO_TRANSLATED_STRING,eLargeText },
	{ L"Chris van der Kuyl",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Roni Percy",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Anne Clarke",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Anthony Kent",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Microsoft Studios",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eExtraLargeText },
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
	{ L"Josh Mulanax",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Shogo Ishii (TekSystems)",						NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Tyler Keenan (Xtreme Consulting Group Inc)",	NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Joshua Bullard (TekSystems)",					NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
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
	{ L"Brianna Witherspoon (Nytec Inc)",				NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
	{ L"Jim Pekola (Xtreme Consulting Group Inc)",		NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },
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
	{ L"David Hickey",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
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
	{ L"Arkadiusz Kala",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Arkadiusz Sykula",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Bartlomiej Kmita",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Malinowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jan Prejs",										NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
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
	{ L"Przemyslaw Malinowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tomasz Dabrowicz",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Tomasz Trzebiatowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

	{ L"Adam Bogucki",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Aleksander Pietraszak",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Arkadiusz Szczytowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Blazej Kohorewicz",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Damian Mielnik",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Dariusz Nowakowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Dominik Rzeznicki",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jacek Piotrowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Rybacki",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jakub Wozniakowski",							NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jaroslaw Radzio",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Kamil Dabrowski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
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
	{ L"Adam Keating",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Jerzy Tyminski",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Paulina Sliwinska",								NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						
	{ L"Sean Kellogg",									NO_TRANSLATED_STRING,			NO_TRANSLATED_STRING,eSmallText },																						

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

};

// Table tells us how many text elements of each type are available in the scene.
static const int gs_aNumTextElements[ eNumTextTypes ] =
{
	3,		// CScene_Credits::eExtraLargeText
	5,		// CScene_Credits::eLargeText
	5,		// CScene_Credits::eMediumText
	15		// CScene_Credits::eSmallText
};

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Credits::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	int iPad = *static_cast<int *>(pInitData->pvInitData);

	MapChildControls();

	// if we're not in the game, we need to use basescene 0 
	if(Minecraft::GetInstance()->level==nullptr)
	{
		iPad=0;
	}

	ui.SetTooltips( iPad, -1, IDS_TOOLTIPS_BACK);

	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		CREDITS_SCREEN_MIN_Y = 200.0f/1.5f;		// Y pos at which credits are removed from top of screen.
		CREDITS_SCREEN_MAX_Y = 630.0f/1.5f;		// Y pos at which credits appear at bottom of screen.
		CREDITS_FADE_HEIGHT = 100.0f/1.5f;		// Height over which credits fade in or fade out.
		gs_aLineSpace[eExtraLargeText]=53.0f;
		gs_aLineSpace[eLargeText]=46.0f;
		gs_aLineSpace[eMediumText]=40.0f;
		gs_aLineSpace[eSmallText]=21.0f;
	}
	else
	{
		CREDITS_SCREEN_MIN_Y = 200.0f;		// Y pos at which credits are removed from top of screen.
		CREDITS_SCREEN_MAX_Y = 630.0f;		// Y pos at which credits appear at bottom of screen.
		CREDITS_FADE_HEIGHT = 100.0f;		// Height over which credits fade in or fade out.
		gs_aLineSpace[eExtraLargeText]=80.0f;
		gs_aLineSpace[eLargeText]=70.0f;
		gs_aLineSpace[eMediumText]=60.0f;
		gs_aLineSpace[eSmallText]=32.0f;
	}


	// Init text elements.
	int iIDTag  = 1;
	LPWSTR idString = new wchar_t[ 32 ];
	
	for ( int i = 0; i < eNumTextTypes; ++i )
	{
		m_aTextTypes[ i ].m_iNextFreeElement = 0;
		m_aTextTypes[ i ].m_iNumUsedElements = 0;
		m_aTextTypes[ i ].m_iMaxElements = gs_aNumTextElements[ i ];
		m_aTextTypes[ i ].m_appTextElements = new CXuiControl* [ m_aTextTypes[ i ].m_iMaxElements ];

		for ( int j = 0; j < m_aTextTypes[ i ].m_iMaxElements; ++j )
		{
			swprintf( idString, 32, L"XuiText%d", iIDTag );	
			++iIDTag;

			HXUIOBJ text;
			GetChildById( idString, &text );

			VOID* pTextObj;
			XuiObjectFromHandle( text, &pTextObj );
			m_aTextTypes[ i ].m_appTextElements[ j ] = static_cast<CXuiControl *>(pTextObj);
			m_aTextTypes[ i ].m_appTextElements[ j ]->SetShow( false );
		}
	}
	delete [] idString;

	// How many lines of text are in the credits?

	m_iNumTextDefs = MAX_CREDIT_STRINGS;//sizeof( gs_aCreditDefs ) / sizeof( SCreditTextItemDef );

	// Are there any additional lines needed for the DLC credits?
	m_iNumTextDefs+=app.GetDLCCreditsCount();

	m_iCurrDefIndex = -1;
	m_fMoveSinceLastDef = 0.0f;
	m_fMoveToNextDef = 0.0f;

	// Add timer to tick credits update at 60Hz
	HRESULT timerResult = SetTimer( CREDITS_TICK_TIMER_ID, ( 1000 / 60 ) );
	assert( timerResult == S_OK );

	return S_OK;
}

HRESULT CScene_Credits::OnDestroy()
{
	// Free up memory that we allocated.
	for ( int i = 0; i < eNumTextTypes; ++i )
	{
		delete [] m_aTextTypes[ i ].m_appTextElements;
	}

	return S_OK;
}

HRESULT CScene_Credits::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr=S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}


	return hr;
}

HRESULT CScene_Credits::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// ignore any joypads other than the main
	BYTE bFocusUser=XuiElementGetFocusUser(pControlNavigateData->hObjSource);
	// get the user from the control
	/*if(!=ProfileManager.GetLockedProfile())
	{
		bHandled=true;
		return S_OK;
	}*/
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Credits::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);


	return S_OK;
}



HRESULT CScene_Credits::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// Update pointer from stick input on timer.
	if ( pTimer->nId == CREDITS_TICK_TIMER_ID )
    {
		D3DXVECTOR3 vTextPos;

		// Do we need to spawn a new item?
		if ( ( m_iCurrDefIndex == -1 ) || ( m_fMoveSinceLastDef >= m_fMoveToNextDef ) )
		{
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
				// DLC credit
				pDef = app.GetDLCCredits(m_iCurrDefIndex-MAX_CREDIT_STRINGS);
			}
			else
			{
				// Get text def for this item.
				pDef = &( gs_aCreditDefs[ m_iCurrDefIndex ] );
			}

			int iNewTextType = ( int )( pDef->m_eType );
			STextType* pTextType = &( m_aTextTypes[ iNewTextType ] );

			// Are there any text elements available for this item?
			if ( pTextType->m_iNumUsedElements < pTextType->m_iMaxElements )
			{
				// Get the correct text element to use.
				CXuiControl* pElement = pTextType->m_appTextElements[ pTextType->m_iNextFreeElement ];
				
				// Set up the new text element.
				if ( pDef->m_iStringID[0] == NO_TRANSLATED_STRING )
				{
					pElement->SetText( pDef->m_Text );
				}
				else // using additional translated string.
				{
					LPWSTR creditsString = new wchar_t[ 128 ];
					if(pDef->m_iStringID[1]!=NO_TRANSLATED_STRING)
					{
						swprintf( creditsString, 128, pDef->m_Text, app.GetString( pDef->m_iStringID[0] ),  app.GetString( pDef->m_iStringID[1] ) );	
					}
					else
					{
						swprintf( creditsString, 128, pDef->m_Text, app.GetString( pDef->m_iStringID[0] ) );	
					}
					pElement->SetText( creditsString );
					delete [] creditsString;
				}

				pElement->SetShow( true );
				pElement->SetOpacity( 0.0f );

				pElement->GetPosition( &vTextPos );
				vTextPos.y = CREDITS_SCREEN_MAX_Y;
				pElement->SetPosition( &vTextPos );

				// Set next free element of this type.
				++( pTextType->m_iNextFreeElement );

				if ( pTextType->m_iNextFreeElement >= pTextType->m_iMaxElements )
				{
					pTextType->m_iNextFreeElement = 0;
				}

				++pTextType->m_iNumUsedElements;
				
				int iNextDef = m_iCurrDefIndex + 1;
				if ( iNextDef == m_iNumTextDefs )
				{
					// Large space before looping back to start of credits.
					m_fMoveToNextDef = 400.0f;
				}
				else if ( iNextDef < MAX_CREDIT_STRINGS )
				{
					// Determine space to next item.
					m_fMoveToNextDef = gs_aLineSpace[ gs_aCreditDefs[ iNextDef ].m_eType ];
				}
				else
				{
					// Determine space to next item.
					m_fMoveToNextDef = gs_aLineSpace[app.GetDLCCredits(iNextDef-MAX_CREDIT_STRINGS)->m_eType]; 
				}

				m_fMoveSinceLastDef = 0.0f;
			}
		}

		// Scroll up every active text element.
		for ( int i = 0; i < eNumTextTypes; ++i )
		{
			STextType* pTextType = &( m_aTextTypes[ i ] );

			// Process in reverse order from latest one back (so that we can easily remove oldest if it has scrolled off the top).
			int iElementIndex = pTextType->m_iNextFreeElement;

			--iElementIndex;
			if ( iElementIndex < 0 )
			{
				iElementIndex = pTextType->m_iMaxElements - 1;
			}

			// For each element that it is use
			for ( int j = 0; j < pTextType->m_iNumUsedElements; ++j )
			{
				// Get the actual element. 
				CXuiControl* pElement = pTextType->m_appTextElements[ iElementIndex ];

				// Scroll element up.
				pElement->GetPosition( &vTextPos );
				vTextPos.y -= 1.0f;
				pElement->SetPosition( &vTextPos );

				// Is it off the top?
				if ( vTextPos.y < CREDITS_SCREEN_MIN_Y )
				{
					// Remove it.
					pElement->SetShow( false );
					--( pTextType->m_iNumUsedElements );
				}
				else
				{
					// Set transparency to fade in at bottom or out at top.
					float fOpacity = 1.0f;
					if ( vTextPos.y < ( CREDITS_SCREEN_MIN_Y + CREDITS_FADE_HEIGHT ) )
					{
						fOpacity = ( vTextPos.y - CREDITS_SCREEN_MIN_Y ) / CREDITS_FADE_HEIGHT;
					}
					else if  ( vTextPos.y > ( CREDITS_SCREEN_MAX_Y - CREDITS_FADE_HEIGHT ) )
					{
						fOpacity = ( CREDITS_SCREEN_MAX_Y - vTextPos.y ) / CREDITS_FADE_HEIGHT;
					}
					pElement->SetOpacity( fOpacity );
				}

				// Determine next element index.
				--iElementIndex;
				if ( iElementIndex < 0 )
				{
					iElementIndex = pTextType->m_iMaxElements - 1;
				}
			}
		}

		m_fMoveSinceLastDef += 1.0f;

		// This message has been dealt with, don't pass it on further.
		bHandled = TRUE;
	}
	return S_OK;
}