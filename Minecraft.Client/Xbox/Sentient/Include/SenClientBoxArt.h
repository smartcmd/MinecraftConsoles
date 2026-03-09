/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client BoxArt API
//
// Include this to get access to all BoxArt-related Sentient features.

#pragma once

#include "SenClientRawData.h"
#include "SenClientResource.h"
#include "SenClientSys.h"
#include "SenClientXML.h"

#include <xonline.h>

namespace Sentient
{
	//====================//
	//                    //
	//    BoxArt Types    //
	//                    //
	//====================//

	/// @brief           Box Art sorting options.
	///
	/// @details         When enumerating box Art, these are the options for pre-sorting the returned list.
	///
	enum SenBoxArtSortBy
	{
		/// Sort by the default order (currently just the order stored on the server).
		SenBoxArtSortBy_Default = 0,
	};

	/// @brief           Box art command/action types.
	///
	/// @details         When the user selects box art, these are the options for generating the resulting action.
	///
	enum SenBoxArtCommand
	{
		/// The game should call up the marketplace UI with info.GetOfferID().
		SenBoxArtCommand_Store = 0,

		// (more commands TBD)
	};

	/// @brief           Basic box art information.
	///
	/// @details         This structure contains the original uploaded info plus any info implied by the raw data that was uploaded (e.g. data sizes).
	///
	struct SenBoxArtInfo : public SenResourceInfo
	{
		SenRawDataTransferInfo image;      ///< Points to a file, typically JPG or PNG, which can be passed to D3DXCreateTextureFromFileInMemory().
		SenRawDataTransferInfo xml;        ///< Points to an opaque XML file used to hold extended information about the box art.  Use SenBoxArtDownloadExtraInfo() to retrieve.

		SenBoxArtCommand command;          ///< What to do when the box art is selected.
		INT32            commandData0;     ///< 32 bits of arbitrary data to go with command.  (e.g. offerID high bits)
		INT32            commandData1;     ///< 32 bits of arbitrary data to go with command.  (e.g. offerID low bits)

		UINT32			 friendsWithTitle; ///< Number of friends owning the title this box art is associated with.

		/// @brief
		///     Get the offerID associated with this box art.
		///
		/// @details
		///     If command==SenBoxArtCommand_Store, this returns commandData0,1 as a SenSysOfferID.
		///     Otherwise, it returns SenSysOfferID_Invalid.
		///
		SenSysOfferID GetOfferID() const;
	};

	/// @brief           Extra box art information.
	///
	/// @details         This structure contains additional data about box art, such as localized strings.
	///
	struct SenBoxArtExtraInfo
	{
		wchar_t title[XMARKETPLACE_MAX_OFFER_NAME_LENGTH+1];            ///< The localized title associated with the offer.
		wchar_t description[XMARKETPLACE_MAX_OFFER_SELL_TEXT_LENGTH+1]; ///< The localized short description associated with the offer.
	};

	//========================//
	//                        //
	//    BoxArt Functions    //
	//                        //
	//========================//

	
	/// @brief           Search the database for all the box arts that match the search criteria.
	///
	/// @param[in]       userIndex
	///                  The index of the initiating user on the console.  Note: This is NOT a XUID.
	///
	/// @param[in]       boxArtInfoCountMax
	///                  The number of SenBoxArtInfo structures available in @a out_boxArtInfoList.
	///
	/// @param[out]      out_boxArtInfoCount
	///                  This is the number of entries actually enumerated by the call.
	///
	/// @param[out]      out_boxArtInfoList
	///                  The structures to fill in with the enumerated information.
	///                  It is assumed that this is preallocated to at least @a boxArtInfoCountMax entries.
	///
	/// @param[out]		 out_senHandle
	///					 Provides a handle to the async task, which will allow for calling SentientCancel
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
	///                  E_POINTER: out_boxArtInfoList is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Enumerates in default order, at the current time, for all titles, the box art available to display.
	///
	/// @related         SenBoxArtDownloadImage()
	/// @related         SenBoxArtDownloadXML()
	///
	HRESULT SenBoxArtEnumerate(
		int userIndex,
		size_t boxArtInfoCountMax,
		size_t *out_boxArtInfoCount,
		SenBoxArtInfo *out_boxArtInfoList,
		SenHandle *out_senHandle,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );
	
