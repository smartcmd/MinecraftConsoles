
#ifndef SCE_REMOTE_STORAGE_H
#define SCE_REMOTE_STORAGE_H

#include "sceRemoteStorageDefines.h"

/// @brief
/// Initialises the RemoteStorage library.
///
/// Initialises the RemoteStorage library, creates a session on the server and starts the Thread to process requests.
/// This method must be executed to start the RemoteStorage library or none of its functionality will be available.
/// This method will block while it initializes its thread and will return an error if it is 
/// unable to do so. The session will be created on the thread once this is created and it won't be a blocking operation.
///
/// It is important to note that HTTP, SSL and NET libraries are not being initialised by the library and should be initialised outside of it.
///
/// @param params		The structure of type <>SceRemoteStorageInitParams</c> that contains necessary information to start the library. 
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successfully registered on the thread.
/// @retval SCE_REMOTE_STORAGE_ERROR_INVALID_ARGUMENT		At least one of the arguments passed in the input structure is not valid.
/// @retval SCE_REMOTE_STORAGE_ERROR_FAILED_TO_ALLOCATE		There is no enough memory on the library to perform an allocation.
/// @retval USER_ACCOUNT_LINKED								This event will be sent to the event callback when the session is created and linked to PSN on the server
/// @retval PSN_SIGN_IN_REQUIRED							This event will be sent to the event callback when the session is created but not linked to PSN on the server. 
///															This will only happen on the PC version and requires to call <c>sceRemoteStorageOpenWebBrowser()</c> function.
/// @retval ERROR_OCCURRED									This event will be sent to the event callback when an error has occurred in the thread.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageInit(const SceRemoteStorageInitParams & params);

/// @brief
/// Terminates the RemoteStorage library.
///
/// Terminates the RemoteStorage library and deletes the thread that process requests.
/// This method must be executed to terminate the RemoteStorage library to prevent leaks in memory and resources.
/// This method will abort any other pending requests and terminate the library. It won't wait for requests to finish.
/// This method is synchronous and does not make use of the callback to inform the user of success termination. It is executed on the calling thread.
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successful.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageTerm();

/// @brief
/// Aborts a request sent to the RemoteStorage library.
///
/// Aborts a request being processed or pending to be processed by the RemoteStorage library.
/// This method is synchronous and does not make use of the callback to inform the user of success termination. It is executed on the calling thread.
///
/// @param param	A structure containing	the request Id to be aborted. 
///					This request Id is provided by other functions (get/setData, getStatus and OpenWebBrowser) so they can be referenced. 
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successful.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
/// @retval SCE_REMOTE_STORAGE_ERROR_REQ_ID_NOT_FOUND		The request Id sent is not found.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageAbort(const SceRemoteStorageAbortReqParams & params);

/// @brief
/// Opens the default web browser to sign in to PSN on PC.
///
/// Opens the default web browser to sign in to PSN on PC. This function does not have any functionality on other platforms.
/// This method does make use of the callback to inform the user of success termination. This function has priority over other functions on the thread (as getData(), getStatus() 
/// and setData()) and it will be executed as soon as the thread finishes processing a pending request.  
///
/// @param param		The structure containing extra parameters to be passed in. This structure does only exist for future expansions. 
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successfully registered on the thread.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
/// @retval SCE_REMOTE_STORAGE_ERROR_FAILED_TO_ALLOCATE		There is no enough memory on the library to perform an allocation.
/// @retval ERROR_OCCURRED									This event will be sent to the event callback when an error has occurred in the thread.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageOpenWebBrowser(const SceRemoteStorageWebBrowserReqParams & params);

/// @brief
/// Gives details for all files of a user.
///
/// Gives details for all files of a user. It provides generic information (remaining bandwidth per day, HDD space per user, number of files) as well as
/// specific file information (number of bytes, file name, file description, MD5 checksum, timestamp and file visibility). File data is not provided.
/// This method does make use of the callback to inform the user of success termination. The SceRemoteStorageStatus pointer must be pointer a to a valid 
/// location in memory until the callback is called as the output information will be stored in such location.
///
/// @param params		The structure containing extra parameters to be passed in. This structure does only exist for future expansions. 
/// @param status		The structure where the output information will be stored. The memory location being pointed must be valid until the callback gets called.
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successfully registered on the thread.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
/// @retval SCE_REMOTE_STORAGE_ERROR_FAILED_TO_ALLOCATE		There is no enough memory on the library to perform an allocation.
/// @retval ERROR_OCCURRED									This event will be sent to the event callback when an error has occurred in the thread.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageGetStatus(const SceRemoteStorageStatusReqParams & params, SceRemoteStorageStatus * status);

/// @brief
/// Gets section of data from a file specified.
///
/// Gets section of data from a file specified. The amount of data requested can be of any size. To request this information the name of file, the number of bytes and
/// the byte to start reading along with a buffer to store such data must be provided.
/// Metadata information of the file, as description or visibility, will be provided only in the case the first amount of bytes for the file are requested (offset = 0). 
/// This method does make use of the callback to inform the user of success termination. The SceRemoteStorageData pointer must be a pointer to a valid 
/// location in memory until the callback is called as the output information will be stored in such location.
///
/// @param params		The structure containing the file name to read, the start byte to start reading and the amount of bytes to read. 
/// @param status		The structure where the output information will be stored. The memory location being pointed must be valid until the callback gets called.
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successfully registered on the thread.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
/// @retval SCE_REMOTE_STORAGE_ERROR_FAILED_TO_ALLOCATE		There is no enough memory on the library to perform an allocation.
/// @retval ERROR_OCCURRED									This event will be sent to the event callback when an error has occurred in the thread.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageGetData(const SceRemoteStorageGetDataReqParams & params, SceRemoteStorageData * data);

/// @brief
/// Sets chunk of data to a file specified.
///
/// Sets chunk of data to a file specified. The amount of data sent must be of, at least, 5 Megabytes per chunk excepts
/// in the case of the last chunk of the file (or the only one if that is the case) as it can be smaller.
/// The information provided regarding the chunk as the chunk number, total number of chunks, data buffer and its size should be provided in every call.
/// The information provided regarding the file as its name, description and visibility should be provided in the last chunk only (this is, when 
/// chunk number = number of chunks).
/// This method does make use of the callback to inform the user of success termination. The data attribute of the SceRemoteStorageSetDataReqParams pointer 
/// must be a pointer to a valid location in memory until the callback is called as the buffer won't be copied internally.
///
/// @param data		The structure containing the chunk information.
///
/// @retval SCE_REMOTE_STORAGE_SUCCESS						The operation was successfully registered on the thread.
/// @retval SCE_REMOTE_STORAGE_ERROR_NOT_INITIALISED		The RemoteStorage library was not initialised.
/// @retval SCE_REMOTE_STORAGE_ERROR_FAILED_TO_ALLOCATE		There is no enough memory on the library to perform an allocation.
/// @retval ERROR_OCCURRED									This event will be sent to the event callback when an error has occurred in the thread.
///
/// @note System errors may be returned. Design your code so it does expect other errors.
int32_t sceRemoteStorageSetData(const SceRemoteStorageSetDataReqParams & data);

#endif