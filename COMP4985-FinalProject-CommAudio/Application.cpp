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

WNDCLASSEX wc;
MSG Msg;
HDC dc;

DWORD applicationType;

// Window and Control Handles
HWND hwnd;

enum applicationType { CLIENT = 2, SERVER };

// Labels
HWND applicationTypeTitle;

// Edit Texts/Static Texts
HWND ipEditText;
HWND portEditText;
HWND inputEditText;

HWND statusLogDisplayText;

// Buttons				Client						Server
HWND btn1;			//  Request for File			Start Listening
HWND btn2;			//	Listen Radio				Start Radio
HWND btn3;			//	Voicechat					Voicechat
HWND btn4;			//	Stream						---


// Begin jeff_unicast global variables
// TODO: consolidate global variables to eliminate redundancy
PCMWAVEFORMAT PCMWaveFmtRecord;
WAVEHDR WaveHeader;
WAVEHDR wh[NUM_BUFFS];
HANDLE waveDataBlock;
HWAVEOUT wo;
linked_list playQueue = linked_list();
LPSOCKET_INFORMATION SInfo;
sockaddr_in peer;

int playBackFlag = 0;
// end jeff_unicast global variables 




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
	initializeWindow(hInst);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	displayGUIControls();

	RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
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
		case IDM_DESTROY: // quit the program
			GlobalFree(SocketInfo);
			PostQuitMessage(0);
			break;
		case ID_CLIENT:		// Switch to Client application
			applicationType = CLIENT;
			changeApplicationType();
			break;
		case ID_SERVER:		// Switch to Server application
			applicationType = SERVER;
			changeApplicationType();
			break;
		case ID_HELP:		// Display help button
			MessageBox(NULL, HELP_MSG, HELP_TITLE, MB_ICONQUESTION | MB_OK);
			break;
		case ID_BTN1:
			if (applicationType == CLIENT)
			{
				// Request for file
				initUnicastRecv();

			}
			else if (applicationType == SERVER)
			{
				// Start listening
				initUnicastSend();
			}
			break;
		case ID_BTN2:
			if (applicationType == CLIENT)
			{
				// Listen Radio
				initMulticastRecv();

			}
			else if (applicationType == SERVER)
			{
				// Start Radio
				initMulticastSend();
			}
			break;
		case ID_BTN3:
			if (applicationType == CLIENT)
			{
				// Voice chat
				//TestOpenInputDevice(hwnd);
			}
			else if (applicationType == SERVER)
			{
				// Voice chat

			}
			break;
		case ID_BTN4:
			if (applicationType == CLIENT)
			{
				// Stream

			}
			break;
		default:
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

	case MM_WOM_DONE: {
		MMRESULT mmr;
		char msg[1024];
		node *n;

		mmr = waveOutUnprepareHeader((HWAVEOUT)wParam, (LPWAVEHDR)lParam, sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR) {
			printf("Error unpreparing header - MMERROR: %d", mmr);
			ExitProcess(4);
		}


		n = playQueue.get_next();
		if (n != NULL) {
			memcpy(wh[playBackFlag].lpData, n->data, n->size);
			wh[playBackFlag].dwBufferLength = n->size;
			linked_list::free_node(n);


			mmr = waveOutPrepareHeader((HWAVEOUT)wParam, &wh[playBackFlag], sizeof(WAVEHDR));
			if (mmr != MMSYSERR_NOERROR) {
				printf("Error preparing header - MMERROR: %d", mmr);
				waveOutGetErrorText(mmr, msg, 1024);
				//printf("playing: %d: %s", play, msg);
				ExitProcess(2);
			}

			mmr = waveOutWrite((HWAVEOUT)wParam, &wh[playBackFlag], sizeof(WAVEHDR));
			if (mmr != MMSYSERR_NOERROR) {
				printf("Error waveoutwrite - MMERROR: %d", mmr);
				waveOutGetErrorText(mmr, msg, 1024);
				//printf("playing: %d: %s", play, msg);
				ExitProcess(3);
			}

			/*if (playBackFlag == 0) {
				playBackFlag = 1;
			}
			else {
				playBackFlag = 0;
			}*/

			playBackFlag++;
			if (playBackFlag > NUM_BUFFS - 1) {
				playBackFlag = 0;
			}
		}


		break;
	}
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

