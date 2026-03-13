/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client AvatarSuperstars Avatar API
//
// Include this to get access to all Avatar-related Sentient features.

#pragma once

#include "SenClientRawData.h"
#include "SenClientResource.h"
#include "SenClientSys.h"
#include "SenClientXML.h"

#include <xnamath.h>
#include <xonline.h>

namespace Sentient
{
	//====================//
	//                    //
	//    Avatar Types    //
	//                    //
	//====================//

	// When enumerating avatars, these are the options for pre-sorting the
	// returned list.
	enum SenAvatarSortBy
	{
		SenAvatarSortBy_Default     = 0,
		// ... TBD ...

	};

	// This structure contains the information needed to download the 
	// raw data for a given gender.
	struct SenAvatarGenderInfo
	{
		SenRawDataTransferInfo metadata;
		SenRawDataTransferInfo assets;
		SenRawDataTransferInfo xml;
		SenRawDataTransferInfo icon;
	};

	// This structure contains the original uploaded info plus info
	// needed to download raw data.
	struct SenAvatarInfo : public SenResourceInfo
	{
		int                 vipLevel;
		SenAvatarGenderInfo female;
		SenAvatarGenderInfo male;
	};

	struct SenAvatarPalette
	{
		XMCOLOR entry[3];
	};

	struct SenAvatarNamedPalette : SenAvatarPalette
	{
		wchar_t name[57+1];
	};

	/// @brief           Extra avatar information.
	///
	/// @details         This structure contains additional data about the avatar, such as localized strings and palettes.
	///
	struct SenAvatarExtraInfo
	{
		wchar_t title[XMARKETPLACE_MAX_TITLE_NAME_LENGTH+1];            ///< The localized game title associated with the avatar.
		wchar_t name[XMARKETPLACE_MAX_OFFER_NAME_LENGTH+1];             ///< The localized name associated with the avatar.
		wchar_t description[XMARKETPLACE_MAX_OFFER_SELL_TEXT_LENGTH+1]; ///< The localized short description associated with the avatar.

		size_t                paletteCount;
		SenAvatarNamedPalette palette[16];
	};

	//========================//
	//                        //
	//    Avatar Functions    //
	//                        //
	//========================//

