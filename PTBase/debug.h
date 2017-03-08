#ifndef DEBUG_H
#define DEBUG_H

#pragma once;

#define DEBUG 1

DLL_API void CreateDebug();
DLL_API void StartDebug();
DLL_API void StopDebug();
DLL_API bool PTDbgStr(LPCSTR lpszFormat, ...);
DLL_API void InitDebug();
DLL_API void UninitDebug();
#endif