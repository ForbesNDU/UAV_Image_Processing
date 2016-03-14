#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

#ifndef WEB_UTILS_H
#define WEB_UTILS_H

bool ping_request(HINTERNET, LPCWSTR);

#endif
