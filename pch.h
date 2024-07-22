#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcomma"
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wextra-tokens"
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Wlogical-not-parentheses"
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wwritable-strings"
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "d3d8.h"
#else
#include <stdint.h>
#define FALSE false
#define TRUE true
#define D3D_OK 0
#define S_OK 0
#define E_FAIL -1
#define SUCCEEDED(x) (x == S_OK)
#define WINAPI
#define HeapAlloc(a,b,c) malloc(c)
#define lstrlen strlen
#define _snprintf snprintf
#define _vsnprintf vsnprintf
typedef bool BOOL;
typedef float FLOAT;
typedef int32_t HRESULT;
typedef int32_t INT;
typedef int32_t LONG;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef char const* LPCSTR;
typedef void* HWND;
typedef struct { char x[16]; } GUID;
#endif
#include "d3d8types.h"
#include "d3d8caps.h"
