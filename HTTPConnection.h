#pragma once

class HTTPConnection
{
public:
	HTTPConnection() : m_Socket(0), m_bSSL( false ) {}

	static bool InitHTTP();
	static void ShutDownHTTP();
	
	static bool InitSSL();
	static void ShutDownSSL();


	bool ConnectToServer( const char *szServerName, WORD portNum, bool _ssl = false );
	void Disconnect();

	byte* DownloadFile( const char* _url, uint32_t& _fileSize, bool _ssl = false );
	
private:
	SOCKET m_Socket;

	CString m_ServerName;

	bool m_bSSL;
};
