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

#include "Log.h"
#include "logging/ConsoleTarget.h"
#include "logging/FileLogger.h"

#include <direct.h>
#include <Shlobj.h>
#include <sys/stat.h>

// 1 MB
#define MAX_LOG_FILE_SIZE		1048576


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

			// Initialize log targets
			TCHAR sHomeDir[MAX_PATH];
			TCHAR sFilename[MAX_PATH];
			if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
				_stprintf(sFilename, TEXT("%s\\UpUsbIntercept"), sHomeDir);
				// Ensure directory exists
				_mkdir(sFilename);
				// Create log writer
				_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\PrinterIntercept.log"), sHomeDir);
				// Check log size
			    struct stat stat_buf;
			    int rc = stat(sFilename, &stat_buf);
			    // Add file target
			    if ((rc != 0) || (stat_buf.st_size > MAX_LOG_FILE_SIZE)) {
			    	// File does not exists or is too big, overwrite
					Log::addTarget("file_default", new Logging::FileLogger(sFilename, ios_base::out, LogLevel::DEBUG, LogSections::SECTION_DEFAULT));
			    } else {
			    	// File exists and is below max size, append
			    	Log::addTarget("file_default", new Logging::FileLogger(sFilename, ios_base::app | ios_base::out, LogLevel::DEBUG, LogSections::SECTION_DEFAULT));
			    }
			    // Raw usb log
				_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\UsbRaw.log"), sHomeDir);
				Log::addTarget("file_usb_raw", new Logging::FileLogger(sFilename, ios_base::out, LogLevel::DEBUG, LogSections::SECTION_USB_RAW));
			}
			// Generic console log
			Log::addTarget("console", new Logging::ConsoleTarget(LogLevel::INFO, LogSections::SECTION_ANY));
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
		MessageBox(NULL, TEXT("Call to RegisterClassEx failed"), TEXT("FixUp3D"), MB_OK);
		return 1;
	}
	HWND hWnd = CreateWindowEx( WS_EX_TOPMOST,
                              szWindowClass, szTitle,
			                  (WS_OVERLAPPED | WS_CAPTION | /*WS_SYSMENU |*/ WS_MINIMIZEBOX),
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  476, 440,
							  NULL,
							  NULL,
							  hDllHandle,
							  NULL );
	if (!hWnd) {
		TCHAR error[64];
		_stprintf( error, TEXT("Call to CreateWindow failed (Error %lu)"), GetLastError() );
	    MessageBox(NULL, error, TEXT("FixUp3D"), MB_OK);
	    return 2;
	}

	// Show window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	settings->setHWnd(hWnd);

	// Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}