int initializeWindow(HINSTANCE hInstance)
{
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_MYICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);;
	wc.lpszClassName = windowName;
	wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_MYICON), IMAGE_ICON, 16, 16, 0);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		windowName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_LENGTH,
		NULL, NULL, hInstance, NULL);

	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: displayControls()
--
-- DATE: January 19, 2019
--
-- DESIGNER: Kiaan Castillo
--
-- PROGRAMMER: Kiaan Castillo
--
-- INTERFACE: void displayControls()
--
-- RETURNS: void
--
-- NOTES:
-- Creates all of the edit text and button controls.
----------------------------------------------------------------------------------------------------------------------*/
void displayGUIControls()
{
	applicationTypeTitle = CreateWindow("STATIC", APPLICATION_TYPE_LABEL_DEFAULT, WS_VISIBLE | WS_CHILD, X_COORDINATE, Y_COORDINATE, STATICTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);
	CreateWindow("STATIC", STATUS_LOG_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE, Y_COORDINATE + Y_COORDINATE_ADDITION, EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);
	statusLogDisplayText = CreateWindow("STATIC", STATUS_LOG_DEFAULT, WS_VISIBLE | WS_CHILD, X_COORDINATE, Y_COORDINATE + (Y_COORDINATE_ADDITION * 2), STATICTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);

	CreateWindow("STATIC", IP_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE, Y_COORDINATE + (Y_COORDINATE_ADDITION * 5), EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);
	ipEditText = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, X_COORDINATE, Y_COORDINATE + (Y_COORDINATE_ADDITION * 6), EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);

	CreateWindow("STATIC", PORT_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE + (X_COORDINATE_ADDITION * 3.25), Y_COORDINATE + (Y_COORDINATE_ADDITION * 5), EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);
	portEditText = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, X_COORDINATE + (X_COORDINATE_ADDITION * 3.25), Y_COORDINATE + (Y_COORDINATE_ADDITION * 6), EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);

	CreateWindow("STATIC", INPUT_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE + (X_COORDINATE_ADDITION * 6.5), Y_COORDINATE + (Y_COORDINATE_ADDITION * 5), EDITTEXT_LENGTH, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);
	inputEditText = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, X_COORDINATE + (X_COORDINATE_ADDITION * 6.5), Y_COORDINATE + (Y_COORDINATE_ADDITION * 6), EDITTEXT_LENGTH * 1.4, EDITTEXT_WIDTH, hwnd, NULL, NULL, NULL);

	btn1 = CreateWindow("BUTTON", BTN_NA_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE, Y_COORDINATE + (Y_COORDINATE_ADDITION * 7.5), BTN_LENGTH, EDITTEXT_WIDTH, hwnd, (HMENU)ID_BTN1, NULL, NULL);
	btn2 = CreateWindow("BUTTON", BTN_NA_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE + (X_COORDINATE_ADDITION * 2.75), Y_COORDINATE + (Y_COORDINATE_ADDITION * 7.5), BTN_LENGTH, EDITTEXT_WIDTH, hwnd, (HMENU)ID_BTN2, NULL, NULL);
	btn3 = CreateWindow("BUTTON", BTN_NA_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE + (X_COORDINATE_ADDITION * 5.5), Y_COORDINATE + (Y_COORDINATE_ADDITION * 7.5), BTN_LENGTH, EDITTEXT_WIDTH, hwnd, (HMENU)ID_BTN3, NULL, NULL);
	btn4 = CreateWindow("BUTTON", BTN_NA_LABEL, WS_VISIBLE | WS_CHILD, X_COORDINATE + (X_COORDINATE_ADDITION * 8.25), Y_COORDINATE + (Y_COORDINATE_ADDITION * 7.5), BTN_LENGTH, EDITTEXT_WIDTH, hwnd, (HMENU)ID_BTN4, NULL, NULL);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: changeMode()
