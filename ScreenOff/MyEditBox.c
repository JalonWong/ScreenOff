#include <stdio.h>
#include "MyEditBox.h"

#pragma warning(disable: 4996)

#ifdef _DEBUG
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

WNDPROC g_EditWndProc;
HINSTANCE hInst;

void SetHotKeyStr(HWND hWnd, struct tag_HotKey* pKey);
LRESULT CALLBACK MyEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterClassEdit(HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  hInst = hInstance;

  GetClassInfoEx(hInstance, TEXT("Edit"), &wcex);

  g_EditWndProc = wcex.lpfnWndProc;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.lpfnWndProc  = MyEditWndProc;
  wcex.lpszClassName  = MY_EDIT_NAME;
  wcex.style      |= CS_DBLCLKS | CS_PARENTDC | CS_GLOBALCLASS;
  wcex.hInstance    = hInstance;    

  return RegisterClassEx(&wcex);
}

LRESULT CALLBACK MyEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  static struct tag_HotKey hotKey;
  LRESULT lRet = 1;
  HWND hParent;

  hParent = GetParent(hWnd);

  switch (message) {
    case WM_KEYDOWN:        
      // TRACE("%xh\n", wParam);
      if (VK_BACK == wParam || VK_ESCAPE == wParam) {
        SetWindowTextA(hWnd, "");
        memset(&hotKey, 0, sizeof(hotKey));
      }
      else if (wParam >= '0' && wParam <= '9' 
            || wParam >= 'A' && wParam <= 'Z'
            || wParam >= VK_F1 && wParam <= VK_F12) {
        hotKey.ctrl = GetKeyState(VK_CONTROL) < 0;
        hotKey.alt = GetKeyState(VK_MENU) < 0;
        hotKey.shift = GetKeyState(VK_SHIFT) < 0;
        hotKey.win = (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_LWIN) < 0);
        hotKey.key = wParam;
        SetHotKeyStr(hWnd, &hotKey);
      }
      break;
    case EM_GETHOTKEY:
      return (LRESULT)&hotKey;
      break;
    case EM_SETHOTKEY: {
      struct tag_HotKey* pKey = (struct tag_HotKey*)wParam;
      memcpy(&hotKey, pKey, sizeof(hotKey));
      SetHotKeyStr(hWnd, &hotKey);
      break;
    }
    default:
      lRet = g_EditWndProc(hWnd, message, wParam, lParam);
      break;
  }
  return lRet;
}

void SetHotKeyStr(HWND hWnd, struct tag_HotKey* pKey) {
  char *p = pKey->str;
  char buf2[8];
  int key = pKey->key;

  TRACE("Ctrl: %d, Alt: %d, Shift: %d, Win: %d\n", pKey->ctrl, pKey->alt, pKey->shift, pKey->win);

  p[0] = 0;
  if (pKey->ctrl) {
    strcat(p, "Ctrl + ");
  }
  if (pKey->shift) {
    strcat(p, "Shift + ");
  }
  if (pKey->alt) {
    strcat(p, "Alt + ");
  }
  if (pKey->win) {
    strcat(p, "Win + ");
  }

  if (p[0] == 0) {
    return;
  }

  if (key >= 'A' && key <= 'Z') {
    buf2[0] = key;
    buf2[1] = 0;
  } else if (key >= '0' && key <= '9') {
    buf2[0] = key;
    buf2[1] = 0;
  } else if (key >= VK_F1 && key <= VK_F24) {
    sprintf(buf2, "F%d", key-VK_F1+1);
  } else {
    return;
  }
  strcat(p, buf2);
  SetWindowTextA(hWnd, p);
}
