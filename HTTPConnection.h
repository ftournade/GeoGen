#pragma once

//Minimal HTTP/HTTPS client (WinHTTP). All functions are thread-safe once InitHTTP() has been called.
class HTTPConnection
{
public:
	static bool InitHTTP();
	static void ShutDownHTTP();

	//Downloads _url (http:// or https://) into _data.
	//Returns true only for HTTP 200 with a non-empty body. _httpStatus is 0 when no response was received.
	static bool DownloadFile( const wchar_t* _url, std::vector< byte >& _data, DWORD& _httpStatus );
};
