//------------------------------------------------------------------------------
// File: DibHelper.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Helper routines for manipulating bitmaps.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>

#include "dibhelper.h"

#include <stdio.h>
#include <assert.h>

/*
void logToFile(char *log_this) {
    FILE *f = fopen("g:\\yo2", "a");
	fprintf(f, log_this);
	fclose(f);
}
*/

void LocalOutput(const char *str, ...)
{
  char buf[2048];
  va_list ptr;
  va_start(ptr,str);
  vsprintf_s(buf,str,ptr);
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");
  // also works: OutputDebugString(L"yo ho2");
  //logToFile(buf);
}

double PCFreq = 0.0;

// call only needed once...
void WarmupCounter()
{
    LARGE_INTEGER li;
	assert(QueryPerformanceFrequency(&li));
    PCFreq = double(li.QuadPart)/1000.0;
}

__int64 StartCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (__int64) li.QuadPart;
}

double GetCounterSinceStartMillis(__int64 sinceThisTime)
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
	assert(PCFreq);
    return double(li.QuadPart-sinceThisTime)/PCFreq;
}
// use like 
// 	__int64 start = StartCounter();
// ...
// __int64 elapsed = GetCounterSinceStartMillis(start)


HBITMAP CopyScreenToBitmap(LPRECT lpRect, BYTE *pData, BITMAPINFO *pHeader)
{
    HDC         hScrDC, hMemDC;         // screen DC and memory DC
    HBITMAP     hBitmap, hOldBitmap;    // handles to deice-dependent bitmaps
    int         nX, nY, nX2, nY2;       // coordinates of rectangle to grab
    int         nWidth, nHeight;        // DIB width and height

    // check for an empty rectangle
    if (IsRectEmpty(lpRect))
      return NULL;

    // create a DC for the screen and create
    // a memory DC compatible to screen DC   
    hScrDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL); // BOO! for aero at least :P
    hMemDC = CreateCompatibleDC(hScrDC);

    // determine points of where to grab from it
    nX  = lpRect->left;
    nY  = lpRect->top;
    nX2 = lpRect->right;
    nY2 = lpRect->bottom;

    nWidth  = nX2 - nX;
    nHeight = nY2 - nY;

    // create a bitmap compatible with the screen DC
    hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

    // select new bitmap into memory DC
    hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBitmap);

    // bitblt screen DC to memory DC
    BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

    // select old bitmap back into memory DC and get handle to
    // bitmap of the screen
    hBitmap = (HBITMAP) SelectObject(hMemDC, hOldBitmap);

    // Copy the bitmap data into the provided BYTE buffer, in the right format I guess.
	__int64 start = StartCounter();
    GetDIBits(hScrDC, hBitmap, 0, nHeight, pData, pHeader, DIB_RGB_COLORS); // here's probably where we might lose some speed...maybe elsewhere too...also this makes a bitmap for us tho...
	// lodo memcpy?
	LocalOutput("getdibits took %fms ", GetCounterSinceStartMillis(start)); // takes 1.1/3.8ms, but that's with 80fps compared to max 251...but for larger things might make more difference...
    // clean up
    DeleteDC(hScrDC);
    DeleteDC(hMemDC);

    // return handle to the bitmap
    return hBitmap;
}





// some from http://cboard.cprogramming.com/windows-programming/44278-regqueryvalueex.html

// =====================================================================================
HRESULT RegGetDWord(HKEY hKey, LPCTSTR szValueName, DWORD * lpdwResult) {

	// Given a value name and an hKey returns a DWORD from the registry.
	// eg. RegGetDWord(hKey, TEXT("my dword"), &dwMyValue);

	LONG lResult;
	DWORD dwDataSize = sizeof(DWORD);
	DWORD dwType = 0;

	// Check input parameters...
	if (hKey == NULL || lpdwResult == NULL) return E_INVALIDARG;

	// Get dword value from the registry...
	lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE) lpdwResult, &dwDataSize );

	// Check result and make sure the registry value is a DWORD(REG_DWORD)...
	if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);
	else if (dwType != REG_DWORD) return DISP_E_TYPEMISMATCH;

	return NOERROR;
}

DWORD read_config_setting(LPCTSTR szValueName) {
  
  HKEY hKey;
  LONG i;
    
    i = RegOpenKeyEx(HKEY_CURRENT_USER,
       L"SOFTWARE\\os_screen_capture",  0, KEY_READ, &hKey);
    
    if ( i != ERROR_SUCCESS)
    {
        return 0; // zero means not set...
    } else {
      
	DWORD dwVal;

	HRESULT hr = RegGetDWord(hKey, szValueName, &dwVal);
    RegCloseKey(hKey); // done with that
	if (FAILED(hr)) {
      return 0;
    } else {
      return dwVal; // if "the setter" sets it to 0, that is also interpreted as not set...see README
    }
}
 
}


