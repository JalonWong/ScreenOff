#pragma once
#include <windows.h>

#define MY_EDIT_NAME TEXT("MyTextEdit")
#define EM_GETHOTKEY WM_USER+101
#define EM_SETHOTKEY WM_USER+102

struct tag_HotKey {
  BOOL ctrl;
  BOOL alt;
  BOOL shift;
  BOOL win;
  int key;
  char str[128];
};

ATOM MyRegisterClassEdit(HINSTANCE hInstance);
