#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define TCP_SELECTED 999
#define UDP_SELECTED 998
#define DATA_BUFSIZE 66000
#define WM_SOCKET (WM_USER + 1)

#define UNICAST_PORT 8080
#define PACKET_SIZE 8192
#define NUM_BUFFS 2
#define UNICAST_ADDRESS "127.0.0.1" // TODO: change to user input

// Windows Header Files
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <Mmsystem.h>
#include "LinkedList.h"
#include "resource.h"

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	SOCKET Socket;
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
	sockaddr_in peer;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct _CLIENT_THREAD_PARAMS {
	SOCKET_INFORMATION SI;
	SOCKET sock;
	sockaddr_in sin;
} CLIENT_THREAD_PARAMS, *LPCLIENT_THREAD_PARAMS;

extern PCMWAVEFORMAT PCMWaveFmtRecord;
extern WAVEHDR WaveHeader;
extern WAVEHDR wh[NUM_BUFFS];
extern HANDLE waveDataBlock;
extern HWAVEOUT wo;
extern linked_list playQueue;
extern LPSOCKET_INFORMATION SInfo;


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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int initializeWindow(HINSTANCE);
void displayGUIControls();
void changeApplicationType();
void updateStatusLogDisplay(const CHAR *);


BOOL OpenWaveFile(char* FileNameAndPath);