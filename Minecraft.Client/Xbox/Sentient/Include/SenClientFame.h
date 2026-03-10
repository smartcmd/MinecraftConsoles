/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client AvatarSuperstars Fame API
//
// Include this to get access to all Fame-related Sentient features.

#pragma once

#include "SenClientSys.h"

namespace Sentient
{
	/**********************
	 ***** Fame Types *****
	 **********************/
	enum SenFameVIPLevel
	{
		SenFameVIPLevel_Unknown		= 0xFFFFFFFF,
		SenFameVIPLevel_Fan			= 0,
		SenFameVIPLevel_Newcomer	= 1,
		SenFameVIPLevel_UpAndComing = 2,
		SenFameVIPLevel_Headliner	= 3,
		SenFameVIPLevel_Star		= 4,
		SenFameVIPLevel_Superstar	= 5,
	};

	/// @brief					Information about a user's VIP status
	///
	/// @details				This structure contains the user's current VIP level and fame points 
	///
	struct SenFameVIPData
	{
		PlayerUID user;								///< ID for the user whose VIP status this instance describes
		unsigned int vipLevel;					///< Current VIP level [0-n]
		unsigned int lastAckedVIPLevel;			///< VIP level last time this structure was acknowledged by a Sentient client
		unsigned int famePointsCurrent;			///< Fame Points accrued across all Fame titles since the last VIP level passed.
		unsigned int famePointsWeek;			///< Fame Points accrued across all Fame titles this week.
		unsigned int famePointsLifetime;		///< Fame Points accrued across all Fame titles over the user's entire history.
		unsigned int pointsToNextLevel;			///< Incremental Fame Points that must be acquired to gain a new VIP level.
		unsigned int superstarCounter;			///< Number of times the user has achieved the maximum possible VIP level.
		SYSTEMTIME vipLevelExpiresAt;			///< Date at which current VIP level will expire.  Only relevant when vipLevelExpires is true.
		bool vipLevelExpires;					///< Whether or not the current VIP level will expire.
	};

	/// @brief					Information about a single row in a Fame leaderboard
	///
	/// @details				This structure contains the identity of the user and summary information about their Fame status
	///
	struct SenFameLeaderboardEntry
	{
		PlayerUID			user;					///< ID for the user this row describes
		unsigned int	vipLevel;				///< Current VIP level[0-n]
		unsigned int	famePoints;				///< Fame Points accrued.  May be weekly or lifetime depending on leaderboard type queried.
		unsigned int	superstarCounter;		///< Number of times the user has achieved the maximum possible VIP level.
		unsigned int	rank;					///< Global rank in the leaderboard [1-n]
	};

	/// @brief					Leaderboard query ranking options
	///
	/// @details				When querying leaderboards, these are the options for how the leaderboard is ranked.
	///
	enum SenFameLeaderboardRankType
	{
		SenFameLeaderboardRankType_Week,			///< Return ranking for fame points earned this week.
		SenFameLeaderboardRankType_Lifetime,		///< Return ranking for fame points earned all time
		SenFameLeaderboardRankType_Superstar		///< Return ranking for superstar counter
	};

	/// @brief					Leaderboard query filter options
	///
	/// @details				When querying leaderboards, these are the options for how the leaderboard is filtered.
	///
	enum SenFameLeaderboardFilter
	{
		SenFameLeaderboardFilter_Everyone = 0,		///< Return the unfiltered leaderboard
		SenFameLeaderboardFilter_Friends,			///< Filter leaderboard by friends.
	};

	/// @brief					Information about the parameters for a leaderboard query
	///
	/// @details				This structure should be filled in to specify parameters for a leaderboard query
	///
	struct SenFameLeaderboardRequest
	{
		SenFameLeaderboardRankType	type;			///< Ranking option for this query.
		SenFameLeaderboardFilter	filter;			///< Filter option for this query.
		int							startIndex;		///< Rank at which leaderboard query should start.  Set to -1 to center on querying user. 
	};

	/// @brief					Information about the results of a leaderboard query
	///
	/// @details				This structure contains information about the results of a leaderboard query.
	///
	struct SenFameLeaderboardResults
	{
		unsigned int	playerIndex;				///< When playerIndex < numEntriesReturned, provides the index into result set at which the row for the querying user is located.
		size_t			numEntriesReturned;			///< Number of rows returned by the query.
		size_t			numLeaderboardEntries;		///< Total number of rows in the leaderboard.
	};

