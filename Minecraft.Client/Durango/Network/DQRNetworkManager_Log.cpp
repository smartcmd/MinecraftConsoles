#include "stdafx.h"

#include "DQRNetworkManager.h"
#include "PartyController.h"
#include <collection.h>
#include <ppltasks.h>
#include <ws2tcpip.h>
#include "../Minecraft.World/StringHelpers.h"
#include "base64.h"

#ifdef _DURANGO
#include "../Minecraft.World/DurangoStats.h"
#endif

#include "ChatIntegrationLayer.h"

using namespace Concurrency;
using namespace Windows::Foundation::Collections;

void DQRNetworkManager::LogComment( Platform::String^ strText )
{
#ifndef _CONTENT_PACKAGE
	static int64_t firstTime = 0;
	wchar_t buf[64];

	int64_t currentTime = System::currentTimeMillis();
	if( firstTime != 0 )
	{
		_i64tow_s(currentTime - firstTime, buf, 64, 10);
		OutputDebugString(buf);
		OutputDebugString(L" ms: ");
	}
	else
	{
		firstTime = currentTime;
	}
	OutputDebugString(strText->Data());
	OutputDebugString(L"\n");
#endif
}

void DQRNetworkManager::LogCommentFormat( LPCWSTR strMsg, ... )
{
    WCHAR strBuffer[2048];

    va_list args;
    va_start(args, strMsg);
    _vsnwprintf_s( strBuffer, 2048, _TRUNCATE, strMsg, args );
    strBuffer[2047] = L'\0';

    va_end(args);

    LogComment(ref new Platform::String(strBuffer));
}

void DQRNetworkManager::LogCommentWithError( Platform::String^ strTest, HRESULT hr )
{
	Platform::String^ final = strTest + GetErrorString(hr);
	LogComment(final);
}

Platform::String^ DQRNetworkManager::GetErrorString( HRESULT hr )
{
    Platform::String^ str = FormatString(L" %s [0x%0.8x]", ConvertHResultToErrorName(hr)->Data(), hr );
    return str;
}

Platform::String^ DQRNetworkManager::FormatString( LPCWSTR strMsg, ... )
{
    WCHAR strBuffer[2048];

    va_list args;
    va_start(args, strMsg);
    _vsnwprintf_s( strBuffer, 2048, _TRUNCATE, strMsg, args );
    strBuffer[2047] = L'\0';

    va_end(args);

    Platform::String^ str = ref new Platform::String(strBuffer);
    return str;
}

