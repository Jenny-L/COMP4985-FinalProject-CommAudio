#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define TCP_SELECTED 999
#define UDP_SELECTED 998
#define DATA_BUFSIZE 66000
#define WM_SOCKET (WM_USER + 1)


//#define PORT 5150 //TODO: replace with user input

#include "resource.h"
// Windows Header Files
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>





typedef struct _SOCKET_INFORMATION {
	//BOOL RecvPosted;
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
	//_SOCKET_INFORMATION *Next;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct _CLIENT_THREAD_PARAMS {
	LPSOCKET_INFORMATION SI;
	sockaddr_in server;
} CLIENT_THREAD_PARAMS, *LPCLIENT_THREAD_PARAMS;


void displayTextToScreen(const char *errorMessage);
void displayStatistics(int received, double duration);
char* getSendBuffer();
void generateTCPSendBufferData(int size);
void generateUDPSendBufferData(int size);
void openInputDialog();
int getPort();
char* getHostIP();
int getPacketSize();
void setPacketSize(int packetSize);
int getNumPackets();
void setNumPackets(int numPackets);
//void CreateSocketInformation(SOCKET s);
//LPSOCKET_INFORMATION GetSocketInformation(SOCKET s);
//void FreeSocketInformation(SOCKET s);