	/// @brief					Fame progress (challenge) types
	///
	/// @details				Defines a set of well-known challenge types, plus a range for titles to use for their custom challenge types
	///
	enum SenFameProgressID
	{
		SenFameProgressID_TitleDefinedFirst = 0,		///< First possible ID for a title-defined challenge.
		SenFameProgressID_TitleDefinedLast = 1023,		///< Last possible ID for a title-defined challenge.

		SenFameProgressID_FirstPlay = 1024,				///< Challenge tracks the first time a user plays a given title.  This challenge is implemented on the Sentient server.  Do not submit updates for it.
		SenFameProgressID_AvatarAward1,					///< Challenge tracks the user receiving the first available Avatar Award.  Progress against this challenge must be submitted by titles.
		SenFameProgressID_AvatarAward2,					///< Challenge tracks the user receiving the second available Avatar Award.  Progress against this challenge must be submitted by titles.
		SenFameProgressID_FriendsOwnTitle,				

		// These challenges are not currently implemented.  Contact senhelp@microsoft.com before using.
		SenFameProgressID_MPWithFriend,
		SenFameProgressID_MPWithVIP1,
		SenFameProgressID_MPWithVIP2,
		SenFameProgressID_MPWithVIP3,
		SenFameProgressID_MPWithVIP4,
		SenFameProgressID_MPWithVIP5,
		SenFameProgressID_FriendsAtVIP1,
		SenFameProgressID_FriendsAtVIP2,
		SenFameProgressID_FriendsAtVIP3,
		SenFameProgressID_FriendsAtVIP4,
		SenFameProgressID_FriendsAtVIP5,

		SenFameProgressID_Invalid = 0xffffffff			///< Reserved identifier for an invalid challenge.
	};

	/// @brief					Constants that may be reported when Fame APIs return a count.
	enum SenFameCount : unsigned int
	{
		SenFameCount_Unbounded = 0xffffffff				///< Indicates that there is no fixed limit on the number of items.
	};

	/// @brief					Information about a granted award (milestone)
	///
	/// @details				When querying for awards, this structure will be filled out with summary information about any award granted to the user.
	struct SenAwardMessageData
	{
		wchar_t					awardDesc[128];			///< Localized string containing a message for display.
		unsigned int			awardPoints;			///< Fame Points granted as a result of this award.
		unsigned int			awardTrigger;			///< Progress within the associated Challenge that caused the award to trigger.
	};

	/// @brief					Measures a time period.
	///
	/// @details				Provides a display-friendly way to report time differences - e.g. the time until the current Fame week expires.
	struct SenFameTime
	{
		int days;				
		int hours;				
		int minutes;
		int seconds;
	};

	/// @brief					Information about a user's progress against a single Challenge
	///
	/// @details				Provides a display-friendly format for retrieving information about a user's progress against Fame Challenges.
	///
	struct SenFameDisplayData
	{
		wchar_t					milestoneTypeDescription[128];		///< Localized string that describes the challenge.
		SenFameProgressID		milestoneTypeID;					///< ID for the Challenge.
		unsigned int			milestoneCount;						///< Total number of milestones (awards) available for this Challenge.  May be SenFameCount_Unbounded.
		unsigned int			currentMilestone;					///< Index of the milestone the user is currently working towards (i.e. 0 indicates no milestones have been passed).
		unsigned int			xpSinceLastMilestone;				///< Progress achieved since the last milestone in this Challenge was passed.
		unsigned int			xpToNextMilestone;					///< Progress required to hit the next milestone in this Challenge.  Expressed as the progress difference between milestones, i.e. does not vary with xpSinceLastMilestone
		unsigned int			famePointsSoFar;					///< Fame Points achieved in this Challenge.
		unsigned int			famePointsMaximum;					///< Total Fame Points available from this Challenge.  May be SenFameCount_Unbounded.
		bool					isWeekly;							///< When true, progress against this Challenge is reset weekly.
	};

	/// @brief					Information about a participant in a multiplayer game.
	///
	/// @details				Use for reporting information about multiplayer games to the Sentient server.
	///
	struct SenFameMPGameParticipant
	{
		PlayerUID user;				///< ID of a user who should be credited with participation in the game.
		bool winner;			///< When true, this user should be considered among the winners of the game.  There are no restrictions on the number of 'winners' a game may have.
	};


