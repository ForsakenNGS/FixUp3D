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
#include <TCHAR.H>

using namespace std;

static TCHAR 			szWindowClass[] = "UpUsbIntercept";
static TCHAR 			szTitle[] = "UpUsbIntercept";

//BOOLEAN WINAPI DllEntryPoint(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved) {
extern "C" BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved) {
	switch (nReason) {
		case DLL_PROCESS_ATTACH:
		{
			// Initialize original winusb lib
			initWinUsbWrapper();
			// Initialize settings window
			HANDLE hThread = CreateThread(0, NULL, (LPTHREAD_START_ROUTINE)initializeSettingsWindow, (LPVOID)hDllHandle, NULL, NULL);
			if (hThread == 0) {
				MessageBox(NULL, "Failed to create window thread!", "UpUsbIntercept", NULL);
				return false;
			}
		}
			break;
		case DLL_PROCESS_DETACH:
		{
			Core::PrinterSettings	*settings = Core::PrinterSettings::getInstance();
			if (settings != NULL) {
				settings->writeSettingsToConfig();
			}
			break;
		}
		default:
			break;
	}
	return true;
}

LRESULT CALLBACK PrinterDialogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return Core::PrinterSettings::getInstance()->handleWndMessage(hWnd, message, wParam, lParam);
}

int initializeSettingsWindow(HINSTANCE hDllHandle) {
	Core::PrinterSettings *settings = Core::PrinterSettings::getInstanceNew(hDllHandle);
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
		MessageBox(NULL, "Call to RegisterClassEx failed!", "UpUsbIntercept", NULL);
		return 1;
	}
	HWND hWnd = CreateWindow(szWindowClass, szTitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 348, 78, NULL, NULL, hDllHandle, NULL);
	if (!hWnd) {
		char	error[64];
		sprintf(error, "Call to CreateWindow failed! (Error %i)", GetLastError());
	    MessageBox(NULL, error, "UpUsbIntercept", NULL);
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