Platform::String^ DQRNetworkManager::ConvertHResultToErrorName( HRESULT hr )
{
    switch( hr )
    {
        // Generic errors
        case S_OK: return L"S_OK";
        case S_FALSE: return L"S_FALSE";
        case E_OUTOFMEMORY: return L"E_OUTOFMEMORY";
        case E_ACCESSDENIED: return L"E_ACCESSDENIED";
        case E_INVALIDARG: return L"E_INVALIDARG";
        case E_UNEXPECTED: return L"E_UNEXPECTED";
        case E_ABORT: return L"E_ABORT";
        case E_FAIL: return L"E_FAIL";
        case E_NOTIMPL: return L"E_NOTIMPL";
        case E_ILLEGAL_METHOD_CALL: return L"E_ILLEGAL_METHOD_CALL";

        // Authentication specific errors
        case 0x87DD0003: return L"AM_E_XASD_UNEXPECTED";
        case 0x87DD0004: return L"AM_E_XASU_UNEXPECTED";
        case 0x87DD0005: return L"AM_E_XAST_UNEXPECTED";
        case 0x87DD0006: return L"AM_E_XSTS_UNEXPECTED";
        case 0x87DD0007: return L"AM_E_XDEVICE_UNEXPECTED";
        case 0x87DD0008: return L"AM_E_DEVMODE_NOT_AUTHORIZED";
        case 0x87DD0009: return L"AM_E_NOT_AUTHORIZED";
        case 0x87DD000A: return L"AM_E_FORBIDDEN";
        case 0x87DD000B: return L"AM_E_UNKNOWN_TARGET";
        case 0x87DD000C: return L"AM_E_INVALID_NSAL_DATA";
        case 0x87DD000D: return L"AM_E_TITLE_NOT_AUTHENTICATED";
        case 0x87DD000E: return L"AM_E_TITLE_NOT_AUTHORIZED";
        case 0x87DD000F: return L"AM_E_DEVICE_NOT_AUTHENTICATED";
        case 0x87DD0010: return L"AM_E_INVALID_USER_INDEX";
        case 0x87DD0011: return L"AM_E_USER_HASH_MISSING";
        case 0x87DD0012: return L"AM_E_ACTOR_NOT_SPECIFIED";
        case 0x87DD0013: return L"AM_E_USER_NOT_FOUND";
        case 0x87DD0014: return L"AM_E_INVALID_SUBTOKEN";
        case 0x87DD0015: return L"AM_E_INVALID_ENVIRONMENT";
        case 0x87DD0016: return L"AM_E_XASD_TIMEOUT";
        case 0x87DD0017: return L"AM_E_XASU_TIMEOUT";
        case 0x87DD0018: return L"AM_E_XAST_TIMEOUT";
        case 0x87DD0019: return L"AM_E_XSTS_TIMEOUT";
        case 0x8015DC00: return L"XO_E_DEVMODE_NOT_AUTHORIZED";
        case 0x8015DC01: return L"XO_E_SYSTEM_UPDATE_REQUIRED";
        case 0x8015DC02: return L"XO_E_CONTENT_UPDATE_REQUIRED";
        case 0x8015DC03: return L"XO_E_ENFORCEMENT_BAN";
        case 0x8015DC04: return L"XO_E_THIRD_PARTY_BAN";
        case 0x8015DC05: return L"XO_E_ACCOUNT_PARENTALLY_RESTRICTED";
        case 0x8015DC06: return L"XO_E_DEVICE_SUBSCRIPTION_NOT_ACTIVATED";
        case 0x8015DC08: return L"XO_E_ACCOUNT_BILLING_MAINTENANCE_REQUIRED";
        case 0x8015DC09: return L"XO_E_ACCOUNT_CREATION_REQUIRED";
        case 0x8015DC0A: return L"XO_E_ACCOUNT_TERMS_OF_USE_NOT_ACCEPTED";
        case 0x8015DC0B: return L"XO_E_ACCOUNT_COUNTRY_NOT_AUTHORIZED";
        case 0x8015DC0C: return L"XO_E_ACCOUNT_AGE_VERIFICATION_REQUIRED";
        case 0x8015DC0D: return L"XO_E_ACCOUNT_CURFEW";
        case 0x8015DC0E: return L"XO_E_ACCOUNT_ZEST_MAINTENANCE_REQUIRED";
        case 0x8015DC0F: return L"XO_E_ACCOUNT_CSV_TRANSITION_REQUIRED";
        case 0x8015DC10: return L"XO_E_ACCOUNT_MAINTENANCE_REQUIRED";
        case 0x8015DC11: return L"XO_E_ACCOUNT_TYPE_NOT_ALLOWED";
        case 0x8015DC12: return L"XO_E_CONTENT_ISOLATION";
        case 0x8015DC13: return L"XO_E_ACCOUNT_NAME_CHANGE_REQUIRED";
        case 0x8015DC14: return L"XO_E_DEVICE_CHALLENGE_REQUIRED";
        case 0x8015DC20: return L"XO_E_EXPIRED_DEVICE_TOKEN";
        case 0x8015DC21: return L"XO_E_EXPIRED_TITLE_TOKEN";
        case 0x8015DC22: return L"XO_E_EXPIRED_USER_TOKEN";
        case 0x8015DC23: return L"XO_E_INVALID_DEVICE_TOKEN";
        case 0x8015DC24: return L"XO_E_INVALID_TITLE_TOKEN";
        case 0x8015DC25: return L"XO_E_INVALID_USER_TOKEN";

        // winsock errors
        case MAKE_HRESULT(1,7,WSAEWOULDBLOCK) : return L"WSAEWOULDBLOCK";
        case MAKE_HRESULT(1,7,WSAEINPROGRESS) : return L"WSAEINPROGRESS";
        case MAKE_HRESULT(1,7,WSAEALREADY) : return L"WSAEALREADY";
        case MAKE_HRESULT(1,7,WSAENOTSOCK) : return L"WSAENOTSOCK";
        case MAKE_HRESULT(1,7,WSAEDESTADDRREQ) : return L"WSAEDESTADDRREQ";
        case MAKE_HRESULT(1,7,WSAEMSGSIZE) : return L"WSAEMSGSIZE";
        case MAKE_HRESULT(1,7,WSAEPROTOTYPE) : return L"WSAEPROTOTYPE";
        case MAKE_HRESULT(1,7,WSAENOPROTOOPT) : return L"WSAENOPROTOOPT";
        case MAKE_HRESULT(1,7,WSAEPROTONOSUPPORT) : return L"WSAEPROTONOSUPPORT";
        case MAKE_HRESULT(1,7,WSAESOCKTNOSUPPORT) : return L"WSAESOCKTNOSUPPORT";
        case MAKE_HRESULT(1,7,WSAEOPNOTSUPP) : return L"WSAEOPNOTSUPP";
        case MAKE_HRESULT(1,7,WSAEPFNOSUPPORT) : return L"WSAEPFNOSUPPORT";
        case MAKE_HRESULT(1,7,WSAEAFNOSUPPORT) : return L"WSAEAFNOSUPPORT";
        case MAKE_HRESULT(1,7,WSAEADDRINUSE) : return L"WSAEADDRINUSE";
        case MAKE_HRESULT(1,7,WSAEADDRNOTAVAIL) : return L"WSAEADDRNOTAVAIL";
        case MAKE_HRESULT(1,7,WSAENETDOWN) : return L"WSAENETDOWN";
        case MAKE_HRESULT(1,7,WSAENETUNREACH) : return L"WSAENETUNREACH";
        case MAKE_HRESULT(1,7,WSAENETRESET) : return L"WSAENETRESET";
        case MAKE_HRESULT(1,7,WSAECONNABORTED) : return L"WSAECONNABORTED";
        case MAKE_HRESULT(1,7,WSAECONNRESET) : return L"WSAECONNRESET";
        case MAKE_HRESULT(1,7,WSAENOBUFS) : return L"WSAENOBUFS";
        case MAKE_HRESULT(1,7,WSAEISCONN) : return L"WSAEISCONN";
        case MAKE_HRESULT(1,7,WSAENOTCONN) : return L"WSAENOTCONN";
        case MAKE_HRESULT(1,7,WSAESHUTDOWN) : return L"WSAESHUTDOWN";
        case MAKE_HRESULT(1,7,WSAETOOMANYREFS) : return L"WSAETOOMANYREFS";
        case MAKE_HRESULT(1,7,WSAETIMEDOUT) : return L"WSAETIMEDOUT";
        case MAKE_HRESULT(1,7,WSAECONNREFUSED) : return L"WSAECONNREFUSED";
        case MAKE_HRESULT(1,7,WSAELOOP) : return L"WSAELOOP";
        case MAKE_HRESULT(1,7,WSAENAMETOOLONG) : return L"WSAENAMETOOLONG";
        case MAKE_HRESULT(1,7,WSAEHOSTDOWN) : return L"WSAEHOSTDOWN";
        case MAKE_HRESULT(1,7,WSAEHOSTUNREACH) : return L"WSAEHOSTUNREACH";
        case MAKE_HRESULT(1,7,WSAENOTEMPTY) : return L"WSAENOTEMPTY";
        case MAKE_HRESULT(1,7,WSAEPROCLIM) : return L"WSAEPROCLIM";
        case MAKE_HRESULT(1,7,WSAEUSERS) : return L"WSAEUSERS";
        case MAKE_HRESULT(1,7,WSAEDQUOT) : return L"WSAEDQUOT";
        case MAKE_HRESULT(1,7,WSAESTALE) : return L"WSAESTALE";
        case MAKE_HRESULT(1,7,WSAEREMOTE) : return L"WSAEREMOTE";
        case MAKE_HRESULT(1,7,WSASYSNOTREADY) : return L"WSASYSNOTREADY";
        case MAKE_HRESULT(1,7,WSAVERNOTSUPPORTED) : return L"WSAVERNOTSUPPORTED";
        case MAKE_HRESULT(1,7,WSANOTINITIALISED) : return L"WSANOTINITIALISED";
        case MAKE_HRESULT(1,7,WSAEDISCON) : return L"WSAEDISCON";
        case MAKE_HRESULT(1,7,WSAENOMORE) : return L"WSAENOMORE";
        case MAKE_HRESULT(1,7,WSAECANCELLED) : return L"WSAECANCELLED";
        case MAKE_HRESULT(1,7,WSAEINVALIDPROCTABLE) : return L"WSAEINVALIDPROCTABLE";
        case MAKE_HRESULT(1,7,WSAEINVALIDPROVIDER) : return L"WSAEINVALIDPROVIDER";
        case MAKE_HRESULT(1,7,WSAEPROVIDERFAILEDINIT) : return L"WSAEPROVIDERFAILEDINIT";
        case MAKE_HRESULT(1,7,WSASYSCALLFAILURE) : return L"WSASYSCALLFAILURE";
        case MAKE_HRESULT(1,7,WSASERVICE_NOT_FOUND) : return L"WSASERVICE_NOT_FOUND";
        case MAKE_HRESULT(1,7,WSATYPE_NOT_FOUND) : return L"WSATYPE_NOT_FOUND";
        case MAKE_HRESULT(1,7,WSA_E_NO_MORE) : return L"WSA_E_NO_MORE";
        case MAKE_HRESULT(1,7,WSA_E_CANCELLED) : return L"WSA_E_CANCELLED";
        case MAKE_HRESULT(1,7,WSAEREFUSED) : return L"WSAEREFUSED";
        case MAKE_HRESULT(1,7,WSAHOST_NOT_FOUND) : return L"WSAHOST_NOT_FOUND";
        case MAKE_HRESULT(1,7,WSATRY_AGAIN) : return L"WSATRY_AGAIN";
        case MAKE_HRESULT(1,7,WSANO_RECOVERY) : return L"WSANO_RECOVERY";
        case MAKE_HRESULT(1,7,WSANO_DATA) : return L"WSANO_DATA";
        case MAKE_HRESULT(1,7,WSA_QOS_RECEIVERS) : return L"WSA_QOS_RECEIVERS";
        case MAKE_HRESULT(1,7,WSA_QOS_SENDERS) : return L"WSA_QOS_SENDERS";
        case MAKE_HRESULT(1,7,WSA_QOS_NO_SENDERS) : return L"WSA_QOS_NO_SENDERS";
        case MAKE_HRESULT(1,7,WSA_QOS_NO_RECEIVERS) : return L"WSA_QOS_NO_RECEIVERS";
        case MAKE_HRESULT(1,7,WSA_QOS_REQUEST_CONFIRMED) : return L"WSA_QOS_REQUEST_CONFIRMED";
        case MAKE_HRESULT(1,7,WSA_QOS_ADMISSION_FAILURE) : return L"WSA_QOS_ADMISSION_FAILURE";
        case MAKE_HRESULT(1,7,WSA_QOS_POLICY_FAILURE) : return L"WSA_QOS_POLICY_FAILURE";
        case MAKE_HRESULT(1,7,WSA_QOS_BAD_STYLE) : return L"WSA_QOS_BAD_STYLE";
        case MAKE_HRESULT(1,7,WSA_QOS_BAD_OBJECT) : return L"WSA_QOS_BAD_OBJECT";
        case MAKE_HRESULT(1,7,WSA_QOS_TRAFFIC_CTRL_ERROR) : return L"WSA_QOS_TRAFFIC_CTRL_ERROR";
        case MAKE_HRESULT(1,7,WSA_QOS_GENERIC_ERROR) : return L"WSA_QOS_GENERIC_ERROR";
        case MAKE_HRESULT(1,7,WSA_QOS_ESERVICETYPE) : return L"WSA_QOS_ESERVICETYPE";
        case MAKE_HRESULT(1,7,WSA_QOS_EFLOWSPEC) : return L"WSA_QOS_EFLOWSPEC";
        case MAKE_HRESULT(1,7,WSA_QOS_EPROVSPECBUF) : return L"WSA_QOS_EPROVSPECBUF";
        case MAKE_HRESULT(1,7,WSA_QOS_EFILTERSTYLE) : return L"WSA_QOS_EFILTERSTYLE";
        case MAKE_HRESULT(1,7,WSA_QOS_EFILTERTYPE) : return L"WSA_QOS_EFILTERTYPE";
        case MAKE_HRESULT(1,7,WSA_QOS_EFILTERCOUNT) : return L"WSA_QOS_EFILTERCOUNT";
        case MAKE_HRESULT(1,7,WSA_QOS_EOBJLENGTH) : return L"WSA_QOS_EOBJLENGTH";
        case MAKE_HRESULT(1,7,WSA_QOS_EFLOWCOUNT) : return L"WSA_QOS_EFLOWCOUNT";
        case MAKE_HRESULT(1,7,WSA_QOS_EUNKOWNPSOBJ) : return L"WSA_QOS_EUNKOWNPSOBJ";
        case MAKE_HRESULT(1,7,WSA_QOS_EPOLICYOBJ) : return L"WSA_QOS_EPOLICYOBJ";
        case MAKE_HRESULT(1,7,WSA_QOS_EFLOWDESC) : return L"WSA_QOS_EFLOWDESC";
        case MAKE_HRESULT(1,7,WSA_QOS_EPSFLOWSPEC) : return L"WSA_QOS_EPSFLOWSPEC";
        case MAKE_HRESULT(1,7,WSA_QOS_EPSFILTERSPEC) : return L"WSA_QOS_EPSFILTERSPEC";
        case MAKE_HRESULT(1,7,WSA_QOS_ESDMODEOBJ) : return L"WSA_QOS_ESDMODEOBJ";
        case MAKE_HRESULT(1,7,WSA_QOS_ESHAPERATEOBJ) : return L"WSA_QOS_ESHAPERATEOBJ";
        case MAKE_HRESULT(1,7,WSA_QOS_RESERVED_PETYPE) : return L"WSA_QOS_RESERVED_PETYPE";

        // HTTP specific errors
        case WEB_E_UNSUPPORTED_FORMAT: return L"WEB_E_UNSUPPORTED_FORMAT";
        case WEB_E_INVALID_XML: return L"WEB_E_INVALID_XML";
        case WEB_E_MISSING_REQUIRED_ELEMENT: return L"WEB_E_MISSING_REQUIRED_ELEMENT";
        case WEB_E_MISSING_REQUIRED_ATTRIBUTE: return L"WEB_E_MISSING_REQUIRED_ATTRIBUTE";
        case WEB_E_UNEXPECTED_CONTENT: return L"WEB_E_UNEXPECTED_CONTENT";
        case WEB_E_RESOURCE_TOO_LARGE: return L"WEB_E_RESOURCE_TOO_LARGE";
        case WEB_E_INVALID_JSON_STRING: return L"WEB_E_INVALID_JSON_STRING";
        case WEB_E_INVALID_JSON_NUMBER: return L"WEB_E_INVALID_JSON_NUMBER";
        case WEB_E_JSON_VALUE_NOT_FOUND: return L"WEB_E_JSON_VALUE_NOT_FOUND";
        case HTTP_E_STATUS_UNEXPECTED: return L"HTTP_E_STATUS_UNEXPECTED";
        case HTTP_E_STATUS_UNEXPECTED_REDIRECTION: return L"HTTP_E_STATUS_UNEXPECTED_REDIRECTION";
        case HTTP_E_STATUS_UNEXPECTED_CLIENT_ERROR: return L"HTTP_E_STATUS_UNEXPECTED_CLIENT_ERROR";
        case HTTP_E_STATUS_UNEXPECTED_SERVER_ERROR: return L"HTTP_E_STATUS_UNEXPECTED_SERVER_ERROR";
        case HTTP_E_STATUS_AMBIGUOUS: return L"HTTP_E_STATUS_AMBIGUOUS";
        case HTTP_E_STATUS_MOVED: return L"HTTP_E_STATUS_MOVED";
        case HTTP_E_STATUS_REDIRECT: return L"HTTP_E_STATUS_REDIRECT";
        case HTTP_E_STATUS_REDIRECT_METHOD: return L"HTTP_E_STATUS_REDIRECT_METHOD";
        case HTTP_E_STATUS_NOT_MODIFIED: return L"HTTP_E_STATUS_NOT_MODIFIED";
        case HTTP_E_STATUS_USE_PROXY: return L"HTTP_E_STATUS_USE_PROXY";
        case HTTP_E_STATUS_REDIRECT_KEEP_VERB: return L"HTTP_E_STATUS_REDIRECT_KEEP_VERB";
        case HTTP_E_STATUS_BAD_REQUEST: return L"HTTP_E_STATUS_BAD_REQUEST";
        case HTTP_E_STATUS_DENIED: return L"HTTP_E_STATUS_DENIED";
        case HTTP_E_STATUS_PAYMENT_REQ: return L"HTTP_E_STATUS_PAYMENT_REQ";
        case HTTP_E_STATUS_FORBIDDEN: return L"HTTP_E_STATUS_FORBIDDEN";
        case HTTP_E_STATUS_NOT_FOUND: return L"HTTP_E_STATUS_NOT_FOUND";
        case HTTP_E_STATUS_BAD_METHOD: return L"HTTP_E_STATUS_BAD_METHOD";
        case HTTP_E_STATUS_NONE_ACCEPTABLE: return L"HTTP_E_STATUS_NONE_ACCEPTABLE";
        case HTTP_E_STATUS_PROXY_AUTH_REQ: return L"HTTP_E_STATUS_PROXY_AUTH_REQ";
        case HTTP_E_STATUS_REQUEST_TIMEOUT: return L"HTTP_E_STATUS_REQUEST_TIMEOUT";
        case HTTP_E_STATUS_CONFLICT: return L"HTTP_E_STATUS_CONFLICT";
        case HTTP_E_STATUS_GONE: return L"HTTP_E_STATUS_GONE";
        case HTTP_E_STATUS_LENGTH_REQUIRED: return L"HTTP_E_STATUS_LENGTH_REQUIRED";
        case HTTP_E_STATUS_PRECOND_FAILED: return L"HTTP_E_STATUS_PRECOND_FAILED";
        case HTTP_E_STATUS_REQUEST_TOO_LARGE: return L"HTTP_E_STATUS_REQUEST_TOO_LARGE";
        case HTTP_E_STATUS_URI_TOO_LONG: return L"HTTP_E_STATUS_URI_TOO_LONG";
        case HTTP_E_STATUS_UNSUPPORTED_MEDIA: return L"HTTP_E_STATUS_UNSUPPORTED_MEDIA";
        case HTTP_E_STATUS_RANGE_NOT_SATISFIABLE: return L"HTTP_E_STATUS_RANGE_NOT_SATISFIABLE";
        case HTTP_E_STATUS_EXPECTATION_FAILED: return L"HTTP_E_STATUS_EXPECTATION_FAILED";
        case HTTP_E_STATUS_SERVER_ERROR: return L"HTTP_E_STATUS_SERVER_ERROR";
        case HTTP_E_STATUS_NOT_SUPPORTED: return L"HTTP_E_STATUS_NOT_SUPPORTED";
        case HTTP_E_STATUS_BAD_GATEWAY: return L"HTTP_E_STATUS_BAD_GATEWAY";
        case HTTP_E_STATUS_SERVICE_UNAVAIL: return L"HTTP_E_STATUS_SERVICE_UNAVAIL";
        case HTTP_E_STATUS_GATEWAY_TIMEOUT: return L"HTTP_E_STATUS_GATEWAY_TIMEOUT";
        case HTTP_E_STATUS_VERSION_NOT_SUP: return L"HTTP_E_STATUS_VERSION_NOT_SUP";

        // WinINet specific errors
        case INET_E_INVALID_URL: return L"INET_E_INVALID_URL";
        case INET_E_NO_SESSION: return L"INET_E_NO_SESSION";
        case INET_E_CANNOT_CONNECT: return L"INET_E_CANNOT_CONNECT";
        case INET_E_RESOURCE_NOT_FOUND: return L"INET_E_RESOURCE_NOT_FOUND";
        case INET_E_OBJECT_NOT_FOUND: return L"INET_E_OBJECT_NOT_FOUND";
        case INET_E_DATA_NOT_AVAILABLE: return L"INET_E_DATA_NOT_AVAILABLE";
        case INET_E_DOWNLOAD_FAILURE: return L"INET_E_DOWNLOAD_FAILURE";
        case INET_E_AUTHENTICATION_REQUIRED: return L"INET_E_AUTHENTICATION_REQUIRED";
        case INET_E_NO_VALID_MEDIA: return L"INET_E_NO_VALID_MEDIA";
        case INET_E_CONNECTION_TIMEOUT: return L"INET_E_CONNECTION_TIMEOUT";
        case INET_E_INVALID_REQUEST: return L"INET_E_INVALID_REQUEST";
        case INET_E_UNKNOWN_PROTOCOL: return L"INET_E_UNKNOWN_PROTOCOL";
        case INET_E_SECURITY_PROBLEM: return L"INET_E_SECURITY_PROBLEM";
        case INET_E_CANNOT_LOAD_DATA: return L"INET_E_CANNOT_LOAD_DATA";
        case INET_E_CANNOT_INSTANTIATE_OBJECT: return L"INET_E_CANNOT_INSTANTIATE_OBJECT";
        case INET_E_INVALID_CERTIFICATE: return L"INET_E_INVALID_CERTIFICATE";
        case INET_E_REDIRECT_FAILED: return L"INET_E_REDIRECT_FAILED";
        case INET_E_REDIRECT_TO_DIR: return L"INET_E_REDIRECT_TO_DIR";
    }

    return L"";
}
