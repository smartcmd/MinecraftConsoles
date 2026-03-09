/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client UGC API
//
// Include this to get access to all UGC related Sentient features

#pragma once

// Local headers
#include "SenClientMarkers.h"
#include "SenClientUGCLeaderboards.h"
#include "SenClientUGCTypes.h"

// Sentient headers
#include "SenClientSys.h"
#include "SenClientCulture.h"

namespace Sentient
{
	/**********************************
	 ***** UGC Creation Functions *****
	 **********************************/
	
	/// @brief			Generate a unique ID that will be used to
	///					identify a given instance of UGC.  This ID
	///					will is referenced by every other UGC function.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[out]		outResult
	///					The unique ID that has been generated and provisioned 
	///					for an instance of UGC.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			TBD
	///
	/// @details		All UGC functions require a uniquely provisioned UGC ID.
	///
	/// @related		All UGC related functions.
	HRESULT SenUGCCreatePublishingUGCID(
		int userIndex,
		SenUGCID *outResult,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	///
	/// @brief Async output information for SenUGCUpload or Download callers
	/// 
	/// @details Contains progress or retry information in addition to a cancellation token
	/// 
    struct SenUGCProgressInfo 
    {
        SenHandle out_taskHandle;  /// token for canceling the upload process
        INT8 percentageComplete;   /// 1-100, how much percent is complete of upload process for blob
        size_t bytesCompleted;     /// how many bytes have been successfully transmitted for the task
		HRESULT lastStepResult;    /// sentient client SDK HRESULT value
		int numRetries;			   /// does not reset between internal steps for a long-running task
    };

	//************************************
	// Method:    SenUGCUpload
	// FullName:  Sentient::SenUGCUpload
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: const SenUGCMetaData * metaData
	// Parameter: int nrMainDataBlobs
	// Parameter: const void * * mainDataBlobs
	// Parameter: const size_t * mainDataBlobSizes
	// Parameter: bool pushToFeed
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Upload the metadata, as well as one or more binary data blobs to the server.
	// There are multiple data blobs supported (the exact number is defined in 
	//   SenUGCMainData_NrBlobs). One use would be that a game may want store a 
	//   preview thumbnail that can be downloaded without having to download the 
	//   rest of the UGC. This could save bandwidth and make the game more responsive.
	// Note: data blob 0 should be the main level data blob, for the automatic 
	//   download counter to work.
	//   The metadata will also have a data blob associated, but this should be 
	//   kept to a minimum, as UGC download menus will probably want to download
	//   metadata for a lot of UGCs at once.
	// Note: if a level has been uploaded with main data before, and the creator
	//   wants to just modify the metadata, they can upload the metadata with the
	//   maindatablobs being nullptr.
    // NOTE: for large items, use the SenUGCUploadMainData method with the SenUGCProgressInfo
    //   signature so you can get the running progress and a cancellation token 
    //   to abort the upload (allowing UI for the user, etc)
    //************************************
	HRESULT SenUGCUpload(
		int userIndex, 
		SenUGCID ugcID, 
		const SenUGCMetaData *metaData, 
		int nrMainDataBlobs, 
		const void **mainDataBlobs, 
		const size_t *mainDataBlobSizes, 
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );

	//************************************
	// Method:    SenUGCUploadMetadata
	// FullName:  Sentient::SenUGCUploadMetadata
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: const SenUGCMetaData * metaData
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Upload the metadata and one binary data blob to the server.
	// NOTE: data blob at index 0 should be the main level data blob, for the automatic 
	// download counter to work.
	// The metadata will also have a data blob associated, but this should be 
	// kept to a minimum, as UGC download menus will probably want to download
	// metadata for a lot of UGCs at once.
    // NOTE: If a creator uploads metadata again, it will overwrite the previous
    // stored blob with the new one.
	//************************************
	HRESULT SenUGCUploadMetadata(
		int userIndex, 
		SenUGCID ugcID, 
		const SenUGCMetaData* metaData, 
		SenSysCompletedCallback userCallback, 
		void* userCallbackData);

	//************************************
	// Method:    SenUGCUploadMainData
	// FullName:  Sentient::SenUGCUploadMainData
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: int mainDataBlobIndex
	// Parameter: const void * mainDataBlob
	// Parameter: const size_t mainDataBlobSize
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Parameter: SenUGCProcessInfo* out_ugcUploadInfo
	// Upload one binary data blob to the server.
    // This SenUGCUpload method with the SenUGCProcessInfo signature is so you 
    //   can get the progress percentage and a cancellation token, which can 
    //   be used to abort the upload. This is useful for large uploads where 
    //   you may want to allow the user to cancel.
    // NOTE: This call is asynchronous ONLY and will error for synchronous 
    //   attempts with a nullptr param for userCallback.
    // There are multiple data blobs supported (the exact number is defined in 
	//   SenUGCMainData_NrBlobs) on subsequent calls. Slot zero is to be used by a 
    //   game to store a preview thumbnail, which can then be downloaded without 
    //   having to download the rest of the UGC. This could save bandwidth and 
    //   make the game more responsive.
	// NOTE: data blob at index 0 should be the main level data blob, for the automatic 
	//   download counter to work.
	//   The metadata will also have a data blob associated, but this should be 
	//   kept to a minimum, as UGC download menus will probably want to download
	//   metadata for a lot of UGCs at once.
	// NOTE: if a level has been uploaded with main data before, and the creator
	//   wants to just modify the metadata, they can upload the metadata with the
	//   main data blob being nullptr. 
    // NOTE: If a creator uploads a data blob again, it will overwrite the previous
    //   stored blob with the new one.
	//************************************
	HRESULT SenUGCUploadMainDataBlob(
		int userIndex, 
		SenUGCID ugcID, 
		int mainDataBlobIndex, 
		const void* mainDataBlob, 
		const size_t mainDataBlobSize, 
		SenSysCompletedCallback userCallback, 
		void* userCallbackData,
		SenUGCProgressInfo* out_progressInfo);

	//************************************
	// Method:    SenUGCDelete
	// FullName:  Sentient::SenUGCDelete
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Delete the UGC - only the user that created the UGC can delete it.
	//************************************
	HRESULT SenUGCDelete(
		int userIndex,
		SenUGCID ugcID,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );

	/*************************************
	 ***** UGC Consumption Functions *****
	 *************************************/

	//************************************
	// Method:    SenUGCEnumerate
	// FullName:  Sentient::SenUGCEnumerate
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: SenUGCSortBy sortBy
	// Parameter: SenUGCType type
	// Parameter: SenUGCAuthorType authorType
	// Parameter: int nrAuthors
	// Parameter: const XUID * authorList
	// Parameter: SenUGCMetaDataFlags metaDataFlagFilter
	// Parameter: SenUGCPublishState minPublishStateFilter
	// Parameter: SenUGCPublishState maxPublishStateFilter
	// Parameter: SenSysDateTime newerThan
	// Parameter: SenUGCDescriptor descriptor
	// Parameter: int maxNrResults
	// Parameter: SenUGCSearchResult * outBuffer
	// Parameter: unsigned int * outNrResults
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Search the database for all the UGCs that match various search criteria.
	// outBuffer should be an preallocated array of [sizeof(SenUGCSearchResult)*maxNrResults] bytes.
	//************************************
	HRESULT SenUGCEnumerate( 
		int userIndex,
		SenSysTitleID titleID,
		SenUGCSortBy sortBy,
		SenUGCType type,
		SenUGCAuthorType authorType,
		int nrAuthors,
		const PlayerUID *authorList,
		SenUGCMetaDataFlags metaDataFlagFilter,
		SYSTEMTIME *newerThan,
		SenUGCDescriptor descriptor,
		int maxNrResults,
		SenUGCSearchResult *outBuffer,
		UINT *outNrResults,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );


	/// @brief			public 
	///
	/// @param[in]		userIndex
	/// @param[in]		titleID
	/// @param[in]		sortBy
	/// @param[in]		type
	/// @param[in]		authorType
	/// @param[in]		nrAuthors
	/// @param[in]		authorList
	/// @param[in]		metaDataFlagFilter
	/// @param[in]		newerThan
	/// @param[in]		nrDescriptors
	/// @param[in]		descriptors
	/// @param[in]		maxNrResults
	/// @param[out]		outBuffer
	/// @param[out]		outNrResults
	/// @param[in]		userCallback
	/// @param[in]		userCallbackData
	///
	/// @return			Search the database for all the UGCs that match various search criteria.
	///					outBuffer should be an preallocated array of [sizeof(SenUGCSearchResult)*maxNrResults] bytes.
	///
	/// @details		Enumerate by name will perform a look based on a various search criteria.  The Collection 
	///					of descriptors will perform an equality lookup on Descriptor
	///
	/// @related		SenUGCEnumerate
	HRESULT SenUGCEnumerate( 
		int userIndex,
		SenSysTitleID titleID,
		SenUGCSortBy sortBy,
		SenUGCType type,
		SenUGCAuthorType authorType,
		int nrAuthors,
		const PlayerUID *authorList,
		SenUGCMetaDataFlags metaDataFlagFilter,
		SYSTEMTIME *newerThan,
		int nrDescriptors,
		INT64 *descriptors,
		int maxNrResults,
		SenUGCSearchResult *outBuffer,
		UINT *outNrResults,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );

	/// @brief			public 
	///
	/// @param[in]	userIndex
	/// @param[in]	titleID
	/// @param[in]	sortBy
	/// @param[in]	type
	/// @param[in]	authorType
	/// @param[in]	nrAuthors
	/// @param[in]	authorList
	/// @param[in]	metaDataFlagFilter
	/// @param[in]	newerThan
	/// @param[in]	nrDescriptors
	/// @param[in]	descriptors
	/// @param[in]	maxNrResults
	/// @param[out]	outBuffer
	/// @param[out]	outNrResults
	/// @param[in]	userCallback
	/// @param[in]	userCallbackData
	///
	/// @return			Search the database for all the UGCs that match various search criteria.
	///					outBuffer should be an preallocated array of [sizeof(SenUGCSearchResult)*maxNrResults] bytes.
	///
	/// @details		Enumerate by Descriptor using a Logical And against all submitted Descriptor values.
	///					The API filters the results on a specific UGC Type as well as an author
	///					list type.  Author List type of Everyone is NOT supported.
	///					Note: The collection of descriptor bit masks is constrained to four.  
	///
	/// @related		SenUGCEnumerate
	HRESULT SenUGCEnumerateByDescriptorWithLogicalAnd( 
		int userIndex,
		SenSysTitleID titleID,
		SenUGCSortBy sortBy,
		SenUGCType type,
		SenUGCAuthorType authorType,
		int nrAuthors,
		const PlayerUID *authorList,
		SenUGCMetaDataFlags metaDataFlagFilter,
		SYSTEMTIME *newerThan,
		int nrDescriptors,
		INT64 *descriptors,
		int maxNrResults,
		SenUGCSearchResult *outBuffer,
		UINT *outNrResults,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );

	/// @brief			public 
	///
	/// @param[in]		userIndex
	/// @param[in]		titleID
	/// @param[in]		type
	/// @param[in]		authorType
	/// @param[in]		nrAuthors
	/// @param[in]		authorList
	/// @param[in]		name
	/// @param[in]		maxNrResults
	/// @param[out]		outBuffer
	/// @param[out]		outNrResults
	/// @param[in]		userCallback
	/// @param[in]		userCallbackData
	///
	/// @return			Search the database for all the UGCs that match various search criteria.
	///					outBuffer should be an preallocated array of [sizeof(SenUGCSearchResult)*maxNrResults] bytes.
	///
	/// @details		Enumerate by name will perform a wild card lookup on UGC name.  The lookup will return anything 
	///					in the range of "%<NAME>%".  The API filters the results on a specific UGC Type as well as an author
	///					list type.  Author List type of Everyone is NOT supported.
	///
	/// @related		SenUGCEnumerate
	__declspec(deprecated("Use SenUGCEnumerateByName() instead"))
	HRESULT SenUGCEnumerate( 
		int userIndex,
		SenSysTitleID titleID,
		SenUGCType type,
		SenUGCAuthorType authorType,
		int nrAuthors,
		const PlayerUID *authorList,
		const wchar_t *name,
		int maxNrResults,
		SenUGCSearchResult *outBuffer,
		UINT *outNrResults,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );

	/// @brief			public 
	///
	/// @param[in]	userIndex
	/// @param[in]	titleID
	/// @param[in]	type
	/// @param[in]	authorType
	/// @param[in]	nrAuthors
	/// @param[in]	authorList
	/// @param[in]	name
	/// @param[in]	performWildCardLookup
	/// @param[in]	maxNrResults
	/// @param[out]	outBuffer
	/// @param[out]	outNrResults
	/// @param[in]	userCallback
	/// @param[in]	userCallbackData
	///
	/// @return			Search the database for all the UGCs that match various search criteria.
	///					outBuffer should be an preallocated array of [sizeof(SenUGCSearchResult)*maxNrResults] bytes.
	///
	/// @details		Enumerate by name will perform an exact or wild card string lookup on UGC name.  The API filters the results 
	///                 on a specific UGC Type as well as an author	list type.  Author List type of Everyone is NOT supported.
	///
	/// @related		SenUGCEnumerate
	HRESULT SenUGCEnumerateByName( 
		int userIndex,
		SenSysTitleID titleID,
		SenUGCType type,
		SenUGCAuthorType authorType,
		int nrAuthors,
		const PlayerUID *authorList,
		const wchar_t *name,
		bool performWildCardLookup,
		int maxNrResults,
		SenUGCSearchResult *outBuffer,
		UINT *outNrResults,
		SenSysCompletedCallback userCallback,
		void* userCallbackData );

	//************************************
	// Method:    SenUGCDownloadMetaData
	// FullName:  Sentient::SenUGCDownloadMetaData
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: int nrUGCs
	// Parameter: const SenUGCID * ugcIDList
	// Parameter: SenUGCDownloadedMetaData2 * outBuffer
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Download the metadata for an array of UGCs
	// Note that the metadata structure is a superset of the uploaded metadata,
	// with various other information exposed.
	// outBuffer should be an preallocated array of [(sizeof(SenUGCDownloadedMetaData)+SenUGCMetaData::BlobSizeLimit)*nrUGCs] bytes.
	// This new signature is compatible with resubmission feature and 64-bit UGC Ids.
	//************************************
	HRESULT SenUGCDownloadMetaData(
		int userIndex,
		SenSysTitleID titleID,
		int nrUGCs,
		const SenUGCID *ugcIDList,
		SenUGCDownloadedMetaData2 *out_metaData,
		size_t *out_metaDataCount,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	__declspec(deprecated("Use signature with SenUGCDownloadedMetaData2."))
	HRESULT SenUGCDownloadMetaData(
		int userIndex,
		SenSysTitleID titleID,
		int nrUGCs,
		const SenUGCID *ugcIDList,
		SenUGCDownloadedMetaData *out_metaData,
		size_t *out_metaDataCount,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	//************************************
	// Method:    SenUGCDownloadMainDataBlob
	// FullName:  Sentient::SenUGCDownloadMainDataBlob
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: int mainDataBlobID
	// Parameter: size_t bufferSize
	// Parameter: UINT blobVersion
	// Parameter: void * outBuffer
	// Parameter: size_t * outSize
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Download a single blob of UGC data
	// note that zero byte downloads will fail.
	// ID, blobVersion, and bufferSize should be coming 
	// from SenUGCDownloadedMetaData.
	// outBuffer should be an preallocated array of bufferSize bytes.
	//************************************
	HRESULT SenUGCDownloadMainDataBlob(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		UINT mainDataBlobIndex,
		size_t bufferSize,
		UINT blobVersion,
		void *outBuffer,
		size_t *outSize,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	__declspec(deprecated("Use signature with blobVersion."))
	HRESULT SenUGCDownloadMainDataBlob(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		int mainDataBlobIndex,
		size_t bufferSize,
		void *outBuffer,
		size_t *outBytesReceived,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	//************************************
	// Method:    SenUGCDownloadMainDataBlob
	// FullName:  Sentient::SenUGCDownloadMainDataBlob
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcID
	// Parameter: int mainDataBlobID
	// Parameter: size_t bufferSize
	// Parameter: UINT blobVersion
	// Parameter: void * outBuffer
	// Parameter: size_t * outBytesReceived
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Download a single blob of UGC data
	// NOTE: zero byte downloads will fail.
	// ID, mainDataRevision and bufferSize should be coming from a 
	//   SenUGCDownloadedMetaData2 (where bufferSize comes from 
	//   SenUGCDownloadedMetaData2's BlobInfo[mainDataBlobID].Size).
	//   outBuffer should be an preallocated array of bufferSize bytes.
    // This signature includes an out param to include the progress
    //   percentage and cancellation token, etc to monitor and abort 
    //   long running downloads (can be wired up to UI for users).
	// BlobVersion is the version of the blob you want to download,
	//   which is available on the metadata information for the 
	//   main data blobs (via DownloadMetaData).
	//************************************
	HRESULT SenUGCDownloadMainDataBlob(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		UINT mainDataBlobIndex,
		size_t bufferSize,
		UINT blobVersion,
		void *outBuffer,
		size_t *outBytesReceived,
		SenSysCompletedCallback userCallback,
		void *userCallbackData,
        SenUGCProgressInfo* out_progressInfo);

	__declspec(deprecated("Use signature with blobVersion."))
	HRESULT SenUGCDownloadMainDataBlob(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		int mainDataBlobIndex,
		size_t bufferSize,
		void *outBuffer,
		size_t *outBytesReceived,
		SenSysCompletedCallback userCallback,
		void *userCallbackData,
        SenUGCProgressInfo* out_progressInfo);

	/**********************************************
	 ***** UGC Reviewing and Rating Functions *****
	 **********************************************/

	//************************************
	// Method:    SenUGCSetReviewScore
	// FullName:  Sentient::SenUGCSetReviewScore
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcId
	// Parameter: SenUGCReviewScoreType type
	// Parameter: UINT score
	// Parameter: bool isFavorite
	// Parameter: SenSysCompletedCallback callback
	// Parameter: void * callbackData
	//************************************
	HRESULT SenUGCSetReviewScore(
		int userIndex,
		SenUGCID ugcId,
		SenUGCReviewScoreType type,
		unsigned int score,
		bool isFavorite,
		SenSysCompletedCallback callback, 
		void *callbackData );

	//************************************
	// Method:    SenUGCGetReviewScore
	// FullName:  Sentient::SenUGCGetReviewScore
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenUGCID ugcId
	// Parameter: SenUGCReviewScoreType type
	// Parameter: UINT * out_score
	// Parameter: SenSysCompletedCallback callback
	// Parameter: void * callbackData
	//************************************
	HRESULT SenUGCGetReviewScore(
		int userIndex, 
		SenUGCID ugcId, 
		SenUGCReviewScoreType type, 
		unsigned int *out_score,
		SenSysCompletedCallback callback, 
		void *callbackData );

	//************************************
	// Method:    SenUGCSetFavoriteFlag
	// FullName:  Sentient::SenUGCSetFavoriteFlag
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: SenUGCID ugcID
	// Parameter: bool isFavorite
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Users can flag the UGCs that they really like, which can be used for
	// the search results
	//************************************
	HRESULT SenUGCSetFavoriteFlag(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		BOOL isFavorite,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	//************************************
	// Method:    SenUGCGetFavoriteFlag
	// FullName:  Sentient::SenUGCGetFavoriteFlag
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: SenUGCID ugcID
	// Parameter: BOOL * outResult
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Users can flag the UGCs that they really like, which can be used for
	// the search results
	//************************************
	HRESULT SenUGCGetFavoriteFlag(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		BOOL *outResult,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	//************************************
	// Method:    SenUGCGetFriendFavoriteCount
	// FullName:  Sentient::SenUGCGetFriendFavoriteCount
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: SenUGCID ugcID
	// Parameter: int * outResult
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Find out how many friends of the user have flagged this UGC as 
	// a favorite (inclusive the user's favorite flag also)
	//************************************
	HRESULT SenUGCGetFriendFavoriteCount(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		int *outResult,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );
	
	//************************************
	// Method:    SenUGCAddToCustomCounters
	// FullName:  Sentient::SenUGCAddToCustomCounters
	// Access:    public 
	// Returns:   HRESULT
	// Qualifier:
	// Parameter: int userIndex
	// Parameter: SenSysTitleID titleID
	// Parameter: SenUGCID ugcID
	// Parameter: INT64 customCounters[SenUGCDownloadedMetaData_NrCustomCounters]
	// Parameter: SenSysCompletedCallback userCallback
	// Parameter: void * userCallbackData
	// Users can add to a fixed number of global counters stored on the 
	// servers, to count up a few basic stats per UGC (number of deaths, 
	// number of enemies killed, total playtime etc.)
	//************************************
	HRESULT SenUGCAddToCustomCounters(
		int userIndex,
		SenSysTitleID titleID,
		SenUGCID ugcID,
		INT64 customCounters[SenUGCDownloadedMetaData_NrCustomCounters],
		SenSysCompletedCallback userCallback, 
		void* userCallbackData );

	/// @brief			API to flag a given piece of UGC as offensive. 
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]		ugcID
	///					The unique ID for an instance of UGC.
	///
	/// @param[in]		offensivenessFlag
	///					Offensive flag type.
	///
	/// @param[in]		reason
	///					Reason for marking a given piece of UGC as offensive.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///
	/// @details		Users can flag a level that they think is offensive. The goal is that
	///					the Sentient system will automatically be able to take down 
	///					levels after enough people have flagged a UGC as bad.
	///					The number of votes to take something down will depend on the 
	///					reliability of the reviewers (number of offensiveness flags vs number of
	///					downloads etc.) as well as the number of offensive uploads by the creator.
	///					This function is also used by moderators to confirm or deny something as 
	///					being offensive.
	///
	/// @related		<Related API>
	HRESULT SenUGCSetOffensivenessFlag(
		int userIndex,
		SenUGCID ugcID,
		SenUGCOffensivenessFlag offensivenessFlag,
		const wchar_t *reason,
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );

	/// @brief			This function will return whether or not a particular 
	///                 piece of UGC has been banned.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]		The unique ID for UGC.
	///
	/// @param[in]		True if content is banned (and should not be viewed by user).
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///
	/// @details		Any piece of UGC can been banned by a moderator or 
	///					moderation engine.  This API allows clients to verify 
	///					if a given piece of UGC has been banned.
	///
	/// @related		SenUGCCreatePublishingUGCID()
	/// @related		SenUGCSetOffensivenessFlag()
	/// @related		SenUGCPublish()
	HRESULT SenUGCIsBanned(
		int userIndex,
		SenUGCID ugcID,
		BOOL *out_result,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/*********************
	 ***** UGC Feeds *****
	 *********************/

	/// @brief          UGC Feed information
	///
	/// @details        When enumerating feeds, these are the available feeds that can be retrieved.
	///
	struct SenUGCFeedInfo
	{
		SenUGCFeedType feedType;
		wchar_t Name[32];
		wchar_t Description[128];
	};

	/// @brief			Retrieves a specific feed based on feedtype. 
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///`
	/// @param[in]		titleID
	///					
	/// @param[in]		feedType
	///					Feed Type identifier for for the given feed being retrieved.
	///
	/// @param[in]		maxNumberOfFeedItems
	///					Used to indicate the number of items to be returned by @a out_feedInfo.
	///					If the actual number of items exceeds this, you will receive an error.
	///
	/// @param[out]		out_buffer
	///					Pointer to the collection of structures to fill with SenUGCFeedItem data.
	///
	/// @param[out]		out_buffersize
	///					The number of entries actually enumerated by the call.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///					E_POINTER: out_buffer or out_buffersize are null.
	///
	/// @details		<Insert detailed method documentation>
	///
	/// @related		SenUGCEnumerateFeeds()
	HRESULT SenUGCGetFeed(
		int userIndex, 
		SenSysTitleID titleID, 
		SenUGCFeedType feedType, 
		size_t maxNumberOfFeedItems, 
		SenUGCFeedItem *out_buffer,
		UINT *out_buffersize,
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );
	
	/// @brief			Retrieves a collection of feeds that are viewable by the
	///					current user.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]      culture
	///                 This is the result of a call to SenCultureFind() or SenCultureGet*().
	///                 You may also pass nullptr to use the culture set with SenCultureSetCurrent().
	///					May be nullptr for default culture.
	///
	/// @param[in]		maxResults
	///					Used to indicate the number of items to be returned by @a out_feedInfo.
	///					If the actual number of items exceeds this, you will receive an error.
	///
	/// @param[out]		out_feedInfo
	///					Pointer to a collection of structures to fill with SenUGCFeedInfo data.
	///					
	/// @param[out]		out_resultCount
	///					The number of entries actually enumerated by the call.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///					E_POINTER: out_feedInfo or out_resultCount are null.
	///
	/// @details		<Insert detailed method documentation>
	///
	/// @related		SenUGCGetFeed()
	HRESULT SenUGCEnumerateFeeds(
		int userIndex,
		size_t maxResults,
		SenUGCFeedInfo *out_feedInfo,
		size_t *out_resultCount,
		SenSysCompletedCallback userCallback,
		void *userCallbackData);

	/// @brief			API that publishes UGC and makes the content accessible 
	///					to everyone on the Sentient service.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]		ugcID
	///					The unique ID for an instance of UGC.
	///
	/// @param[in]		leaderboardDefinition
	///					Optional parameter.  Definition for a Leaderboard that 
	///					will be created and associated to newly published UGC.
	///
	/// @param[out]		out_leaderboardId
	///					Created Leaderboard Id.  Only returned to the client if
	///					a Leaderboards Definition is passed to the server.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///
	/// @details		UGC is only accessible to the  author until it has been published. 
	///					The user making the call must be the author of the UGC item.  
	///					The call will fail if this UGC item has previously been published.
	///					By supplying an optional Leaderboard definition a Leaderboard is automatically
	//					allocated and associated with the UGC item.  
	///					This is the preferred way of creating UGC Leaderboards. 
	///
	/// @related		SenCreateLeaderboard()
	HRESULT SenUGCPublish(
		int userIndex,
		SenUGCID ugcID,
		const SenLeaderboardDefinition *leaderboardDefinition,
		SenLeaderboardId *out_leaderboardId,
		SenSysCompletedCallback userCallback,
		void *userCallbackData);

	/// @brief			API that publishes a new version of a UGC item and makes the revised content accessible 
	///					to everyone on the Sentient service.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]		ugcID
	///					The unique ID for an instance of UGC.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///
	/// @details		New versions of UGC are only accessible to the author until it has been republished. 
	///					The user making the call must be the author of the UGC item.  
	///
	/// @related		SenUGCPublish()
	HRESULT SenUGCRepublish(
		int userIndex,
		SenUGCID ugcID,
		SenSysCompletedCallback userCallback,
		void* userCallbackData);

} // namespace Sentient
