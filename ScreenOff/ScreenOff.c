// ScrennOff.c : Defines the entry point for the application.

#include "ScreenOff.h"
#include <Shellapi.h>
#include <stdio.h>
#include "MyEditBox.h"

#pragma warning(disable: 4996)

#define MAX_LOADSTRING  100
#define WM_TRAY         (WM_USER + 100)
#define WIN_WIDTH       430
#define WIN_HEIGHT      170
#define BUTTON_WIDTH    140

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT    0x4000
#endif

#ifdef _DEBUG
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

struct tag_GlobalData {
  BOOL bLock;
  struct tag_HotKey hotKey;
};

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HMENU hMenu;
NOTIFYICONDATA nid;
HWND hEdit, hBtnLock;
struct tag_GlobalData g_Data;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void  InitTray(HINSTANCE hInstance, HWND hWnd);
void  ShowTrayMsg();
void  SetMainWinCen(HWND hWnd);
int   MyRegisterHotKey(HWND h, struct tag_HotKey* pkey);
void  ReadData();
void  SaveData();

int APIENTRY _tWinMain(HINSTANCE hInstance,
           HINSTANCE hPrevInstance,
           LPTSTR    lpCmdLine,
           int       nCmdShow) {
  MSG msg;
  HACCEL hAccelTable;
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);  

  // Initialize global strings
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadString(hInstance, IDC_SCRENNOFF, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);
  MyRegisterClassEdit(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, nCmdShow)) {
    return FALSE;
  }

  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCRENNOFF));

  // Main message loop:
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style          = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc    = WndProc;
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hInstance      = hInstance;
  wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCRENNOFF));
  wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
  wcex.lpszMenuName   = NULL;//MAKEINTRESOURCE(IDC_SCRENNOFF);
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SCRENNOFF));

  return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  HWND hWnd;

  hInst = hInstance; // Store instance handle in our global variable

  hWnd = CreateWindow(szWindowClass, szTitle, WS_CAPTION|WS_SYSMENU,
    0, 0, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

  if (!hWnd) {
    return FALSE;
  }

  InitTray(hInstance, hWnd);

#ifdef _DEBUG
  ShowWindow(hWnd, nCmdShow);
#else
  ShowWindow(hWnd, SW_HIDE);
#endif
  UpdateWindow(hWnd);

  return TRUE;
}

LPTSTR get_cmd_line(void) {
  LPTSTR p;

  p = GetCommandLine();
  if(TEXT('\"') == *p) {
    do {
      p++;
    } while(TEXT('\"')!= *p);
    p++;
  } else {
    while(TEXT(' ')!= *p) {
      p++;
    }
  }

  while(TEXT(' ') == *p) {
    p++;
  }
  if(TEXT('\"') == *p) {
    LPTSTR q;
    p++;
    q = p; 
    while(TEXT('\"') != *q) {
      q++;
    } 
    *q=0;
  }
  return p;
}

