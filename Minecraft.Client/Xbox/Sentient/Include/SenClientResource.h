/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client Resource API
//
// Include this to get access to all Resource-related Sentient features.

#pragma once

#include "SenClientTypes.h"


namespace Sentient
{
	//======================//
	//                      //
	//    Resource Types    //
	//                      //
	//======================//

	/// @brief           Resource types/categories.
	///
	/// @details         This value is normally used to fill the top 8 bits of a resource ID, with the bottom 24 being an index.
	///
	enum SenResourceType : INT8
	{
		SenResourceType_Invalid,
		SenResourceType_Avatar,
		SenResourceType_BoxArt,
		SenResourceType_Config,
		SenResourceType_Help,
	};

	/// @brief           Resource types/categories.
	///
	/// @details         The top 8 bits are always the SenResourceType.  The bottom 24 are typically an index into the category.
	///                  These categories do not overlap, so that we can choose to store different resource types in the same or 
    ///                  different tables, depending on what is more useful.  Note, though, that you should expect E_INVALIDARG
	///                  back if you pass an ID from one category to a call wanting a different category.
	///                  Some categories' IDs will be hardwired/assumed, while others will be generated inside of Microsoft (e.g. box art).
	///
    enum SenResourceID : INT32
    {
		/// This is used to indicate a failed search, an invalid resource structure, or sometimes to substitute for a default.
        SenResourceID_Invalid     = static_cast<INT32>(SenResourceType_Invalid) << 24,

        /// This is the first VIP reward costume and there is one for each title.
        SenResourceID_Superstar_0 = static_cast<INT32>(SenResourceType_Avatar)  << 24,
        /// This is the second VIP reward costume and there is one for each title.
        SenResourceID_Superstar_1,
        /// This is the third VIP reward costume and there is one for each title.
        SenResourceID_Superstar_2,
        /// This is the fourth VIP reward costume and there is only one, shared across all titles.
        SenResourceID_Superstar_3,
        /// This is the fifth VIP reward costume and there is only one, shared across all titles.
        SenResourceID_Superstar_4,

        /// This is used for the cross-sell screen and contains things such as an image, offerID, strings, etc.
        SenResourceID_BoxArt_0    = static_cast<INT32>(SenResourceType_BoxArt)  << 24,

        /// This is used for game-private config files, and is only the base of the range.
		/// Titles may use the entire 24-bit space for various custom config files.
        SenResourceID_Config_0    = static_cast<INT32>(SenResourceType_Config)  << 24,

        /// This is used for server-supplied help files/text.
		/// At the moment, this is not supported.
        SenResourceID_Help_0      = static_cast<INT32>(SenResourceType_Help)    << 24,

    };

	/// @brief           Resource schedule priority.
	///
	/// @details         This is currently reserved for later use in overriding one resource with another on a schedule.
	///                  It is not currently used and may be changed at a later date.
	///
    typedef INT32 SenResourcePriority;
    enum 
    {
        SenResourcePriority_Default = 0,
    };

	/// @brief           Generic resource information.
	///
	/// @details         This structure contains enough information to uniquely identify
    ///                  one resource at a given time on the schedule, e.g. an avatar or 
    ///                  box art at 3am.  (Note that schedules are not currently used and
	///                  may be re-architected in the future.)
	///
    struct SenResourceInfo
    {
        SenSysTitleID       titleID;    ///< This is the title the resource is assigned to, or SenTitleID_Shared for all.
        SenResourceID       resourceID; ///< This is the resource ID within the title's space.

        SenResourcePriority priority;   ///< Schedule priority.  This is not currently used and may be changed at a later date.
        SYSTEMTIME          begins;     ///< Scheduled begin time (inclusive).  This is not currently used and may be changed at a later date.
        SYSTEMTIME          ends;       ///< Scheduled end time (exclusive).  This is not currently used and may be changed at a later date.
    };


	//==========================//
	//                          //
	//    Resource Functions    //
	//                          //
	//==========================//

	// None at the moment.

} // namespace Sentient
