/*
 * main.cpp
 *
 *  Created on: 13.01.2015
 *      Author: Forsaken
 */

#include "main.h"
#include "dll_funcs.h"
#include "PrinterSettings.h"
#include <iostream>
#include <stdio.h>
#include <tchar.h>

using namespace std;

static const TCHAR szWindowClass[] = TEXT("FixUp3DCLS");
static const TCHAR szTitle[]       = TEXT("FixUp3D");

extern "C" BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved) {
	switch (nReason) {
		case DLL_PROCESS_ATTACH:
		{
			// Initialize original winusb lib
			if( !InitWinUsbWrapper() )
			{
				MessageBox(NULL, TEXT("Failed to load original winusb.dll"), TEXT("FixUp3D"), MB_OK);
				return FALSE;
			}

			// Initialize settings window
			HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initializeSettingsWindow, (LPVOID)hDllHandle, 0, NULL);
			if (hThread == 0) {
				MessageBox(NULL, TEXT("Failed to create window thread"), TEXT("FixUp3D"), MB_OK);
				return FALSE;
			}
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			Core::PrinterSettings	*settings = Core::PrinterSettings::getInstance();
			if (settings != NULL) {
				settings->writeSettingsToConfig();
			}
		}
		break;

		default:
			break;
	}
	return TRUE;
}

LRESULT CALLBACK PrinterDialogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return Core::PrinterSettings::getInstance()->handleWndMessage(hWnd, message, wParam, lParam);
}

int initializeSettingsWindow(HINSTANCE hDllHandle)
{
//	Core::PrinterSettings *settings =
	Core::PrinterSettings::getInstanceNew(hDllHandle);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = PrinterDialogWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hDllHandle;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = 0;
	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, TEXT("Call to RegisterClassEx failed"), TEXT("FixUp3D"), MB_OK);
		return 1;
	}
	HWND hWnd = CreateWindow(szWindowClass, szTitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 348, 78, NULL, NULL, hDllHandle, NULL);
	if (!hWnd) {
		TCHAR error[64];
		_stprintf( error, TEXT("Call to CreateWindow failed (Error %lu)"), GetLastError() );
	    MessageBox(NULL, error, TEXT("FixUp3D"), MB_OK);
	    return 2;
	}

	// Show window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}