	/// @brief           Search the database for all the avatars that match the search criteria.
	///
	/// @param[in]       userIndex
	///                  The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]       avatarInfoCountMax
	///                  The number of SenAvatarInfo structures available in @a out_avatarInfoList.
	///
	/// @param[out]      out_avatarInfoCount
	///                  This is the number of entries actually enumerated by the call.
	///
	/// @param[out]      out_avatarInfoList
	///                  The structures to fill in with the enumerated information.
	///                  It is assumed that this is preallocated to at least @a avatarInfoCountMax entries.
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
	///                  E_POINTER: out_avatarInfoList is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Enumerates in default order, at the current time, for all titles.
	///
	/// @related         SenAvatarDownloadExtraInfo()
	/// @related         SenAvatarDownloadMetadata()
	/// @related         SenAvatarDownloadAssets()
	/// @related         SenAvatarDownloadIcon()
	///
	HRESULT SenAvatarEnumerate(
		int userIndex,
		size_t avatarInfoCountMax,
		size_t *out_avatarInfoCount,
		SenAvatarInfo *out_avatarInfoList,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	// Search the database for a specific avatar at the current time.
	/// @brief           Search the database for a specific avatar at the current time
	///
	/// @param[in]       titleID
	///                  The ID of the title that the avatar item is associated with.
	///
	/// @param[in]       resourceID
	///                  The ID of the specific asset about which information should be returned.
	///
	/// @param[out]      out_avatarInfo
	///                  The structures to fill in with the information.
	///                  It is assumed that this is preallocated to at least 1 entry.
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
	///                  E_POINTER: out_avatarInfo is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @related         SenAvatarEnumerate()
	/// @related         SenAvatarDownloadExtraInfo()
	/// @related         SenAvatarDownloadMetadata()
	/// @related         SenAvatarDownloadAssets()
	/// @related         SenAvatarDownloadIcon()
	///
	HRESULT SenAvatarFind(
		SenSysTitleID titleID,
		SenResourceID resourceID,
		SenAvatarInfo *out_avatarInfo,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download the avatar metadata to the client.
	///
	/// @param[in]       avatarInfo
	///                  The info describing the attributes and data of the avatar asset.
	///                  This is obtained from SenAvatarEnumerate().
	///
	/// @param[in]       male
	///                  Whether or not to receive information about the male avatar (vs. the female)
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a avatarInfo.[fe]male.metadata.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the metadata.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: avatarInfo.resourceID or avatarInfo.[fe]male.metadata is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @related         SenAvatarEnumerate()
	/// @related         SenAvatarDownloadExtraInfo()
	/// @related         SenAvatarDownloadAssets()
	/// @related         SenAvatarDownloadIcon()
	///
	HRESULT SenAvatarDownloadMetadata(
		const SenAvatarInfo &avatarInfo,
		bool male,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download the avatar asset binary data to the client.
	///
	/// @param[in]       avatarInfo
	///                  The info describing the attributes and data of the avatar asset.
	///                  This is obtained from SenAvatarEnumerate().
	///
	/// @param[in]       male
	///                  Whether or not to receive information about the male avatar (vs. the female)
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a avatarInfo.[fe]male.assets.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the asset data.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: avatarInfo.resourceID or avatarInfo.[fe]male.assets is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @related         SenAvatarEnumerate()
	/// @related         SenAvatarDownloadExtraInfo()
	/// @related         SenAvatarDownloadMetadata()
	/// @related         SenAvatarDownloadIcon()
	///
	HRESULT SenAvatarDownloadAssets(
		const SenAvatarInfo &avatarInfo,
		bool male,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download the avatar icon binary data to the client.
	///
	/// @param[in]       avatarInfo
	///                  The info describing the attributes and data of the avatar asset.
	///                  This is obtained from SenAvatarEnumerate().
	///
	/// @param[in]       male
	///                  Whether or not to receive information about the male avatar (vs. the female)
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a avatarInfo.[fe]male.icon.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the binary icon data.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: avatarInfo.resourceID or avatarInfo.[fe]male.icon is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @related         SenAvatarEnumerate()
	/// @related         SenAvatarDownloadExtraInfo()
	/// @related         SenAvatarDownloadMetadata()
	/// @related         SenAvatarDownloadAssets()
	///
	HRESULT SenAvatarDownloadIcon(
		const SenAvatarInfo &avatarInfo,
		bool male,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download extra information about a given avatar to the client.
	///
	/// @param[in]       avatarInfo
	///                  The info describing the attributes and data of the avatar.
	///                  This is obtained from SenAvatarEnumerate().
	///
	/// @param[in]       male
	///                  Whether or not to receive information about the male avatar (vs. the female).
	///
	/// @param[out]      out_avatarExtraInfo
	///                  The structure to populate with extra information.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: avatarInfo.resourceID or avatarInfo.xml is invalid.
	///                  E_POINTER: out_avatarExtraInfo is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         On completion, the structure will contain extra information, such as localized strings.
	///
	/// @related         SenAvatarEnumerate()
	/// @related         SenAvatarDownloadMetadata()
	/// @related         SenAvatarDownloadAssets()
	/// @related         SenAvatarDownloadIcon()
	///
	HRESULT SenAvatarDownloadExtraInfo(
		const SenAvatarInfo &avatarInfo,
		bool male,
		SenAvatarExtraInfo *out_avatarExtraInfo,
		SenSysCompletedCallback userCallback, void *userCallbackData );

	// Download the raw binary data to the client.
	// It is assumed that out_data is preallocated to (at least) dataSizeMax bytes,
	// which in turn should be at least avatarInfo.[fe]male.xml.GetBufferSize() bytes.
	HRESULT SenAvatarDownloadXML(
		const SenAvatarInfo &avatarInfo,
		bool male,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	// Obtain a wide, localized string with a given avatar's game title.
	//
	// Assumes you have already downloaded the XML and run it through
	// SenXMLParse() to get a SenXML struct.
	//
	//  First method, filling a fixed-size buffer:
	//
	//   wchar_t buffer[1234];
	//   SenAvatarXMLGetTitle( xml, loc, _countof(buffer), nullptr, buffer );
	//
	// Second method, filling a dynamically-allocated buffer:
	//
	//   size_t bufferLength;
	//   SenAvatarXMLGetTitle( xml, loc, 0, &bufferLength, nullptr );
	//   wchar_t buffer = new wchar_t[bufferLength];
	//   SenAvatarXMLGetTitle( xml, loc, bufferLength, nullptr, buffer );
	//
	// Note that bufferLength is in wchars, and includes the terminating nul.
	// The actual length of the _string_ is (*out_bufferLength - 1).
	//
	HRESULT SenAvatarXMLGetTitle(
		const SenXML &senXML,
		size_t bufferLengthMax,
		size_t *out_bufferLength,  // optional
		wchar_t *out_buffer );     // optional

	// Obtain a wide, localized string with a given avatar's name.
	// See the similar SenAvatarGetTitle() for details.
	HRESULT SenAvatarXMLGetName(
		const SenXML &senXML,
		size_t bufferLengthMax,
		size_t *out_bufferLength,  // optional
		wchar_t *out_buffer );     // optional

	// Obtain a wide, localized string with a given avatar's description.
	// See the similar SenAvatarGetTitle() for details.
	HRESULT SenAvatarXMLGetDescription(
		const SenXML &senXML,
		size_t bufferLengthMax,
		size_t *out_bufferLength,  // optional
		wchar_t *out_buffer );     // optional

	// Obtain the number of custom avatar palettes.
	HRESULT SenAvatarXMLGetPaletteCount(
		const SenXML &senXML,
		size_t *out_paletteCount );

	// Obtain the localized name for an avatar palettes.
	// See the similar SenAvatarGetTitle() for details.
	HRESULT SenAvatarXMLGetPaletteName(
		const SenXML &senXML,
		int paletteIndex,
		size_t bufferLengthMax,
		size_t *out_bufferLength,  // optional
		wchar_t *out_buffer );     // optional

	// Obtain a single palette at a given index in the list of palettes
	// for a given avatar.
	HRESULT SenAvatarXMLGetPalette(
		const SenXML &senXML,
		int paletteIndex,
		SenAvatarPalette *out_palette );

	// Extract all palette entries at once.
	// It is assumed that out_paletteList is preallocated to (at least)
	// paletteCountMax entries.
	HRESULT SenAvatarXMLGetPalettes(
		const SenXML &senXML,
		size_t paletteCountMax,
		size_t *out_paletteCount,  // optional
		SenAvatarPalette *out_paletteList );

} // namespace Sentient