--
-- DATE: January 19, 2019
--
-- DESIGNER: Kiaan Castillo
--
-- PROGRAMMER: Kiaan Castillo
--
-- INTERFACE: void changeMode()
--
-- RETURNS: void
--
-- NOTES:
-- Changes the title and labels on the window depending on the mode chosen.
----------------------------------------------------------------------------------------------------------------------*/
void changeApplicationType()
{
	switch (applicationType)
	{
	case -1:
		SetWindowText(applicationTypeTitle, APPLICATION_TYPE_LABEL_DEFAULT);
		SetWindowText(btn1, BTN_NA_LABEL);
		SetWindowText(btn2, BTN_NA_LABEL);
		SetWindowText(btn3, BTN_NA_LABEL);
		SetWindowText(btn4, BTN_NA_LABEL);
		break;
	case CLIENT:
		SetWindowText(applicationTypeTitle, APPLICATION_TYPE_LABEL_CLIENT);
		SetWindowText(btn1, REQUESTFORFILE_BTN_LABEL);
		SetWindowText(btn2, RADIO_BTN_LABEL_CLIENT);
		SetWindowText(btn3, VOICECHAT_BTN_LABEL);
		SetWindowText(btn4, STREAM_BTN_LABEL);
		break;
	case SERVER:
		SetWindowText(applicationTypeTitle, APPLICATION_TYPE_LABEL_SERVER);
		SetWindowText(btn1, STARTLISTENING_BTN_LABEL);
		SetWindowText(btn2, RADIO_BTN_LABEL_SERVER);
		SetWindowText(btn3, VOICECHAT_BTN_LABEL);
		SetWindowText(btn4, BTN_NA_LABEL);
		break;
	default:
		break;
	}

	updateStatusLogDisplay(STATUS_LOG_CHANGEDAPP);
	RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}

