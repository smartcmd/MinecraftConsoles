/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client UGC Types
//
// Include this to get access to all Sentient UGC Types.

#pragma once

#include "SenClientTypes.h"


namespace Sentient
{
	//****** UGC Creation
	typedef int SenUGCFeedID;

	enum SenUGCPublishState
	{
		SenUGCPublishState_Deleted						=	0x00,
		SenUGCPublishState_Banned						=	0x01,
		SenUGCPublishState_Unpublished					=	0x10,
		SenUGCPublishState_Published					=	0x30,
		SenUGCPublishState_PendingModeratorReview		=   0x31
	};

	enum SenUGCType
	{
		SenUGCType_Movie					=	0,
		SenUGCType_Audio,
		SenUGCType_Image,
		SenUGCType_FirstCustom				=	0x20
	};

	enum SenUGCDescriptor
	{
		SenUGCDescriptor_Illegal			=	0xffff,
		SenUGCDescriptor_None				=	0x0000,
		SenUGCDescriptor_FirstCustom		=	0x0100,
	};

	typedef unsigned int SenUGCMetaDataFlags;
	enum 
	{
		SenUGCMetaDataFlags_NoFlags			=	0x00000000,
		SenUGCMetaDataFlags_FriendsCanEdit	=	0x00000001,
		SenUGCMetaDataFlags_EveryoneCanEdit	=	0x00000002,
		SenUGCMetaDataFlags_Reserved1		=	0x00000004,
		SenUGCMetaDataFlags_Reserved2		=	0x00000008,
		SenUGCMetaDataFlags_Reserved3		=	0x00000010,
		SenUGCMetaDataFlags_Reserved4		=	0x00000020,
		SenUGCMetaDataFlags_Reserved5		=	0x00000040,
		SenUGCMetaDataFlags_Reserved6		=	0x00000080,
		SenUGCMetaDataFlags_FirstCustom		=	0X00000100
	};

	typedef ULONGLONG SenLeaderboardId;

	struct SenUGCMetaData
	{
		SenUGCMetaData()
		{
#pragma warning ( disable : 4996 )	// @TODO - Removed once Int16 Descriptors are deprecated
			memset(descriptors, 0, sizeof(SenUGCDescriptor) * NrUgcDescriptors);
#pragma warning ( default : 4996 )
			memset(descriptors2, 0, sizeof(__int64) * NrUgcDescriptors);
		}

		SenUGCID			parentID;

		SenUGCType			type;
		SenUGCMetaDataFlags flags;

		static const int	NameLength					=	64;
		wchar_t				name[NameLength];
		static const int	ShortNameLength				=	12;
		wchar_t				shortName[ShortNameLength];

		static const int	NrUgcDescriptors			=	4;
		
		__declspec(deprecated("Descriptors have increased in size (from Int16 to Int64).  Please Use descriptors2 instead"))
		SenUGCDescriptor	descriptors[NrUgcDescriptors];
		__int64				descriptors2[NrUgcDescriptors];

		static const int	BlobSizeLimit				=	1024;
		size_t				metaDataBlobSize;
		void*				metaDataBlob;
	};

	const int SenUGCMainData_NrBlobs					=	4;

	struct SenUGCSearchResult
	{
		PlayerUID				author;
		SenUGCID			ID;
		int					revision;
	};

#define SenUGCDownloadedMetaData_NrCustomCounters 4

	// This structure contains the original uploaded metadata plus any stats gathered from the UGC server
	struct SenUGCDownloadedMetaData
	{
		PlayerUID				author;
		SenUGCMetaData		base;
		SenUGCID			ID;
		SYSTEMTIME			authorTime;
		int					revision;

		SenUGCPublishState	publishState;

		int					averageReviewScore;
		int					nrReviews;
		int					nrFavoriteFlags;
		INT64				customCounters[SenUGCDownloadedMetaData_NrCustomCounters];
		int					nrDownloads;	//automatically counts how many times MainDataBlob 0 has been downloaded
		size_t				mainDataSizes[SenUGCMainData_NrBlobs];
		SenLeaderboardId	leaderboardId;
	};

	struct SenUGCBlobMetaData
	{
		size_t Size;
		UINT Version;
	};

	struct SenUGCStatsMetaData
	{
		UINT AverageReviewScore;
		UINT NumberOfReviews;
		UINT NumberOfFavorites; // number of times tagged as a favorite item
		UINT NumDownloads;      // how many times any blob has been downloaded
		SenLeaderboardId LeaderboardId;
		INT64 CustomCounters[SenUGCDownloadedMetaData_NrCustomCounters];
	};

// Matches with documentation and internal server-side value
#define MainDataBlobSizeLimit 16777216

