#include "stdafx.h"
#include "HTTPConnection.h"

#include <winhttp.h>

namespace
{
	HINTERNET g_hSession = nullptr;

	//Closes a WinHTTP handle when leaving scope
	class ScopedInternetHandle
	{
	public:
		explicit ScopedInternetHandle( HINTERNET _h ) : m_h( _h ) {}
		~ScopedInternetHandle() { if( m_h ) WinHttpCloseHandle( m_h ); }

		operator HINTERNET() const { return m_h; }

	private:
		ScopedInternetHandle( const ScopedInternetHandle& );
		ScopedInternetHandle& operator=( const ScopedInternetHandle& );

		HINTERNET m_h;
	};
}

bool HTTPConnection::InitHTTP()
{
	if( g_hSession )
		return true;

	const wchar_t* userAgent = L"GeoGen/0.1";

	//Automatic proxy (Windows 8.1+), fall back to the default proxy settings on older systems
	g_hSession = WinHttpOpen( userAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

	if( !g_hSession )
		g_hSession = WinHttpOpen( userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

	if( !g_hSession )
		return false;

	//Resolve, connect, send, receive (ms)
	WinHttpSetTimeouts( g_hSession, 10000, 10000, 10000, 20000 );

	return true;
}

void HTTPConnection::ShutDownHTTP()
{
	if( g_hSession )
	{
		WinHttpCloseHandle( g_hSession );
		g_hSession = nullptr;
	}
}

bool HTTPConnection::DownloadFile( const wchar_t* _url, std::vector< byte >& _data, DWORD& _httpStatus )
{
	_data.clear();
	_httpStatus = 0;

	if( !g_hSession )
		return false;

	//Split the URL

	URL_COMPONENTS urlComponents = {};
	urlComponents.dwStructSize = sizeof( urlComponents );
	urlComponents.dwHostNameLength = (DWORD)-1;
	urlComponents.dwUrlPathLength = (DWORD)-1;
	urlComponents.dwExtraInfoLength = (DWORD)-1;

	if( !WinHttpCrackUrl( _url, 0, 0, &urlComponents ) )
		return false;

	std::wstring hostName( urlComponents.lpszHostName, urlComponents.dwHostNameLength );
	std::wstring path( urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength );
	path.append( urlComponents.lpszExtraInfo, urlComponents.dwExtraInfoLength ); //query string

	const bool bSecure = ( urlComponents.nScheme == INTERNET_SCHEME_HTTPS );

	//Send the request

	ScopedInternetHandle hConnection( WinHttpConnect( g_hSession, hostName.c_str(), urlComponents.nPort, 0 ) );

	if( !hConnection )
		return false;

	ScopedInternetHandle hRequest( WinHttpOpenRequest(	hConnection, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
														WINHTTP_DEFAULT_ACCEPT_TYPES, bSecure ? WINHTTP_FLAG_SECURE : 0 ) );

	if( !hRequest )
		return false;

	if(    !WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 )
		|| !WinHttpReceiveResponse( hRequest, nullptr ) )
		return false;

	DWORD statusCode = 0;
	DWORD statusCodeSize = sizeof( statusCode );

	if( !WinHttpQueryHeaders(	hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
								&statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX ) )
		return false;

	_httpStatus = statusCode;

	if( statusCode != 200 )
		return false;

	//Read the body

	for( ;; )
	{
		DWORD available = 0;

		if( !WinHttpQueryDataAvailable( hRequest, &available ) )
		{
			_data.clear();
			return false;
		}

		if( available == 0 )
			break; //done

		const size_t offset = _data.size();
		_data.resize( offset + available );

		DWORD read = 0;

		if( !WinHttpReadData( hRequest, &_data[ offset ], available, &read ) )
		{
			_data.clear();
			return false;
		}

		_data.resize( offset + read );
	}

	return !_data.empty();
}
