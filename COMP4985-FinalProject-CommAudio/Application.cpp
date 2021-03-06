/*------------------------------------------------------------------------------------------------------------------
--	SOURCE FILE:	Application.cpp - The application layer. The main entry point
--										for the program. Sets up the window, WinMain, WndProc,
--										and handles user input and displaying information to screen.
--
--
--	PROGRAM:		Protocol Analysis
--
--
--	FUNCTIONS:		void displayTextToScreen(const char *errorMessage);
--					void displayStatistics(int received, double duration);
--					char* getSendBuffer();
--					void generateTCPSendBufferData(int size);
--					void generateUDPSendBufferData(int size);
--					void openInputDialog();
--					int getPort();
--					char* getHostIP();
--					int getPacketSize();
--					void setPacketSize(int packetSize);
--					int getNumPackets();
--					void setNumPackets(int numPackets);
--					
--
--
--	DATE:			Feb 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


#include "Application.h"
#include "Transport.h"
#include "Session.h"
#include "targetver.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "winmenu.h"
#include <string>


TCHAR Name[] = TEXT("Protocol Analysis");
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#pragma warning (disable: 4096)
HWND hwnd;
LPSOCKET_INFORMATION SocketInfo;
HANDLE ThreadHandle;
DWORD ThreadId;
char *sendBuffer;
int PORT = 0;
char HOSTIP[255];
char filePath[1024];
int PACKETSIZE = 0;
int NUMPACKETS = 0;
bool fileFlag = false;
int fileIndex = 0;


TCHAR numPackets[2][10] = { TEXT("10"), TEXT("100") };
TCHAR packetSizes[5][10] = { TEXT("1024"), TEXT("4096"), TEXT("16384"), TEXT("32768"), TEXT("65536") };



/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		ServerTransferProc
--
--
--	DATE:			Feb 19, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		BOOL CALLBACK HostIPProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT uMsg: The message
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
--
--
--	RETURNS:		Returns boolean only for the purpose of the required prototype.
--
--
--	NOTES:			Processes user input from server transfer settings dialog box.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK ServerTransferProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char portString[10];

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_INIT:
			if (!GetDlgItemTextA(hwndDlg, IDC_PORT, portString, 10)) {
				*portString = 0;
				MessageBoxA(NULL, "Please indicate a port!", "Error", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
			PORT = atoi(portString);
			if (IsDlgButtonChecked(hwndDlg, IDC_TCP_RADIO) == BST_CHECKED) {
				displayTextToScreen("Server - TCP");
				initServer(hwnd, TCP_SELECTED);
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_UDP_RADIO) == BST_CHECKED) {
				displayTextToScreen("Server - UDP");
				initServer(hwnd, UDP_SELECTED);
			}
			else {
				MessageBoxA(NULL, "Please select a protocol!", "Error", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
			

			//fall through
		case IDC_CANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	
	return FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		ClientTransferProc
--
--
--	DATE:			Feb 19, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		BOOL CALLBACK HostIPProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
--							HWND hwndDlg: A handle to the dialog box
--							UINT uMsg: The message
--							WPARAM wParam: Additional message-specific information
--							LPARAM lParam: Additional message-specific information
--
--
--	RETURNS:		Returns boolean only for the purpose of the required prototype.
--
--
--	NOTES:			Processes user input from client transfer settings dialog box.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK ClientTransferProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char portString[10];
	char packetSizeString[10];
	char numPacketsString[10];

	switch (uMsg)
	{

	case WM_INITDIALOG: {
		HWND hwndList = GetDlgItem(hwndDlg, IDC_NUM_PACKETS);
		for (int i = 0; i < ARRAYSIZE(numPackets); i++) {
			int pos = (int)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)numPackets[i]);
			SendMessage(hwndList, CB_SETITEMDATA, pos, (LPARAM)i);
		}
		SendMessage(hwndList, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		hwndList = GetDlgItem(hwndDlg, IDC_PACKET_SIZE);
		for (int i = 0; i < ARRAYSIZE(packetSizes); i++) {
			int pos = (int)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)packetSizes[i]);
			SendMessage(hwndList, CB_SETITEMDATA, pos, (LPARAM)i);
		}
		SendMessage(hwndList, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDC_FILE_SELECT: {
			
			openInputDialog();
			SetDlgItemText(hwndDlg, IDC_FILE_NAME, filePath);
			HWND hwndList = GetDlgItem(hwndDlg, IDC_NUM_PACKETS);
			SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)TEXT(""));
			SendMessage(hwndList, CB_SETCURSEL, -1, 0);
			EnableWindow(hwndList, false);
			hwndList = GetDlgItem(hwndDlg, IDC_PACKET_SIZE);
			SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)TEXT("Use file size"));
			SendMessage(hwndList, CB_SETCURSEL, ARRAYSIZE(packetSizes), 0);
			EnableWindow(hwndList, false);
			fileFlag = true;
			break;
		}
		case IDC_INIT:
			if (!GetDlgItemTextA(hwndDlg, IDC_HOST_IP, HOSTIP, 255)) {
				MessageBoxA(NULL, "Please indicate a host!", "Error", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
			if (!GetDlgItemTextA(hwndDlg, IDC_PORT, portString, 10)) {
				MessageBoxA(NULL, "Please indicate a port!", "Error", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
			PORT = atoi(portString);
			if (IsDlgButtonChecked(hwndDlg, IDC_TCP_RADIO) == BST_CHECKED) {
				initClient(hwnd, TCP_SELECTED);
				displayTextToScreen("Client - TCP");
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_UDP_RADIO) == BST_CHECKED) {
				initClient(hwnd, UDP_SELECTED);
				displayTextToScreen("Client - UDP");
			}
			else {
				MessageBoxA(NULL, "Please select a protocol!", "Error", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
			
			
			if (fileFlag) {
				PACKETSIZE = DATA_BUFSIZE;
				NUMPACKETS = 1;
			}
			else {
				GetDlgItemTextA(hwndDlg, IDC_PACKET_SIZE, packetSizeString, 10);
				PACKETSIZE = atoi(packetSizeString);
				GetDlgItemTextA(hwndDlg, IDC_NUM_PACKETS, numPacketsString, 10);
				NUMPACKETS = atoi(numPacketsString);
			}

			//fall through
		case IDC_CANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}

	return FALSE;
}


/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		WinMain
--
--
--	DATE:			Feb 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy, Microsoft, Aman Abdulla
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPTSTR lspszCmdParam, int nCmdShow)
--							HINSTANCE hInst: The handle to the current application instance
--							HINSTANCE hPrevInstance: The handle to the previous application instance
--							LPTSTR lspszCmdParam: The command line parameters
--							int nCmdShow: A flag that says how the window will be displayed
--
--
--	RETURNS:		int
--
--
--	NOTES:			Sets up application window and registers it with the OS. Runs main messsage loop.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	WSADATA wsaData;
	DWORD Ret;
	WORD wVersionRequested;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	wVersionRequested = MAKEWORD(2, 2);

	if ((Ret = WSAStartup(wVersionRequested, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return 0;
	}


	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		WndProc
--
--
--	DATE:			January 22, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy, Microsoft, Aman Abdulla
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--							HWND hwnd: The handle to the window
--							UINT Message: The message received from the operating system
--							WPARAM wParam: Additional information about the message
--							LPARAM lParam: Additional information about the message
--
--
--	RETURNS:		int
--
--
--	NOTES:			Handles messages sent from Windows to application. Examples include connecting to Comm ports,
--					sending characters when in Connect mode, escaping Connect mode, accesing Help menu, and exiting
--					the program.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	SOCKET Accept;
	//LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes, SendBytes;
	DWORD Flags;
	int iRc;
	char errorMessage[1024];

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_SERVER_TRANSFER:
			DialogBoxA(NULL, MAKEINTRESOURCE(IDD_SERVER_TRANSFER_SETTINGS), hwnd, ServerTransferProc);
			break;
		case IDM_CLIENT_TRANSFER:
			DialogBoxA(NULL, MAKEINTRESOURCE(IDD_CLIENT_TRANSFER_SETTINGS), hwnd, ClientTransferProc);
			break;
	
		case IDM_DESTROY: // quit the program
			GlobalFree(SocketInfo);
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:	// Terminate program
		GlobalFree(SocketInfo);
		PostQuitMessage(0);
		break;
	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam)) {
			char errorMessage[1024];
			sprintf_s(errorMessage, "Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
			OutputDebugStringA(errorMessage);
			//FreeSocketInformation(wParam);
			closesocket(SocketInfo->Socket);
			GlobalFree(SocketInfo);
		}
		else {
			switch (WSAGETSELECTEVENT(lParam)) 
			{
			case FD_ACCEPT:

				// accept connection
				if ((Accept = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
				{
					printf("accept() failed with error %d\n", WSAGetLastError());
					break;
				}

				// Create a socket information structure and populate to associate with the
				// socket for processing I/O.
				//CreateSocketInformation(Accept);
				if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
					sizeof(SOCKET_INFORMATION))) == NULL)
				{
					printf("GlobalAlloc() failed with error %d\n", GetLastError());
					return 0;
				}
				SocketInfo->Socket = Accept;
				SocketInfo->BytesSEND = 0;
				SocketInfo->BytesRECV = 0;


				sprintf_s(errorMessage, "Socket number %d connected\n", Accept);
				OutputDebugStringA(errorMessage);

				// create worker thread
				if ((ThreadHandle = CreateThread(NULL, 0, TCPServerWorkerThread, (LPVOID)SocketInfo, 0, &ThreadId)) == NULL) {
					OutputDebugStringA("CreateThread failed");
					return 0;
				}

				break;
			case FD_CONNECT:
				if ((iRc = WSAGETSELECTERROR(lParam)) == 0) {
					OutputDebugStringA("FD_CONNECT success\n");
				}
				else {
					OutputDebugStringA("FD_CONNECT fail\n");
				}

				if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
					sizeof(SOCKET_INFORMATION))) == NULL)
				{
					printf("GlobalAlloc() failed with error %d\n", GetLastError());
					return 0;
				}
				SocketInfo->Socket = wParam;
				SocketInfo->BytesSEND = 0;
				SocketInfo->BytesRECV = 0;

				// store buffer, transfer metadata into client global variables in application with getters
				generateTCPSendBufferData(PACKETSIZE);

				// init write thread
				if ((ThreadHandle = CreateThread(NULL, 0, TCPClientWorkerThread, (LPVOID)SocketInfo, 0, &ThreadId)) == NULL) {
					OutputDebugStringA("CreateThread failed");
					return 0;
				}

				break;

			case FD_CLOSE:
				OutputDebugStringA("Closing socket\n");
				//FreeSocketInformation(wParam);
				closesocket(SocketInfo->Socket);
				GlobalFree(SocketInfo);
				break;
			}
		}
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		displayTextToScreen
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void displayTextToScreen(const char *message)
--							char *message: The message to print to the screen.
--
--	RETURNS:
--
--
--	NOTES:			Prints message to the screen.
----------------------------------------------------------------------------------------------------------------------*/
void displayTextToScreen(const char *message) {
	HDC hdc;
	hdc = GetDC(hwnd);

	BitBlt(hdc, 0, 0, 600, 400, 0, 0, 0, WHITENESS);

	TextOut(hdc, 0, 0, message, strlen(message));
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		displayStatistics
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void displayStatistics(int received, double duration)
--							int received: number of packets received
--							double duration: transfer time in ms
--
--	RETURNS:
--
--
--	NOTES:			Prints statistics to the screen.
----------------------------------------------------------------------------------------------------------------------*/
void displayStatistics(int received, double duration) {
	char printout[1024];
	HDC hdc;
	hdc = GetDC(hwnd);

	//BitBlt(hdc, 0, 0, 600, 400, 0, 0, 0, WHITENESS);
	
	sprintf_s(printout, "Packet size: %d", PACKETSIZE);
	TextOut(hdc, 0, 32, printout, strlen(printout));
	sprintf_s(printout, "Packets Received: %d", received);
	TextOut(hdc, 0, 64, printout, strlen(printout));
	sprintf_s(printout, "Packets Expected: %d", NUMPACKETS);
	TextOut(hdc, 0, 96, printout, strlen(printout));
	sprintf_s(printout, "Transfer time: %f ms", duration);
	TextOut(hdc, 0, 128, printout, strlen(printout));
	TextOut(hdc, 0, 128, printout, strlen(printout));
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		getSendBuffer
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		char* getSendBuffer() 
--
--	RETURNS:		Pointer to data to send
--
--
--	NOTES:			
----------------------------------------------------------------------------------------------------------------------*/
char* getSendBuffer() {
	return sendBuffer;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		generateTCPSendBufferData
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void generateTCPSendBufferData(int size)
--							int size: size of packet
--
--	RETURNS:		
--
--
--	NOTES:			Sets the send buffer for TCP with data
----------------------------------------------------------------------------------------------------------------------*/
void generateTCPSendBufferData(int size) {
	sendBuffer = (char *)malloc(sizeof(char) * size);
	if (fileFlag) {
		FILE *fp;
		long lSize;

		fopen_s(&fp, filePath, "rb");
		fseek(fp, 0L, SEEK_END);
		lSize = ftell(fp);
		rewind(fp);
		if (fread(sendBuffer, lSize, 1, fp) != 1) {
			exit(1);
		}
		fclose(fp);
	}
	else {
		for (int i = 0; i < size; ++i) {
			*(sendBuffer + i) = 'a';
		}
		*(sendBuffer + size - 3) = '\r';
		*(sendBuffer + size - 2) = '\n';
		*(sendBuffer + size - 1) = '\0';
	}
	
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		generateUDPSendBufferData
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void generateUDPSendBufferData(int size)
--							int size: size of packet
--
--	RETURNS:
--
--
--	NOTES:			Sets the send buffer for UDP with metadata and generated
----------------------------------------------------------------------------------------------------------------------*/
void generateUDPSendBufferData(int size) {
	sendBuffer = (char *)malloc(sizeof(char) * size);
	int index;
	char metaData[15];
	sprintf_s(metaData, "%d %d ", getPacketSize(), getNumPackets());
	for (index = 0; metaData[index] != '\0'; index++) {
		*(sendBuffer + index) = metaData[index];
	}
	for (int i = index; i < size; i++) {
		*(sendBuffer + i) = 'a';
	}
	*(sendBuffer + size - 3) = '\r';
	*(sendBuffer + size - 2) = '\n';
	*(sendBuffer + size - 1) = '\0';
}


/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		openInputDialog
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void openInputDialog()
--
--	RETURNS:
--
--
--	NOTES:			Opens dialog that gets and stores user input for a file path.
----------------------------------------------------------------------------------------------------------------------*/
void openInputDialog() {
	OPENFILENAMEA ofn;       // common dialog box structure
							 // buffer for file name
	HWND hwnd = { 0 };      // owner window
	HANDLE hf;              // file handle
	char buffer[1024];

	LPDWORD numByteRead = 0;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of filePath to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	if (GetOpenFileNameA(&ofn) == TRUE) {
		hf = CreateFileA(ofn.lpstrFile, GENERIC_READ | GENERIC_WRITE, 0,
			(LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		int filesize = GetFileSize(hf, NULL);
		int tmp;
		OutputDebugStringA(filePath);
		CloseHandle(hf);
	}
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		getPort
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		int getPort()
--
--	RETURNS:		Port number
--
--
--	NOTES:			Gets port number specifed by the user.
----------------------------------------------------------------------------------------------------------------------*/
int getPort() {
	return PORT;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		getHostIP
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		char *getHostIP()
--
--	RETURNS:		Host IP number
--
--
--	NOTES:			Gets IP number specifed by the user.
----------------------------------------------------------------------------------------------------------------------*/
char *getHostIP() {
	return &(HOSTIP[0]);
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		getPacketSize
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		int getPacketSize()
--
--	RETURNS:		Packet Size
--
--
--	NOTES:			Gets packet size specifed by the user.
----------------------------------------------------------------------------------------------------------------------*/
int getPacketSize() {
	return PACKETSIZE;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		getNumPackets
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		int getNumPackets()
--
--	RETURNS:		Number of Packets
--
--
--	NOTES:			Gets number of packets specifed by the user.
----------------------------------------------------------------------------------------------------------------------*/
int getNumPackets() {
	return NUMPACKETS;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		setPacketSize
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void setPacketSize(int packetSize)
--							int packetSize: size of packets
--
--	RETURNS:
--
--
--	NOTES:			Sets size of packets.
----------------------------------------------------------------------------------------------------------------------*/
void setPacketSize(int packetSize) {
	PACKETSIZE = packetSize;
}

/*------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		setNumPackets
--
--
--	DATE:			February 13, 2019
--
--
--	REVISIONS:
--
--
--	DESIGNER:		Jeffrey Choy
--
--
--	PROGRAMMER:		Jeffrey Choy
--
--
--	INTERFACE:		void setNumPackets(int numPacks)
--							int numPacks: number of packets
--
--	RETURNS:		
--
--
--	NOTES:			Sets number of packets.
----------------------------------------------------------------------------------------------------------------------*/
void setNumPackets(int numPacks) {
	NUMPACKETS = numPacks;
}