int main(void) {
  HINSTANCE   hInstance;
  LPTSTR      lpCmdLine;
  int         ret;

  hInstance = GetModuleHandle(NULL);
  lpCmdLine = get_cmd_line();
  ret = _tWinMain(hInstance,0,lpCmdLine,1);
  ExitProcess(ret);
  return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  static int cxChar,cyChar;
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;
  long lTmp;

  switch (message) {
    case WM_CREATE:
      lTmp = GetDialogBaseUnits();
      cxChar = LOWORD (lTmp);
      cyChar = HIWORD (lTmp);
    
      SetMainWinCen(hWnd);

      ReadData();

      MyRegisterHotKey(hWnd, &g_Data.hotKey);

      CreateWindow(TEXT("Static"), TEXT("Hot Key:"),
        WS_CHILD | WS_VISIBLE,
        15, 50, 100, cyChar, hWnd, (HMENU)IDC_STATIC, hInst, NULL);

      hEdit = CreateWindowEx (WS_EX_STATICEDGE, MY_EDIT_NAME ,NULL,
        WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
        90, 50, WIN_WIDTH-30-90, cyChar+2, hWnd, (HMENU)IDE_IN, hInst, NULL);

      CreateWindow(TEXT("Button"), TEXT("About"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        WIN_WIDTH-BUTTON_WIDTH-30, 15, BUTTON_WIDTH, cyChar+8, hWnd, (HMENU)IDM_ABOUT, hInst, NULL);
      CreateWindow(TEXT("Button"), TEXT("Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
        15, 80, WIN_WIDTH-45, cyChar+8, hWnd, (HMENU)IDB_SAVE, hInst, NULL);
      hBtnLock = CreateWindow(TEXT("Button"), TEXT("Enable Screen Lock"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 
        15, 15, 250, cyChar + 8, hWnd, (HMENU)IDB_LOCK, hInst, NULL);
      break;
    case WM_SHOWWINDOW:
      SendMessage(hBtnLock, BM_SETCHECK, g_Data.bLock?BST_CHECKED:BST_UNCHECKED, 0);
      SendMessage(hEdit, EM_SETHOTKEY, (WPARAM)&g_Data.hotKey, 0);
      break;
    case WM_TRAY:
      switch(lParam) {
        case WM_RBUTTONDOWN: {
          POINT pt;                
          GetCursorPos(&pt);

          SetForegroundWindow(hWnd);

          //EnableMenuItem(hMenu, IDM_SHOW, MF_GRAYED);

          TrackPopupMenu(hMenu, TPM_RIGHTALIGN, pt.x, pt.y, 0, hWnd, NULL);
          /*int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
          if(cmd == IDM_SHOW)
            MessageBox(hWnd, APP_TIP, APP_NAME, MB_OK);
          if(cmd == IDM_EXIT)
            PostMessage(hWnd, WM_DESTROY, NULL, NULL);*/
          break;
        }                    
        case WM_LBUTTONDOWN:
          break;
        case WM_LBUTTONDBLCLK:
          ShowWindow(hWnd, SW_RESTORE);
          break;
      }
      break;
    case WM_COMMAND:
      wmId    = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      // Parse the menu selections:
      switch (wmId) {
        case IDM_ABOUT:
          DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
          break;
        case IDM_EXIT:
          DestroyWindow(hWnd);
          break;
        case ID_SCREEN:
          SendMessage(hWnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
          if (g_Data.bLock) {
            LockWorkStation();
          }
          break;
        case IDM_SHOW:
          ShowWindow(hWnd, SW_RESTORE);
          break;
        case IDB_SAVE: {
          struct tag_HotKey* pKey = (struct tag_HotKey*)SendMessage(hEdit, EM_GETHOTKEY, 0, 0);
          g_Data.bLock = SendMessage(hBtnLock, BM_GETCHECK, 0, 0);

          UnregisterHotKey(hWnd, ID_SCREEN);
          if (MyRegisterHotKey(hWnd, pKey)) {
            MessageBox(hWnd, TEXT("Failed to register hotkey!"),szTitle, MB_OK|MB_ICONWARNING);
            MyRegisterHotKey(hWnd, &g_Data.hotKey);
          } else {
            memcpy(&g_Data.hotKey, pKey, sizeof(struct tag_HotKey));
            SaveData();
            ShowWindow(hWnd, SW_HIDE);
          }
          break;
        }                    
        default:
          return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    case WM_HOTKEY:
      if (wParam == ID_SCREEN) {
        UnregisterHotKey(hWnd, ID_SCREEN);
        while (GetKeyState(VK_CONTROL) < 0 || GetKeyState(VK_MENU) < 0
            || GetKeyState(VK_SHIFT) < 0 || GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_LWIN) < 0) {
          Sleep(100);
        }
        PostMessage(hWnd, WM_COMMAND, wParam, 0);
        MyRegisterHotKey(hWnd, &g_Data.hotKey);
      }
      break;
    case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      break;
    case WM_CLOSE:
      ShowWindow(hWnd, SW_HIDE);
      break;
    case WM_DESTROY:
      UnregisterHotKey(hWnd, ID_SCREEN);
      Shell_NotifyIcon(NIM_DELETE, &nid);
      PostQuitMessage(0);
      break;
    /*case WM_QUERYENDSESSION:
      TRACE("WM_QUERYENDSESSION/n");
      break;*/
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

void InitTray(HINSTANCE hInstance, HWND hWnd) {  
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hWnd;
  nid.uID = IDI_SCRENNOFF;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
  nid.uCallbackMessage = WM_TRAY;
  nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCRENNOFF));
  lstrcpy(nid.szTip, szTitle);
  Shell_NotifyIcon(NIM_ADD, &nid);

  hMenu = CreatePopupMenu();  // Tray Menu
  AppendMenu(hMenu, MF_STRING, IDM_SHOW, TEXT("Show Window"));
  AppendMenu(hMenu, MF_STRING, ID_SCREEN, TEXT("Turn off the Screen"));
  AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("Exit"));
}

// Tray notification
void ShowTrayMsg() {
  lstrcpy(nid.szInfoTitle, szTitle);
  lstrcpy(nid.szInfo, TEXT("Some Message"));
  nid.uTimeout = 1000;
  Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void SetMainWinCen(HWND hWnd) {
  int ScreenWidth, ScreenHeight;
  RECT rect;

  ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
  ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

  GetWindowRect(hWnd, &rect);

  rect.left = (ScreenWidth-rect.right)/2;
  rect.top = (ScreenHeight-rect.bottom)/2;

  SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_NOACTIVATE);
}

int MyRegisterHotKey(HWND hWnd, struct tag_HotKey* pKey) {
  UINT uFlag = 0;

  if (pKey->ctrl) {
    uFlag |= MOD_CONTROL;
  }
  if (pKey->shift) {
    uFlag |= MOD_SHIFT;
  }
  if (pKey->alt) {
    uFlag |= MOD_ALT;
  }
  if (pKey->win) {
    uFlag |= MOD_WIN;
  }

  if (uFlag == 0) {
    return 0;
  }

  uFlag |= MOD_NOREPEAT;
  if(0 == RegisterHotKey(hWnd, ID_SCREEN, uFlag, pKey->key)) {
    if(ERROR_INVALID_FLAGS == GetLastError()) {
      uFlag &= ~MOD_NOREPEAT;
      if(0 == RegisterHotKey(hWnd, ID_SCREEN, uFlag, pKey->key)) {
        return GetLastError(); // ERROR_HOTKEY_ALREADY_REGISTERED
      }
    } else {
      return GetLastError();
    }
  }

  return 0;
}

void ReadData() {
  long fileSize;
  FILE *fp;

  memset(&g_Data, 0, sizeof(g_Data));
  fp = fopen("data", "rb");
  if (NULL == fp) {
    return;
  }

  fseek(fp, 0, SEEK_END);
  fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (fileSize != sizeof(g_Data)) {
    fclose(fp);
    return;
  }

  fread(&g_Data, sizeof(g_Data), 1, fp);
  fclose(fp);
}

void SaveData() {
  FILE *fp = fopen("data", "wb");
  if (NULL == fp) {
    return;
  }

  fwrite(&g_Data, sizeof(g_Data), 1, fp);
  fclose(fp);
}

void SetDialogCenter(HWND hDlg) {
  RECT rect, DRect;
  int w,h;

  GetWindowRect(GetParent(hDlg), &rect);
  GetWindowRect(hDlg, &DRect);
  w = DRect.right - DRect.left;
  h = DRect.bottom - DRect.top;
  MoveWindow (hDlg, 
    (rect.right-rect.left-w)/2 + rect.left, 
    (rect.bottom-rect.top-h)/2 + rect.top, 
    w, h, TRUE);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      SetDialogCenter(hDlg);
      return (INT_PTR)TRUE;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}