	// Version specifically to support Resubmission feature
	struct SenUGCDownloadedMetaData2
	{
		SenUGCID UgcId;
		PlayerUID AuthorXuid;
		SenUGCMetaData MetaData;

		SYSTEMTIME CreationDate; // first time it's created
		SYSTEMTIME ModifiedDate; // any modification, metadata included
		SYSTEMTIME PublishedDate; // anytime it's published/republished

		SenUGCPublishState PublishState;
		UINT Revision;

		SenUGCBlobMetaData BlobInfo[SenUGCMainData_NrBlobs]; // size and version of each blob, 0's if not present
		SenUGCStatsMetaData StatsInfo;   // stats of ugc item: counters, aggregates, etc
	};

	enum SenUGCSortBy
	{
		SenUGCSortBy_CreationDate,			
		SenUGCSortBy_TopFavorites,			
		SenUGCSortBy_TopReviewScoreAverage, 
		SenUGCSortBy_TopDownloadsTotal		
	};

	enum SenUGCAuthorType
	{
		SenUGCAuthorType_Everyone,
		SenUGCAuthorType_LocalUsers,
		SenUGCAuthorType_Friends,
		SenUGCAuthorType_List
	};

	enum SenUGCOffensivenessFlag
	{
		SenUGCOffensivenessFlag_None					= 0x00000000,

		SenUGCOffensivenessFlag_Discriminating			= 0x00000001,
		SenUGCOffensivenessFlag_Explicit				= 0x00000002,
		SenUGCOffensivenessFlag_Obscene					= 0x00000004,
		SenUGCOffensivenessFlag_FoulLanguage			= 0x00000008,
		SenUGCOffensivenessFlag_LicenseInfringement		= 0x00000010,
		SenUGCOffensivenessFlag_IllegalAdvertising		= 0x00000020,
		SenUGCOffensivenessFlag_Illegal					= 0x00000040,
	};

	struct SenUGCFeedItem
	{
		PlayerUID Author;
		SenUGCID UgcId;
		SYSTEMTIME DateCreated;
	};

	enum SenUGCFeedType
	{
		SenUGCFeedType_Unknown     = 0x00,
		SenUGCFeedType_NewestItems = 0x10,
		SenUGCFeedType_TopReviewed = 0x20,
		SenUGCFeedType_TopDownloaded = 0x30,
		SenUGCFeedType_PendingModeratorReview = 0x60,
		SenUGCFeedType_Custom		= 0x90
	};

	enum SenUGCReviewScoreType
	{
		SenUGCReviewScoreType_Quality	 = 0x00,
		SenUGCReviewScoreType_Difficulty = 0x10
	};

	// ***** Leaderboard types
	typedef __int64 SenLeaderboardEntryValue;
	typedef ULONGLONG SenLeaderboardActorId;

	enum SenLeaderboardSortType
	{
		SenLeaderboardSortType_Ascending = 0x00,
		SenLeaderboardSortType_Descending = 0x10
	};


	/// @brief          Leaderboard Metadata retrieval flag.
	///
	/// @details        When retrieving Leaderboard information, a client can choose
	///					to bypass retrieval or retrieve specific details regarding the 
	///					Metadata stored for a given Leaderboard or Leaderboard Entry.
	///
	enum SenLeaderboardMetadataFlag
	{
		// Bypass the retrieval of metadata blob size and binary data
		SenLeaderboardmetadataFlag_BypassRetrieval	=	0x00000000,
		// Retrieve and populate the size of each metadata blob
		SenLeaderboardmetadataFlag_RetrieveSizeOnly	=	0x00000001,
		// Retrieve and populate both the size and metadata blob binary data
		SenLeaderboardmetadataFlag_RetrieveData		=	0x00000002,
	};

	struct SenLeaderboardDefinition
	{
		static const int		NameLength = 50;

		WCHAR					Name[NameLength];
		SenLeaderboardSortType	SortType;
	};

	struct SenLeaderboardEntry
	{
		SenLeaderboardActorId ActorId;
		SenLeaderboardEntryValue Value;
		LONGLONG Descriptor;
		unsigned int Rank;
		size_t metaDataBlobSize;
		size_t reserved_metaDataBlobCount;
		void *metaDataBlob;
	};

} // namespace Sentient