	/// @brief           Download the raw box art image data to the client.
	///
	/// @param[in]       boxArtInfo
	///                  The info describing the attributes and data of the box art.
	///                  This is obtained from SenBoxArtEnumerate().
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a boxArtInfo.image.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the raw image data.
	///
	/// @param[out]		 out_senHandle
	///					 Provides a handle to the async task, which will allow for calling SentientCancel
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: boxArtInfo.resourceID or boxArtInfo.image is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         The data is expected to contain a PNG or JPG file, suitable for use with D3DXCreateTextureFromFileInMemory().
	///
	/// @related         SenBoxArtEnumerate()
	/// @related         SenBoxArtDownloadXML()
	///
	HRESULT SenBoxArtDownloadImage(
		const SenBoxArtInfo &boxArtInfo,
		size_t dataSizeMax,
		void *out_data, 
		SenHandle *out_senHandle,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download extra information about a give box art to the client.
	///
	/// @param[in]       boxArtInfo
	///                  The info describing the attributes and data of the box art.
	///                  This is obtained from SenBoxArtEnumerate().
	///
	/// @param[out]      out_boxArtExtraInfo
	///                  The structure to populate with extra information.
	///
	/// @param[out]		 out_senHandle
	///					 Provides a handle to the async task, which will allow for calling SentientCancel
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: boxArtInfo.resourceID or boxArtInfo.xml is invalid.
	///                  E_POINTER: out_boxArtExtraInfo is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         On completion, the structure will contain extra information, such as localized strings.
	///
	/// @related         SenBoxArtEnumerate()
	/// @related         SenBoxArtDownloadImage()
	///
	HRESULT SenBoxArtDownloadExtraInfo(
		const SenBoxArtInfo &boxArtInfo,
		SenBoxArtExtraInfo *out_boxArtExtraInfo,
		SenHandle *out_senHandle,
		SenSysCompletedCallback userCallback, void *userCallbackData );

	/// @brief           Download the raw box art xml data to the client.
	///
	/// @param[in]       boxArtInfo
	///                  The info describing the attributes and data of the box art.
	///                  This is obtained from SenBoxArtEnumerate().
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a boxArtInfo.xml.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the raw image data.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: boxArtInfo.resourceID or boxArtInfo.xml is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         The data is expected to contain a UTF-8 XML file, though those details
	///                  are irrelevant, as the contents are parsed transparently with SenXMLParse().
	///
	/// @related         SenBoxArtEnumerate()
	/// @related         SenBoxArtDownloadImage()
	///
	/// @deprecated      Use SenBoxArtDownloadExtraInfo() instead.
	///
	__declspec(deprecated("This function is deprecated, use SenBoxArtDownloadExtraInfo instead")) HRESULT SenBoxArtDownloadXML(
		const SenBoxArtInfo &boxArtInfo,
		size_t dataSizeMax,
		void *out_data,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Obtain a wide, localized string describing a given box art's game title.
	///
	/// @param[in]       senXML
	///                  This is the result of a call to SenXMLParse() with the results of
	///                  SenBoxArtDownloadXML().
	///
	/// @param[in]       culture
	///                  This is the result of a call to SenCultureFind() or SenCultureGet*().
	///                  You may also pass nullptr to use the culture set with SenCultureSetCurrent().
	///
	/// @param[in]       bufferLength
	///                  Note that bufferLength is in wchars, and needs to _include_ space for the terminating nul.
	///
	/// @param[out]      out_bufferLength
	///                  Used to return the actual number of wchars written to the buffer, including the terminating nul.
	///                  The actual length of the _string_ is (*out_bufferLength - 1).
	///                  Pass @a out_bufferLength = nullptr if you don't care about the actual size.
	///
	/// @param[out]      out_buffer
	///                  The buffer to fill in with the string.
	///                  It is assumed that this is preallocated to at least @a bufferLength wchars.
	///                  Pass @a out_buffer = nullptr if you are only interested in finding out the necessary buffer size.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_UNEXPECTED: passed a nullptr culture without a default culture being set first.
	///                  E_INVALIDARG: senXML does not contain parsed XML data.
	///                  E_FAIL: Failed to locate text.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Assumes you have already downloaded the XML and run it through
	///                  SenXMLParse() to get a SenXML struct.
	///
	///                  First method, filling a fixed-size buffer:
	///
	///                  wchar_t buffer[1234];
	///                  SenBoxArtXMLGetTitle( xml, culture, _countof(buffer), nullptr, buffer );
	///
	///                  Second method, filling a dynamically-allocated buffer:
	///
	///                  size_t bufferLength;
	///                  SenBoxArtXMLGetTitle( xml, culture, 0, &bufferLength, nullptr );
	///                  wchar_t buffer = new wchar_t[bufferLength];
	///                  SenBoxArtXMLGetTitle( xml, culture, bufferLength, nullptr, buffer );
	///
	/// @related         SenBoxArtDownloadXML()
	/// @related         SenXMLParse()
	///
	/// @deprecated      Use SenBoxArtDownloadExtraInfo() instead.
	///
	__declspec(deprecated("This function is deprecated, use SenBoxArtDownloadExtraInfo instead")) HRESULT SenBoxArtXMLGetTitle(
		const SenXML &senXML,
		size_t bufferLengthMax,
		size_t *out_bufferLength,
		wchar_t *out_buffer );

	/// @brief           Obtain a wide, localized string describing a given box art's game title.
	///
	/// @param[in]       senXML
	///                  This is the result of a call to SenXMLParse() with the results of
	///                  SenBoxArtDownloadXML().
	///
	/// @param[in]       culture
	///                  This is the result of a call to SenCultureFind() or SenCultureGet*().
	///                  You may also pass nullptr to use the culture set with SenCultureSetCurrent().
	///
	/// @param[in]       bufferLength
	///                  Note that bufferLength is in wchars, and needs to _include_ space for the terminating nul.
	///
	/// @param[out]      out_bufferLength
	///                  Used to return the actual number of wchars written to the buffer, including the terminating nul.
	///                  The actual length of the _string_ is (*out_bufferLength - 1).
	///                  Pass @a out_bufferLength = nullptr if you don't care about the actual size.
	///
	/// @param[out]      out_buffer
	///                  The buffer to fill in with the string.
	///                  It is assumed that this is preallocated to at least @a bufferLength wchars.
	///                  Pass @a out_buffer = nullptr if you are only interested in finding out the necessary buffer size.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_UNEXPECTED: passed a nullptr culture without a default culture being set first.
	///                  E_INVALIDARG: senXML does not contain parsed XML data.
	///                  E_FAIL: Failed to locate text.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Assumes you have already downloaded the XML and run it through
	///                  SenXMLParse() to get a SenXML struct.
	///
	///                  First method, filling a fixed-size buffer:
	///
	///                  wchar_t buffer[1234];
	///                  SenBoxArtXMLGetDescription( xml, culture, _countof(buffer), nullptr, buffer );
	///
	///                  Second method, filling a dynamically-allocated buffer:
	///
	///                  size_t bufferLength;
	///                  SenBoxArtXMLGetDescription( xml, culture, 0, &bufferLength, nullptr );
	///                  wchar_t buffer = new wchar_t[bufferLength];
	///                  SenBoxArtXMLGetDescription( xml, culture, bufferLength, nullptr, buffer );
	///
	/// @related         SenBoxArtDownloadXML()
	/// @related         SenXMLParse()
	///
	/// @deprecated      Use SenBoxArtDownloadExtraInfo() instead.
	///
	__declspec(deprecated("This function is deprecated, use SenBoxArtDownloadExtraInfo instead")) HRESULT SenBoxArtXMLGetDescription(
		const SenXML &senXML,
		size_t bufferLengthMax,
		size_t *out_bufferLength,
		wchar_t *out_buffer );

	/// @brief           Download the raw box art image data to the client.
	///
	/// @param[in]       boxArtInfo
	///                  The info describing the attributes and data of the box art.
	///                  This is obtained from SenBoxArtEnumerate().
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a boxArtInfo.image.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the raw image data.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: boxArtInfo.resourceID or boxArtInfo.image is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         The data is expected to contain a PNG or JPG file, suitable for use with D3DXCreateTextureFromFileInMemory().
	///
	/// @related         SenBoxArtEnumerate()
	/// @related         SenBoxArtDownloadXML()
	///
	__declspec(deprecated("This function is deprecated, use the overload with a SenHandle out param instead")) HRESULT SenBoxArtDownloadImage(
		const SenBoxArtInfo &boxArtInfo,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

} // namespace Sentient
