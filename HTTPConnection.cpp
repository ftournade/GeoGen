#include "stdafx.h"
#include "HTTPConnection.h"


#include <string>
using namespace std;


#pragma comment(lib,"ws2_32.lib")


HINSTANCE hInst;
WSADATA g_wsaData;

bool HTTPConnection::InitHTTP()
{
	if( WSAStartup( 0x101, &g_wsaData ) != 0 )
		return false;

	return true;
}

void HTTPConnection::ShutDownHTTP()
{
	WSACleanup();
}

bool HTTPConnection::InitSSL()
{
	/*
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	SSL_load_error_strings();

	if( OpenSSL_add_ssl_algorithms() < 0 )
		return false;
	*/
	return true;
}

void HTTPConnection::ShutDownSSL()
{
}

void ParseUrl( const char* _url, string &serverName, string &filepath, string &filename )
{
	string::size_type n;
	string url = _url;

	if( url.substr( 0, 7 ) == "http://" )
		url.erase( 0, 7 );

	if( url.substr( 0, 8 ) == "https://" )
		url.erase( 0, 8 );

	n = url.find( '/' );
	if( n != string::npos )
	{
		serverName = url.substr( 0, n );
		filepath = url.substr( n );
		n = filepath.rfind( '/' );
		filename = filepath.substr( n + 1 );
	}

	else
	{
		serverName = url;
		filepath = "/";
		filename = "";
	}
}