void updateStatusLogDisplay(const CHAR * newStatus)
{
	SetWindowText(statusLogDisplayText, newStatus);
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
	//HWND hwnd = { 0 };      // owner window
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



BOOL OpenWaveFile(char* FileNameAndPath)
{
	MMCKINFO MMCkInfoParent;
	MMCKINFO MMCkInfoChild;
	int errorCode;
	HWAVEOUT hWaveOut;

	// open multimedia file
	HMMIO hmmio = mmioOpen((LPSTR)FileNameAndPath, NULL, MMIO_READ);
	if (!hmmio) {
		printf("mmioOpen error\n");
		return FALSE;
	}

	// initialize WAVE header struct
	MMCkInfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	// enter child chunk
	errorCode = mmioDescend(hmmio, &MMCkInfoParent, NULL, MMIO_FINDRIFF);
	if (errorCode) {
		printf("mmioDescend error\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// initialize "FMT" sub-chunk struct
	MMCkInfoChild.ckid = mmioFOURCC('f', 'm', 't', ' ');

	// enter fmt sub-chunk
	errorCode = mmioDescend(hmmio, &MMCkInfoChild, &MMCkInfoParent, MMIO_FINDCHUNK);
	if (errorCode) {
		printf("mmioDescend Child error\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// read fmt subchunk data into Waveformat structure
	DWORD bytesRead = mmioRead(hmmio, (LPSTR)&PCMWaveFmtRecord, MMCkInfoChild.cksize);
	if (bytesRead <= 0) {
		printf("mmioRead error\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// check if output device can play format
	errorCode = waveOutOpen(&hWaveOut, WAVE_MAPPER, (WAVEFORMATEX*)&PCMWaveFmtRecord, 0l, 0l, WAVE_FORMAT_QUERY);
	if (errorCode) {
		printf("Incompatible WAVE format\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// exit fmt subchunk
	errorCode = mmioAscend(hmmio, &MMCkInfoChild, 0);
	if (errorCode) {
		printf("mmioAescend Child error\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// init data subchunk
	MMCkInfoChild.ckid = mmioFOURCC('d', 'a', 't', 'a');

	// enter data subchunk
	errorCode = mmioDescend(hmmio, &MMCkInfoChild, &MMCkInfoParent, MMIO_FINDCHUNK);
	if (errorCode) {
		printf("mmioDescend Child error\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	// get data size from data subchunk
	long lDataSize = MMCkInfoChild.cksize;
	HANDLE waveDataBlock = ::GlobalAlloc(GMEM_MOVEABLE, lDataSize);
	if (waveDataBlock == NULL) {
		printf("error alloc mem\n");
		mmioClose(hmmio, 0);
		return FALSE;
	}

	LPBYTE pWave = (LPBYTE)::GlobalLock(waveDataBlock);

	if (mmioRead(hmmio, (LPSTR)pWave, lDataSize) != lDataSize) {
		printf("error reading data\n");
		mmioClose(hmmio, 0);
		::GlobalFree(waveDataBlock);
		return FALSE;
	}

	WaveHeader.lpData = (LPSTR)pWave;
	WaveHeader.dwBufferLength = lDataSize;
	WaveHeader.dwFlags = 0L;
	WaveHeader.dwLoops = 0L;

	mmioClose(hmmio, 0);
	return TRUE;

}

VOID printPCMInfo() {
	char pcm[4096];
	sprintf_s(pcm, "PCM Wave Format Record:\n");
	sprintf_s(pcm, "wFormatTag: %d\n", PCMWaveFmtRecord.wf.wFormatTag);
	sprintf_s(pcm, "nChannels: %d\n", PCMWaveFmtRecord.wf.nChannels);
	sprintf_s(pcm, "nSamplesPerSec: %d\n", PCMWaveFmtRecord.wf.nSamplesPerSec);
	sprintf_s(pcm, "nAvgBytesPerSec: %d\n", PCMWaveFmtRecord.wf.nAvgBytesPerSec);
	sprintf_s(pcm, "wBitsPerSample: %d\n\n", PCMWaveFmtRecord.wBitsPerSample);
	OutputDebugStringA(pcm);
}

VOID streamPlayback() {
	MMRESULT mmr;
	node* n;
	char msg[1024];

	for (int i = 0; i < NUM_BUFFS; i++) {
		wh[i].lpData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, PACKET_SIZE);
		n = playQueue.get_next();
		memcpy(wh[i].lpData, n->data, n->size);
		wh[i].dwBufferLength = n->size;
		linked_list::free_node(n);
		mmr = waveOutPrepareHeader(wo, &wh[i], sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR) {
			printf("Error preparing header - MMERROR: %d", mmr);
			waveOutGetErrorText(mmr, msg, 1024);
			ExitProcess(2);
		}

		mmr = waveOutWrite(wo, &wh[i], sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR) {
			printf("Wave out write error - MMERROR: %d", mmr);
			waveOutGetErrorText(mmr, msg, 1024);
			ExitProcess(3);
		}

	}
}

BOOL OpenOutputDevice() {

	//HWAVEOUT wo;
	MMRESULT mmr;
	char err[1024];

	mmr = waveOutOpen(&wo, WAVE_MAPPER, (WAVEFORMATEX*)&PCMWaveFmtRecord, (DWORD)hwnd, 0, CALLBACK_WINDOW);
	if (mmr != MMSYSERR_NOERROR) {
		sprintf_s(err, "Error opening device - MMERROR: %d", mmr);
		OutputDebugStringA(err);
		return (FALSE);
	}

	return (TRUE);
}