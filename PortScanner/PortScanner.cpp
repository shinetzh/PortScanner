
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <CommCtrl.h>			//�ؼ�ͷ�ļ�
#include <Windows.h>
#include "resource.h"
#include "head.h"


/*
@shinetzh
this is a test change in master
*/

HWND hDlg;
SubThreadParam SParam;
MainThreadParam MParam;

int CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	DialogBox(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL,
		DlgProc
		);
	return 0;
	//����һ��ģ̬�ĶԻ���
}

int CALLBACK DlgProc(HWND hDlgMain,
	UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	hDlg = hDlgMain;
	DWORD StartIp;
	DWORD StartPort;
	DWORD EndIp;
	DWORD EndPort;
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDC_BUTTON1:
		{
			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, NULL, NULL);
			SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS1), IPM_GETADDRESS, NULL, (LPARAM)&StartIp);
			SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS2), IPM_GETADDRESS, NULL, (LPARAM)&EndIp);
			StartPort = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			EndPort = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			//MainThreadParam MParam;				//���������û��ʹ���߳�ͬ���Ļ���Ҫʹ��ȫ�ֱ������������½��̵߳�ʱ��
													//����̻߳������������ִ�У���������������ڽ��������̻߳���ô���Ĳ���
			MParam.StartIp = StartIp;
			MParam.EndIp = EndIp;
			MParam.StartPort = StartPort;
			MParam.EndPort = EndPort;
			CreateThread(NULL, 0, StartScanner, NULL, 0, NULL);
		}
		break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
	{
		EndDialog(hDlg, 0);
		DestroyWindow(hDlg);
	}
	break;
	default:
		break;
	}
	return 0;
}

//�����е����߳�
DWORD WINAPI StartScanner(LPVOID LpParam)
{

	HANDLE hParamEvent = CreateEvent(NULL, TRUE, FALSE, NULL);						//�����߳�ͬ���¼�
	HANDLE hThreadNum = CreateSemaphore(NULL, 256, 256, NULL);				//�����߳��������Ƶ��ź���
	SParam.hParamEvent = hParamEvent;
	SParam.hThreadNum = hThreadNum;

	for (DWORD Ip = MParam.StartIp; Ip <= MParam.EndIp; Ip++)
	{
		for (DWORD Port = MParam.StartPort; Port <= MParam.EndPort; Port++)
		{
			//�ȴ��ź���������������ط�������Ѹ�ٴ�����256���̺߳�
			//ÿһ�����߳��е�InsertInfo(str);��Ҫ�����̷߳�����Ϣ��
			//���Ǵ�ʱ���߳�ͣ������ط�����������ִ�У��Ӷ�����������
			//�Ӷ���������ֹͣ���С�
			//����������Ҫ�����´�����һ���߳��������߳������Ŀ��ƣ�
			//������ֱ��ʹ�����߳��������ź����Ŀ���,��������������У��ҽ�ԭ���ĺ���StartScanner,��Ϊ���߳�
			DWORD WaitResult = WaitForSingleObject(hThreadNum, 200);			
			
			if (WaitResult == WAIT_OBJECT_0)
			{
				SParam.Ip = Ip;
				SParam.Port = Port;

				CreateThread(NULL, 0, PortScanThread, &SParam, 0, NULL);
				WaitForSingleObject(hParamEvent, INFINITE);								//�ȴ��¼�������
				ResetEvent(hParamEvent);											//�����¼�Ϊfalse

			}
			else if (WaitResult == WAIT_TIMEOUT)
			{
				Port--;
				continue;
			}
		}

	}
	return 1;
}

//�˿�ɨ�����߳�
DWORD WINAPI PortScanThread(LPVOID LpParam)
{
	SubThreadParam SParam;
	MoveMemory(&SParam, LpParam, sizeof(SParam));
	SetEvent(SParam.hParamEvent);						//�����¼�
	if (!InitPortScan())
	{
		return 0;
	}
	SOCKET Sock;
	SOCKADDR_IN SockAddr;
	Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Sock == INVALID_SOCKET)
	{
		MessageBoxA(NULL, "INVALID_SOCKET", NULL, NULL);
	}
	SockAddr.sin_addr.S_un.S_addr = htonl(SParam.Ip);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(SParam.Port);

	char* Ipchar = inet_ntoa(SockAddr.sin_addr);

	char str[200];
	if (connect(Sock, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == 0)
	{
		sprintf_s(str, "%s:%d-----------���ӳɹ�\n", Ipchar, SParam.Port);
	}
	else
	{
		sprintf_s(str, "%s:%d	����ʧ��\n", Ipchar, SParam.Port);
	}
	
	InsertInfo(str);
	closesocket(Sock);
	ReleaseSemaphore(SParam.hThreadNum, 1, NULL);			//�ͷ�һ���ź���
	return 0;
}

BOOL InitPortScan()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 0;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return 0;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");
	return 1;


}

BOOL InsertInfo(char *buff)
{
	SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, NULL, (LPARAM)buff);
	return TRUE;
}