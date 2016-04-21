#include <iostream>
#include <Windows.h>
using namespace std;

struct MainThreadParam
{
	DWORD StartIp;
	DWORD EndIp;
	DWORD StartPort;
	DWORD EndPort;
	HANDLE hCopyEvent;
};

struct SubThreadParam
{
	DWORD Ip;
	DWORD Port;
	HANDLE hThreadNum;
	HANDLE hParamEvent;
};


//int StartScanner(DWORD StartIp, DWORD StartPort, DWORD EndIp, DWORD EndPort);
DWORD WINAPI PortScanThread(LPVOID LpParam);
BOOL InitPortScan();
BOOL InsertInfo(char *buff);
DWORD WINAPI StartScanner(LPVOID LpParam);