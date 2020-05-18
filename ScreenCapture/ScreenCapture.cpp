// ScreenCapture.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "ScreenCapture.h"


#define MAX_LOADSTRING 100
#define WM_TRAY_ALERT WM_USER + 1
#define MENU_QUIT_MESSAGE 0x103

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

string dataPath = "C:";

HMENU hMenu, hPopupMenu, hMenubar;

HANDLE mutex;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	mutex = CreateMutex(NULL, true, "ScreenCapture");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, "이미 실행중입니다.", "알림", MB_OK);
		CloseHandle(mutex);
		return FALSE;
	}

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	// 애플리케이션 초기화를 수행합니다:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENCAPTURE));

	MSG msg;

	// 기본 메시지 루프입니다:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON2));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	POINT winPos = { 0,0 };
	POINT winSize = { 200,0 };

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU,
		winPos.x, winPos.y, winSize.x, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//ShowWindow(hWnd, nCmdShow);
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	return TRUE;
}

void CreateTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA    m_stData;

	memset(&m_stData, 0x00, sizeof(m_stData));

	m_stData.cbSize = sizeof(NOTIFYICONDATA);
	m_stData.hWnd = hwnd;
	m_stData.uID = 100;                                                         //전달값(WPARAM)
	m_stData.uFlags = NIF_ICON | NIF_MESSAGE;                        //옵션
	m_stData.uCallbackMessage = WM_TRAY_ALERT;                                    //사용자메세지등록
	m_stData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));           //아이콘설정


	Shell_NotifyIcon(NIM_ADD, &m_stData);
}

void DestroyTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof nid);
	nid.cbSize = sizeof nid;
	nid.hWnd = hwnd;
	nid.uID = 100;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ScreenCapture(int winX, int winY)
{
	HDC hdc = GetWindowDC(NULL);

	CImage img;

	int colorDepth = ::GetDeviceCaps(hdc, BITSPIXEL);
	img.Create(winX, winY, colorDepth, 0);

	::BitBlt(img.GetDC(), 0, 0, winX, winY, hdc, 0, 0, SRCCOPY);

	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);

	string path = "";
	path = dataPath;

	if (path[path.length() - 1] != '\\')
		path += '\\';

	path += to_string(systemTime.wYear) + "-" + to_string(systemTime.wMonth) + "-" + to_string(systemTime.wDay)
		+ ' ' + to_string(systemTime.wHour) + to_string(systemTime.wMinute) + to_string(systemTime.wSecond) + ".png";

	img.Save(path.c_str(), Gdiplus::ImageFormatPNG);

	ReleaseDC(NULL, hdc);
	img.ReleaseDC();
}

void SelectFilePath(HWND hWnd)
{
	BROWSEINFO BrInfo;
	TCHAR szBuffer[512]; // 경로저장 버퍼 

	::ZeroMemory(&BrInfo, sizeof(BROWSEINFO));
	::ZeroMemory(szBuffer, 512);

	BrInfo.hwndOwner = hWnd;
	BrInfo.lpszTitle = "파일이 저장될 폴더를 선택하세요";
	BrInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pItemIdList = ::SHBrowseForFolder(&BrInfo);
	::SHGetPathFromIDList(pItemIdList, szBuffer);               // 파일경로 읽어오기

	dataPath = szBuffer;
}

void OnHotKey(HWND hwnd, WPARAM id)
{
	switch (id)
	{
	case 6974:
		ScreenCapture(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		break;
	case 1234:
		SelectFilePath(hwnd);
		break;
	default:
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		RegisterHotKey(hWnd, 6974, 0, VK_F10);
		RegisterHotKey(hWnd, 1234, 0, VK_F9);
		CreateTrayIcon(hWnd);
		SelectFilePath(hWnd);

		hMenu = CreateMenu();
		hMenubar = CreateMenu();
		AppendMenu(hMenu, MF_STRING, MENU_QUIT_MESSAGE, TEXT("종료"));
		AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, TEXT("Menu"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case MENU_QUIT_MESSAGE:
			DestroyTrayIcon(hWnd);
			UnregisterHotKey(hWnd, 6974);
			UnregisterHotKey(hWnd, 1234);
			CloseHandle(mutex);
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	case WM_HOTKEY:
		OnHotKey(hWnd, wParam);
	case WM_TRAY_ALERT:
		if (wParam == 100)
			switch (lParam)
			{
			case WM_RBUTTONUP:
				hPopupMenu = GetSubMenu(hMenubar, 0);
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				SetForegroundWindow(hWnd);
				PostMessage(hWnd, WM_NULL, 0, 0);
				break;
			case WM_LBUTTONDBLCLK:
				SelectFilePath(hWnd);
				break;
			default:
				break;
			}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