	/**************************
	 ***** Fame Functions *****
	 **************************/

	/// @brief           Query the server for VIP status information for a collection of arbitrary users.
	///
	/// @param[in]       userIndex
	///                  The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]       userIDCount
	///                  The number of valid XUIDs in @a userIDArray
	///
	/// @param[in]		 userIDArray
	///					 Users for whom VIP data should be retrieved.
	///
	/// @param[out]      out_userFameVIPArray
	///                  The structures to fill in with the retrieved information.
	///                  It is assumed that this is preallocated to at least @a userIDCount entries.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                  E_POINTER: out_userFameVIPArray is nullptr.
	///					 SENTIENT_E_TOO_MANY_CALLS: This call has been rejected to avoid excessive server load.  Try again later.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         This overload can be used to retrieve VIP status for friends or remote participants in a multiplayer session.
	///					 For local users, prefer SenGetFameVIPLevel(int, SenFameVIPData*) 
	///
	HRESULT SenGetFameVIPLevel(
		int userIndex,
		size_t userIDCount,
		const PlayerUID *userIDArray,
		SenFameVIPData *out_userFameVIPArray,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Query for VIP status information for a local user.
	///
	/// @param[in]       userIndex
	///                  The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[out]      out_fameVIPData
	///                  The structure to fill in with the retrieved information.
	///                  It is assumed that this is preallocated to fit at least 1 entry.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                  E_POINTER: out_fameVIPData is nullptr.
	///					 SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_fameVIPData has not been filled in.
	///                  S_OK: The operation completed successfully.
	///
	/// @details         This overload is preferred when retrieving information about local users. 
	///					 There are no restrictions on the call frequency and it will typically return immediately.
	///					 In rare cases where SENTIENT_S_OPERATION_IN_PROGRESS is returned the title should call again
	///					 on the next scheduled iteration of their Sentient update loop.
	///
	HRESULT SenGetFameVIPLevel(
		int userIndex, 
		SenFameVIPData *out_fameVIPData );

	/// @brief			Acknowledge a change in user VIP level reported by the server.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_FAIL: Failed to spawn server call.
	///                 S_OK: Server call spawned successfully.
	///					
	/// @details		When retrieving user VIP status, the server will include the user's last VIP level
	///					for which it has not received an acknowledgement message.  Titles can use this information
	///					to highlight changes in VIP level.  Once a change has been messaged titles should
	///					call this API to clear the server state.
	HRESULT SenAcknowledgeVIPLevelChange(
		int userIndex );

	/// @brief			Query a Fame leaderboard
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]		leaderboardRequest
	///					Defines the parameters for the leaderboard query.
	///
	/// @param[in]		entryCountMax
	///					The maximum number of leaderboard rows to return.
	///
	/// @param[out]		out_entryArray
	///					The structures to fill in with the rows returned from the leaderboard query.
	///					It is assumed that this is preallocated to hold at least entryCountMax entries.
	///
	/// @param[out]		out_leaderboardResults
	///					Summary information about the results of the leaderboard query.
	///
	/// @param[out]		out_senHandle
	///					Provides a handle to the async task, which will allow for calling SentientCancel
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]      userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return         Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_POINTER: out_entryArray or out_leaderboardResults is nullptr.
	///					E_INVALIDARG: userCallback is nullptr and out_senHandle is non-nullptr.  Task handles are not supported for synchronous requests.
	///					SENTIENT_E_TOO_MANY_CALLS: This call has been rejected to avoid excessive server load.  Try again later.
	///                 E_FAIL: Failed to spawn server call.
	///                 S_OK: Server call spawned successfully.
	///
	HRESULT SenGetFameLeaderboard(
		int userIndex, 
		const SenFameLeaderboardRequest &leaderboardRequest,
		size_t entryCountMax, 
		SenFameLeaderboardEntry *out_entryArray, 
		SenFameLeaderboardResults *out_leaderboardResults,
		SenHandle *out_senHandle, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );


	/// @brief			Poll for notifications of when a user passes a Fame milestone.
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[out]		out_awardData
	///					Structure to fill in with information about any new award.
	///
	/// @return			SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_POINTER: out_awardData is nullptr.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_awardData has not been filled in.
	///					S_FALSE: The operation completed successfully but there were no awards to report.  out_awardData has not been filled in.
	///                 S_OK: The operation completed successfully and there was a valid award to report.  out_awardData contains information about the award.
	///
	/// @details		There are no restrictions on how frequently this API may be called, and it returnes immediately, so titles should poll 
	///					in all states where they can display award messages.  When a message is returned it is popped from an internal queue
	///					and will not be returned again by further calls.
	///
	HRESULT SenGetAwardMessage(
		int userIndex, 
		SenAwardMessageData *out_awardData );

	/// @brief			Retrieve the time left before weekly fame progress is reset.
	///
	/// @param[out]		out_timeRemaining
	///					Structure to fill in with information about time remaining.
	///
	///	@return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 E_POINTER: out_timeRemaining is nullptr.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_timeRemaining has not been filled in.
	///                 E_FAIL: Internal failure.  Check log for output.
	///                 S_OK: Call completed successfully and out_timeRemaining has been filled in.
	///
	/// @details		Some Fame Challenges are reset weekly.  Use this API when displaying a timer for the reset.
	///
	HRESULT SenGetTimeLeftInFameWeek(
		SenFameTime *out_timeRemaining );

	// 
	/// @brief			Retrieve the time left before transient VIP level is reset. 
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	///	@param[out]		out_timeRemaining
	///					Structure to fill in with information about time remaining.
	///
	///	@return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_POINTER: out_timeRemaining is nullptr.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_timeRemaining has not been filled in.
	///					S_FALSE: The VIP level of the supplied user does not expire.  out_timeRemaining has not been filled in.
	///                 E_FAIL: Internal failure.  Check log for output.
	///                 S_OK: Call completed successfully and out_timeRemaining has been filled in.
	///
	/// @details		Some VIP levels are reset if the user does not actively maintain them.  Use this API
	///					when displaying a timer for the reset.
	///
	HRESULT SenGetTimeLeftInVIPLevel(
		int userIndex, 
		SenFameTime *out_timeRemaining );

	/// @brief			Get a localized string that names the given VIP level.
	///
	///	@param[in]		vipLevel
	///					The level whose name should be returned.
	///
	/// @param[in]		maxNameLength
	///					The maximum length (including null terminating character) of the string to return.
	///
	///	@param[out]		out_name
	///					The string to fill in with the VIP level name.
	///					It is assumed that this is preallocated to fit at least @a maxNameLength characters.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 E_POINTER: out_name is nullptr.
	///					E_INVALIDARG: vipLevel is outside the range of known VIP levels.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_name has not been filled in.
	///                 S_OK: The operation completed successfully.
	///
	///	@details		Titles can use this API to get the server-defined name for a VIP level for additional flexibility post-release.
	///					In rare cases where SENTIENT_S_OPERATION_IN_PROGRESS is returned the title should call again
	///					on the next scheduled iteration of their Sentient update loop.	
	///	
	HRESULT SenGetVIPLevelName(
		unsigned int vipLevel,
		size_t maxNameLength,
		wchar_t *out_name);


	/// @brief			Get the maximum number of items that will be returned by a call to SenGetFameDisplayData
	///
	/// @param[out]		out_count
	///					Location to be filled in with the number of display data items available.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_POINTER: out_count is nullptr.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and out_count has not been filled in.
	///                 E_FAIL: Internal failure.  Check log for output.
	///                 S_OK: The operation completed successfully.
	///
	/// @details		Titles can use this API to ensure that they allocate a buffer of appropriate size
	///					before making a call to SenGetFameDisplayData.
	///
	HRESULT SenGetFameDisplayDataCount(
		size_t *out_count );

	/// @brief			Retrieve a summary of a user's progress against Fame Challenges.
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.	
	///
	/// @param[in]		startIndex
	///					Global index of first item to return.  
	///					This parameter can be used to support results paging.
	///
	/// @param[in]		maxDisplayDataCount
	///					Maximum number of items to return.
	///
	/// @param[out]		out_dataCount
	///					Location to fill in with number of items actually returned.
	///
	/// @param[out]		out_displayData
	///                 The structures to fill in with the retrieved information.
	///                 It is assumed that this is preallocated to at least @a maxDisplayDataCount entries.
	///
	///	@return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///                 E_POINTER: out_dataCount or out_displayData is nullptr.
	///					E_INVALIDARG: startIndex is greater than the total number of items available.
	///					SENTIENT_S_OPERATION_IN_PROGRESS: The call could not be completed immediately and output parameters have not been filled in.
	///                 E_FAIL: Internal failure.  Check log for output.
	///                 S_OK: The operation completed successfully.
	///
	/// @details		For titles with weekly Challenge rotations, only Challenges active for the current week are reported.
	///					Use SenGetFameDisplayDataCount() to dynamically size a buffer large enough to obtain all the display data entries
	///					in a single call, or use the startIndex parameter for paging.  
	///
	HRESULT SenGetFameDisplayData(
		int userIndex, 
		size_t startIndex, 
		size_t maxDisplayDataCount, 
		size_t *out_dataCount, 
		SenFameDisplayData *out_displayData );

	/// @brief			Notify Sentient about user progress against a Fame Challenge.
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]		milestoneTypeID
	///					The id of the Challenge for which progress has been achieved.
	///
	/// @param[in]		xpGained
	///					Incremental progress.  For binary milestones this can be safely set to 1 each time the awarding event takes place. 
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///					SENTIENT_E_BUFFER_EXHAUSTED: The progress update failed because Sentient's internal buffer is full.
	///                 E_FAIL: Internal failure.  Check log for output.
	///                 S_OK: The operation completed successfully.
	///
	/// @details		Titles should call this API whenever a user makes progress against a Fame Challenge.  Long-term storage for progress exists
	///					on the Sentient server, but it may be buffered on the client to prevent excessive server load.  Titles should call 
	///					SenFlushFameProgress as appropriate (e.g. on game save) to ensure that all progress writes are committed to the server.
	///					Titles should not submit updates against Fame Challenges whose progress is determined by the server(e.g. First Play)
	HRESULT SenUpdateFameProgress(
		int userIndex, 
		unsigned int milestoneTypeID, 
		unsigned int xpGained );

	/// @brief			Ensure that all progress writes are committed to the Sentient server.
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]      userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///					SENTIENT_E_TOO_MANY_CALLS: This call has been rejected to avoid excessive server load.  Try again later.
	///					E_FAIL: Failed to spawn server call.  This may be because there is already a flush scheduled.
	///					S_OK: The operation completed successfully.
	///
	/// @details		Long-term storage for progress exists on the Sentient server, but it may be buffered on the client to 
	///					prevent excessive server load.  Titles should call this API as appropriate (e.g. on game save) to ensure 
	///					that all progress writes are committed to the server.  The callback parameters provide a mechanism for
	///					detecting when the server call has completed and updating user interface appropriately.
	///					When there is no local data that needs flushing, the supplied callback will be invoked before execution
	///					returns from this function, and the return code will be S_OK.
	///						
	HRESULT SenFlushFameProgress(
		int userIndex, 
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );

	/// @brief			Inform the Sentient server about the results of a multiplayer game.
	///
	/// @param[in]		userIndex
	///					The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]		participantCount
	///					The number of valid items in @a participants.
	///
	///	@param[in]		participants
	///					Structures describing the users who participated in this multiplayer game.
	///
	/// @param[out]		out_senHandle
	///					Provides a handle to the async task, which will allow for calling SentientCancel
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]      userCallbackData
	///                 Data to be passed to the @a userCallback on completion.		
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                 SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                 SENTIENT_E_GUEST_ACCESS_VIOLATION: A guest may not spawn this call.
	///					E_INVALIDARG: Either userCallback is nullptr and out_senHandle is non-nullptr, or participantCount is less than 2.
	///                 E_POINTER: participants is nullptr.
	///                 E_FAIL: Failed to spawn server call.
	///                 S_OK: Server call spawned successfully.
	///				
	/// @details		Titles should report multiplayer sessions to the Sentient server via this API in order to support
	///					tracking of progress against certain Fame Challenges.  For proper tracking each user in the multiplayer 
	///					session should independently call this function on completion of a game.
	HRESULT SenRegisterFameMPGame(
		int userIndex,
		size_t participantCount,
		const SenFameMPGameParticipant *participants,
		SenHandle *out_senHandle, 
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );

} // namespace Sentient
