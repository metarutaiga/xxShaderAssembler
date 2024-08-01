#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "d3d9.h"
#define strtok_r strtok_s
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
#define DEFINE_GUID(...)
typedef bool BOOL;
typedef float FLOAT;
typedef int16_t SHORT;
typedef int32_t HRESULT;
typedef int32_t INT;
typedef int32_t LONG;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint16_t USHORT;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef uint64_t UINT64;
typedef uint64_t ULONGLONG;
typedef char const* LPCSTR;
typedef void* HANDLE;
typedef void* HWND;
typedef struct { char x[16]; } GUID;
typedef struct { uint64_t v; } LARGE_INTEGER;
#endif
#include "d3d9types.h"
#include "d3d9caps.h"

#if (DIRECT3D_VERSION >= 0x0900)
#define D3DSIO_TEXM3x3DIFF (D3DSIO_TEXM3x3TEX + 1)
#endif

#if __cplusplus
constexpr uint64_t operator ""_u64(char const* text, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i)
        value += uint64_t(uint8_t(text[i])) << (i % 8 * 8);
    return value;
};
#endif