bool HTTPConnection::ConnectToServer( const char *szServerName, WORD portNum, bool _ssl )
{
	Disconnect();

	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	SOCKET conn;

	conn = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( conn == INVALID_SOCKET )
		return false;

	if( inet_addr( szServerName ) == INADDR_NONE )
	{
		hp = gethostbyname( szServerName );
	}
	else
	{
		addr = inet_addr( szServerName );
		hp = gethostbyaddr( (char*)&addr, sizeof( addr ), AF_INET );
	}

	if( hp == NULL )
	{
		closesocket( conn );
		return false;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons( portNum );
	if( connect( conn, (struct sockaddr*)&server, sizeof( server ) ) )
	{
		closesocket( conn );
		return false;
	}

	int keepAlive = 1;

	if( setsockopt( conn, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(int) ) < 0 ) {
		closesocket( conn );
		return false;
	}

	m_Socket = conn;
	m_bSSL = _ssl;

	return true;
}

void HTTPConnection::Disconnect()
{
	if( m_Socket )
	{
		closesocket( m_Socket );
		m_Socket = 0;
	}

	m_ServerName.Empty();
}


int GetHeaderLength( const char* content )
{
	const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
	const char *findPos;
	int ofset = -1;

	findPos = strstr( content, srchStr1 );
	if( findPos != NULL )
	{
		ofset = findPos - content;
		ofset += strlen( srchStr1 );
	}

	else
	{
		findPos = strstr( content, srchStr2 );
		if( findPos != NULL )
		{
			ofset = findPos - content;
			ofset += strlen( srchStr2 );
		}
	}
	return ofset;
}
/*
byte* ReadUrl2( const char *szUrl, uint32_t &bytesReturnedOut, char **headerOut )
{
	const int bufSize = 256*1024;
	char readBuffer[ bufSize ], sendBuffer[ bufSize ], tmpBuffer[ bufSize ];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	uint32_t totalBytesRead, headerLen;
	int thisReadSize;

	ParseUrl( szUrl, server, filepath, filename );

	///////////// step 1, connect //////////////////////
	conn = ConnectToServer( server.c_str(), 80 );

	///////////// step 2, send GET request /////////////
	sprintf( tmpBuffer, "GET %s HTTP/1.0", filepath.c_str() );
	strcpy( sendBuffer, tmpBuffer );
	strcat( sendBuffer, "\r\n" );
	sprintf( tmpBuffer, "Host: %s", server.c_str() );
	strcat( sendBuffer, tmpBuffer );
	strcat( sendBuffer, "\r\n" );
	strcat( sendBuffer, "\r\n" );
	send( conn, sendBuffer, strlen( sendBuffer ), 0 );

	//    SetWindowText(edit3Hwnd, sendBuffer);
	printf( "Buffer being sent:\n%s", sendBuffer );

	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while( 1 )
	{
		memset( readBuffer, 0, bufSize );
		thisReadSize = recv( conn, readBuffer, bufSize, 0 );

		if( thisReadSize <= 0 )
			break;

		tmpResult = (char*)realloc( tmpResult, thisReadSize + totalBytesRead );

		memcpy( tmpResult + totalBytesRead, readBuffer, thisReadSize );
		totalBytesRead += thisReadSize;
	}

	headerLen = GetHeaderLength( tmpResult );
	uint32_t contenLen = totalBytesRead - headerLen;
	result = new char[ contenLen + 1 ];
	memcpy( result, tmpResult + headerLen, contenLen );
	result[ contenLen ] = 0x0;
	
	char *myTmp = new char[ headerLen + 1 ];
	strncpy( myTmp, tmpResult, headerLen );
	myTmp[ headerLen ] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket( conn );
	return (byte*)result;
}
*/
byte* HTTPConnection::DownloadFile( const char* _url, uint32_t& _fileSize, bool _ssl )
{
	const int bufSize = 64 * 1024;
	char readBuffer[ bufSize ], sendBuffer[ bufSize ], tmpBuffer[ bufSize ];
	char *tmpResult = NULL, *result;
	char *headerBuffer = nullptr;
	uint32_t totalBytesRead, headerLen;
	int thisReadSize;

	string server, filepath, filename;
	ParseUrl( _url, server, filepath, filename );

	if( m_ServerName != server.c_str() )
	{
		if( !ConnectToServer( server.c_str(), 80, _ssl ) )
			return nullptr;

		m_ServerName = server.c_str();
	}

	///////////// step 2, send GET request /////////////
	sprintf( tmpBuffer, "GET %s HTTP/1.0", _url );
	strcpy( sendBuffer, tmpBuffer );
	strcat( sendBuffer, "\r\n" );
	sprintf( tmpBuffer, "Host: %s", server.c_str() );
	strcat( sendBuffer, tmpBuffer );
	strcat( sendBuffer, "\r\n" );
//	strcat( sendBuffer, "Connection: keep-alive\r\n" );
//	strcat( sendBuffer, "\r\n" );
	send( m_Socket, sendBuffer, strlen( sendBuffer ), 0 );


	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while( 1 )
	{
		memset( readBuffer, 0, bufSize );
		thisReadSize = recv( m_Socket, readBuffer, bufSize, 0 );

		if( thisReadSize == 0 )
		{
			break;
		}
		else if( thisReadSize < 0 )
		{
			int wsaError = WSAGetLastError();
/*			LPVOID lpMsgBuf = (LPVOID)"Unknown error";

			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL, wsaError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
							(LPTSTR)&lpMsgBuf, 0, NULL );
				
			LOG_R( "WSA Error: %S\n", lpMsgBuf );
*/
			if( !ConnectToServer( server.c_str(), 80, _ssl ) ) //try reconnecting
			{
				assert( false );
				return nullptr;
			}

			continue;
		}

		tmpResult = (char*)realloc( tmpResult, thisReadSize + totalBytesRead );

		memcpy( tmpResult + totalBytesRead, readBuffer, thisReadSize );
		totalBytesRead += thisReadSize;
	}

	if( !tmpResult )
	{
		Disconnect(); //try reconnecting (comment this line to work in http keep alive mode)
		return nullptr;
	}

	headerLen = GetHeaderLength( tmpResult );
	uint32_t contenLen = totalBytesRead - headerLen;
	result = new char[ contenLen + 1 ];
	memcpy( result, tmpResult + headerLen, contenLen );
	result[ contenLen ] = 0x0;
	/*
	char *myTmp = new char[ headerLen + 1 ];
	strncpy( myTmp, tmpResult, headerLen );
	myTmp[ headerLen ] = NULL;
	delete(tmpResult);
	*/
	_fileSize = contenLen;

	Disconnect();

	return (byte*)result;
}
