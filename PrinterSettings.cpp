/*
 * PrinterSettings.cpp
 *
 *  Created on: 16.01.2015
 *      Author: Forsaken
 */

#include "PrinterSettings.h"
#include "PrinterIntercept.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <direct.h>
#include <Shlobj.h>

using namespace std;

namespace Core {

PrinterSettings* PrinterSettings::instance = 0;

COLORREF  backgroundEditChanged = RGB(255,255,128);
COLORREF  backgroundEditDefault = RGB(128,255,128);

PrinterSettings* PrinterSettings::getInstance() {
	if (instance == 0) {
		// TODO: Proper exception
		throw "Tried to get printer settings (without hInstance) before initializing!";
		return 0;
	}
	return instance;
}

PrinterSettings* PrinterSettings::getInstanceNew(HINSTANCE hInstance) {
	if (instance == 0) {
		instance = new PrinterSettings(hInstance);
	}
	return instance;
}

PrinterSettings::PrinterSettings(HINSTANCE hInstance) {
	hInstDll = hInstance;
	hWindow = NULL;
	hUsbInterface = NULL;
	lPreheatTimer = 0;
	iPrintSetIndex = 0;
	iPrintSetCount = 0;
	// Current setup
	settings.heaterTemp1 = 0;
	settings.heaterTemp2 = 0;
	settings.heaterTemp3 = 0;
	settings.heaterTempOverride1 = false;
	settings.heaterTempOverride2 = false;
	settings.heaterTempOverride3 = false;
	// Window elements
	hLabelHeaterTemp = NULL;
	hLabelHeaterTemp1 = NULL;
	hLabelHeaterTemp2 = NULL;
	hLabelHeaterTemp3 = NULL;
	hEditHeaterTemp1 = NULL;
	hEditHeaterTemp2 = NULL;
	hEditHeaterTemp3 = NULL;
	hLabelPreheatTime = NULL;
	hEditPreheatTime = NULL;
	hButtonSetTemp = NULL;
	hButtonStopPrint = NULL;
	hButtonPrintAgain = NULL;
	// - Print set tabs
	hTabPrinterSets = NULL;
	hLabelNozzleDiameter = NULL;
	hEditNozzleDiameter = NULL;
	hLabelLayerThickness = NULL;
	hEditLayerThickness = NULL;
	hLabelScanWidth = NULL;
	hEditScanWidth = NULL;
	hLabelScanTimes = NULL;
	hEditScanTimes = NULL;
	hLabelHatchWidth = NULL;
	hEditHatchWidth = NULL;
	hLabelHatchSpace = NULL;
	hEditHatchSpace = NULL;
	hLabelHatchLayer = NULL;
	hEditHatchLayer = NULL;
	hLabelSupportWidth = NULL;
	hEditSupportWidth = NULL;
	hLabelSupportSpace = NULL;
	hEditSupportSpace = NULL;
	hLabelSupportLayer = NULL;
	hEditSupportLayer = NULL;
	hLabelScanSpeed = NULL;
	hEditScanSpeed = NULL;
	hLabelHatchSpeed = NULL;
	hEditHatchSpeed = NULL;
	hLabelSupportSpeed = NULL;
	hEditSupportSpeed = NULL;
	hLabelJumpSpeed = NULL;
	hEditJumpSpeed = NULL;
	hLabelScanScale = NULL;
	hEditScanScale = NULL;
	hLabelHatchScale = NULL;
	hEditHatchScale = NULL;
	hLabelSupportScale = NULL;
	hEditSupportScale = NULL;
	hLabelFeedScale = NULL;
	hEditFeedScale = NULL;

	origWndProc = NULL;
}

PrinterSettings::~PrinterSettings() {
	// Write config file
	writeSettingsToConfig();
}

LRESULT PrinterSettings::handleWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
		case WM_CREATE:
		{
			// Create elements
			hLabelHeaterTemp = CreateWindow("Static", "Heater Temp. (°C)", WS_CHILD|WS_VISIBLE, 4, 32, 160, 22, hWnd, (HMENU)IDC_LABEL_HEATER_TEMP, hInstDll, 0);
			hLabelHeaterTemp1 = CreateWindow("Static", "Layer 1", WS_CHILD|WS_VISIBLE, 172, 4, 80, 22, hWnd, (HMENU)IDC_LABEL_HEATER_TEMP, hInstDll, 0);
			hLabelHeaterTemp2 = CreateWindow("Static", "Layer 2", WS_CHILD|WS_VISIBLE, 260, 4, 80, 22, hWnd, (HMENU)IDC_LABEL_HEATER_TEMP, hInstDll, 0);
			hLabelHeaterTemp3 = CreateWindow("Static", "Layer 3+", WS_CHILD|WS_VISIBLE, 348, 4, 80, 22, hWnd, (HMENU)IDC_LABEL_HEATER_TEMP, hInstDll, 0);
			hEditHeaterTemp1 = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 172, 32, 80, 22, hWnd, (HMENU)IDC_INPUT_HEATER_TEMP1, hInstDll, 0);
			hEditHeaterTemp2 = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 260, 32, 80, 22, hWnd, (HMENU)IDC_INPUT_HEATER_TEMP2, hInstDll, 0);
			hEditHeaterTemp3 = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 348, 32, 80, 22, hWnd, (HMENU)IDC_INPUT_HEATER_TEMP3, hInstDll, 0);
			hLabelPreheatTime = CreateWindow("Static", "Preheat Timer (Min)", WS_CHILD|WS_VISIBLE, 4, 60, 160, 22, hWnd, (HMENU)IDC_LABEL_PREHEAT_TIME, hInstDll, 0);
			hEditPreheatTime = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 172, 60, 80, 22, hWnd, (HMENU)IDC_INPUT_PREHEAT_TIME, hInstDll, 0);
			hButtonSetTemp = CreateWindow("Button", "Send Temp", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 4, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_SET_TEMP, hInstDll, 0);
			hButtonStopPrint = CreateWindow("Button", "Stop Print", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 88, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_STOP_PRINT, hInstDll, 0);
			hButtonPrintAgain = CreateWindow("Button", "Print again", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 172, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_PRINT_AGAIN, hInstDll, 0);
			hTabPrinterSets = CreateWindow(WC_TABCONTROL, "Print sets", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE|WS_TABSTOP, 0, 120, 476, 292, hWnd, (HMENU)IDC_TAB_PRINTER_SETS, hInstDll, 0);
			origWndProc = (WNDPROC)SetWindowLongPtr( hTabPrinterSets, GWLP_WNDPROC, (LONG_PTR)PrinterSetTabWndProc);

			// Print sets elements
			hLabelNozzleDiameter = CreateWindow("Static", "Nozzle Diameter", WS_CHILD|WS_VISIBLE, 4, 32, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditNozzleDiameter = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 32, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_NOZZLE_DIAMETER, hInstDll, 0);
			hLabelLayerThickness = CreateWindow("Static", "Layer Thickness", WS_CHILD|WS_VISIBLE, 240, 32, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditLayerThickness = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 32, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_LAYER_THICKNESS, hInstDll, 0);

			hLabelScanWidth = CreateWindow("Static", "Scan Width", WS_CHILD|WS_VISIBLE, 4, 56, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 56, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_WIDTH, hInstDll, 0);
			hLabelScanTimes = CreateWindow("Static", "Scan Times", WS_CHILD|WS_VISIBLE, 240, 56, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanTimes = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 56, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_TIMES, hInstDll, 0);

			hLabelHatchWidth = CreateWindow("Static", "Hatch Width", WS_CHILD|WS_VISIBLE, 4, 80, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 80, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_WIDTH, hInstDll, 0);
			hLabelHatchSpace = CreateWindow("Static", "Hatch Space", WS_CHILD|WS_VISIBLE, 240, 80, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchSpace = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 80, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SPACE, hInstDll, 0);

			hLabelHatchLayer = CreateWindow("Static", "Hatch Layer", WS_CHILD|WS_VISIBLE, 4, 104, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchLayer = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 104, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_LAYER, hInstDll, 0);
			hLabelSupportWidth = CreateWindow("Static", "Support Width", WS_CHILD|WS_VISIBLE, 240, 104, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 104, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_WIDTH, hInstDll, 0);

			hLabelSupportSpace = CreateWindow("Static", "Support Space", WS_CHILD|WS_VISIBLE, 4, 128, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportSpace = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 128, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SPACE, hInstDll, 0);
			hLabelSupportLayer = CreateWindow("Static", "Support Layer", WS_CHILD|WS_VISIBLE, 240, 128, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportLayer = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 128, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_LAYER, hInstDll, 0);

			hLabelScanSpeed = CreateWindow("Static", "Scan Speed", WS_CHILD|WS_VISIBLE, 4, 152, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 152, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_SPEED, hInstDll, 0);
			hLabelHatchSpeed = CreateWindow("Static", "Hatch Speed", WS_CHILD|WS_VISIBLE, 240, 152, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 152, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SPEED, hInstDll, 0);

			hLabelSupportSpeed = CreateWindow("Static", "Support Speed", WS_CHILD|WS_VISIBLE, 4, 176, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 176, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SPEED, hInstDll, 0);
			hLabelJumpSpeed = CreateWindow("Static", "Jump Speed", WS_CHILD|WS_VISIBLE, 240, 176, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditJumpSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 176, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_JUMP_SPEED, hInstDll, 0);

			hLabelScanScale = CreateWindow("Static", "Scan Scale", WS_CHILD|WS_VISIBLE, 4, 200, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 200, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_SCALE, hInstDll, 0);
			hLabelHatchScale = CreateWindow("Static", "Hatch Scale", WS_CHILD|WS_VISIBLE, 240, 200, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 200, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SCALE, hInstDll, 0);

			hLabelSupportScale = CreateWindow("Static", "Support Scale", WS_CHILD|WS_VISIBLE, 4, 224, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 156, 224, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SCALE, hInstDll, 0);
			hLabelFeedScale = CreateWindow("Static", "Feed Scale", WS_CHILD|WS_VISIBLE, 240, 224, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditFeedScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP, 384, 224, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_FEED_SCALE, hInstDll, 0);
			// Read config
			readSettingsFromConfig(hWnd);
			break;
		}
		case WM_CTLCOLOREDIT:
		{
			HDC		hdc = (HDC)wParam;
			HWND	hWndEdit = (HWND)lParam;
			if (hWndEdit == hEditHeaterTemp1) {
				SetBkColor(hdc, (settings.heaterTempOverride1 ? backgroundEditChanged : backgroundEditDefault));
			}
			if (hWndEdit == hEditHeaterTemp2) {
				SetBkColor(hdc, (settings.heaterTempOverride2 ? backgroundEditChanged : backgroundEditDefault));
			}
			if (hWndEdit == hEditHeaterTemp3) {
				SetBkColor(hdc, (settings.heaterTempOverride3 ? backgroundEditChanged : backgroundEditDefault));
			}
			if (hWndEdit == hEditPreheatTime) {
				SetBkColor(hdc, (settings.preheatDelay ? backgroundEditChanged : backgroundEditDefault));
			}
		    return (LRESULT)GetStockObject(HOLLOW_BRUSH);
		}
		case WM_NOTIFY:
		{
			LPNMHDR notification = (LPNMHDR)lParam;
			switch (notification->code) {
				case TCN_SELCHANGING:
				{
					// Return FALSE to allow the selection to change.
					return FALSE;
				}
				case TCN_SELCHANGE:
				{
					iPrintSetIndex = TabCtrl_GetCurSel(hTabPrinterSets);
					UP_PRINT_SET_STRUCT* printSet = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex);
					if (printSet != NULL) {
						updatePrintSet(iPrintSetIndex, printSet);
					}
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			/*
			char	debug[64];
			sprintf(debug, "WM_COMMAND: %x (%i / %i), %x (%i / %i)", wParam, HIWORD(wParam), LOWORD(wParam), lParam, HIWORD(lParam), LOWORD(lParam));
			MessageBox(NULL, debug, "FixUp3D", NULL);
			*/
			if (HIWORD(wParam) == BN_CLICKED) {
				if (LOWORD(wParam) == IDC_BUTTON_SET_TEMP) {
					// Manual temperature set requested
					PrinterIntercept::getInstance()->setNozzle1Temp( settings.heaterTemp3 );
				}
				if (LOWORD(wParam) == IDC_BUTTON_STOP_PRINT) {
					// Stop the current print job
					PrinterIntercept::getInstance()->stopPrint();
				}
				if (LOWORD(wParam) == IDC_BUTTON_PRINT_AGAIN) {
					// Stop the current print job
					PrinterIntercept::getInstance()->printAgain();
				}
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				CHAR*			tmpInputText = new CHAR[32];
				stringstream 	stream;
				if ((LOWORD(wParam) == IDC_INPUT_HEATER_TEMP1) || (LOWORD(wParam) == IDC_INPUT_HEATER_TEMP2) || (LOWORD(wParam) == IDC_INPUT_HEATER_TEMP3)) {
					// Heater temp changed
					ULONG			newHeaterTemp = 0;
					USHORT			iLayer = 0;
					HWND			hWndHeaterTemp = NULL;
					WINBOOL*		pOverride = NULL;
					ULONG*			pHeaterTemp = NULL;
					// Get changed edit hwnd
					switch (LOWORD(wParam)) {
						case IDC_INPUT_HEATER_TEMP1:
							iLayer = 1;
							hWndHeaterTemp = hEditHeaterTemp1;
							pHeaterTemp = &settings.heaterTemp1;
							pOverride = &settings.heaterTempOverride1;
							break;
						case IDC_INPUT_HEATER_TEMP2:
							iLayer = 2;
							hWndHeaterTemp = hEditHeaterTemp2;
							pHeaterTemp = &settings.heaterTemp2;
							pOverride = &settings.heaterTempOverride1;
							break;
						case IDC_INPUT_HEATER_TEMP3:
							iLayer = 3;
							hWndHeaterTemp = hEditHeaterTemp3;
							pHeaterTemp = &settings.heaterTemp3;
							pOverride = &settings.heaterTempOverride1;
							break;
					}
					// Get new text
					GetWindowTextA(hWndHeaterTemp, tmpInputText, 32);
					if (strcmp(tmpInputText, "") == 0) {
						// Disable!
						*pOverride = false;
					} else {
						stream << tmpInputText << flush;
						stream >> newHeaterTemp;
						if (newHeaterTemp != *pHeaterTemp) {
							// Temperature changed
							if ((newHeaterTemp > 80) && (newHeaterTemp < 300)) {
								setHeaterTemperature(iLayer, newHeaterTemp);
							} else {
								// Invalid input, disable override if active
								if (*pOverride) {
									*pOverride = false;
								}
							}
						}
					}
					// Redraw input
				    InvalidateRect(hWndHeaterTemp, 0, 1);
				}
				if (LOWORD(wParam) == IDC_INPUT_PREHEAT_TIME) {
					UINT preheatMinutes = 0;
					GetWindowTextA(hEditPreheatTime, tmpInputText, 32);
					if (strcmp(tmpInputText, "") == 0) {
						// Disable!
						settings.preheatTime = 0;
						settings.preheatDelay = false;
					} else {
						stream << tmpInputText << flush;
						stream >> preheatMinutes;
						if (preheatMinutes > 0) {
							settings.preheatTime = preheatMinutes * 30;
							settings.preheatDelay = true;
						} else {
							settings.preheatTime = 0;
							settings.preheatDelay = false;
						}
					}
					// Redraw input
				    InvalidateRect(hEditPreheatTime, 0, 1);
				}
				delete[] tmpInputText;
			}
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

USHORT PrinterSettings::getHeaterTemperature(USHORT layer) {
	switch (layer) {
		case 0:
		case 1:
			return settings.heaterTemp1;
		case 2:
			return settings.heaterTemp2;
		case 3:
		default:
			return settings.heaterTemp3;
	}
	return 0;
}

ULONG PrinterSettings::getPreheatTime() {
	return settings.preheatTime;
}

BOOL PrinterSettings::getPreheatDelayPrint() {
	return settings.preheatDelay;
}

void PrinterSettings::setHWnd(HWND hWnd) {
	hWindow = hWnd;
}

void PrinterSettings::setHeaterTemperature(USHORT layer, USHORT newTemp) {
	setHeaterTemperature(layer, newTemp, true);
}

void PrinterSettings::setHeaterTemperature(USHORT layer, USHORT newTemp, BOOL override) {
	WINBOOL* 	pHeaterOverride = NULL;
	ULONG*		pHeaterTemp = NULL;
	HWND		hHeaterTempEdit = NULL;
	switch (layer) {
		case 0:
		case 1:
			pHeaterTemp = &settings.heaterTemp1;
			pHeaterOverride = &settings.heaterTempOverride1;
			hHeaterTempEdit = hEditHeaterTemp1;
			break;
		case 2:
			pHeaterTemp = &settings.heaterTemp2;
			pHeaterOverride = &settings.heaterTempOverride2;
			hHeaterTempEdit = hEditHeaterTemp2;
			break;
		case 3:
		default:
			pHeaterTemp = &settings.heaterTemp3;
			pHeaterOverride = &settings.heaterTempOverride3;
			hHeaterTempEdit = hEditHeaterTemp3;
			break;
	}
	if (override || !*pHeaterOverride) {
		// No custom temperature is set / Setting new custom temperature
		*pHeaterTemp = newTemp;
		if (!override) {
			CHAR* cHeaterTemp = new CHAR[16];
			sprintf(cHeaterTemp, "%lu", *pHeaterTemp);
			// Update input field
			SetWindowText(hHeaterTempEdit, cHeaterTemp);
			// Cleanup
			delete[] cHeaterTemp;
		}
	}
	// Set override flag if required
	*pHeaterOverride = (override ? true : *pHeaterOverride);
}

void PrinterSettings::setPreheatTimer(ULONG preheatSeconds) {
	settings.preheatTime = preheatSeconds;
	// Update input field
	CHAR* cPreheatMinutes = new CHAR[16];
	sprintf(cPreheatMinutes, "%lu", settings.preheatTime / 30);
	SetWindowText(hEditPreheatTime, cPreheatMinutes);
	delete[] cPreheatMinutes;
}

void PrinterSettings::setUsbHandle(WINUSB_INTERFACE_HANDLE newHandle) {
	hUsbInterface = newHandle;
}

void PrinterSettings::readSettingsFromConfig(HWND hWnd) {
	char	sHomeDir[MAX_PATH];
	char	sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		sprintf(sFilename, "%s\\FixUp3D", sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Open/create config file
		sprintf(sFilename, "%s\\FixUp3D\\config.cfg", sHomeDir);
		HANDLE hFile = CreateFile(sFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			// Config file does not exist
			MessageBox(NULL, TEXT("Failed to read configuration (Could not open file)"), TEXT("FixUp3D"), MB_OK);
			return;
		}
		DWORD	dwVersion = 0;
		DWORD	dwBytesRead = 0;
		if (ReadFile(hFile, &dwVersion, sizeof(DWORD), &dwBytesRead, NULL)) {
			if (dwVersion == PRINTER_SETTING_VERSION) {
				if (ReadFile(hFile, &settings, sizeof(settings), &dwBytesRead, NULL)) {
					// Success!
					CHAR* cInputText = new CHAR[16];
					sprintf(cInputText, "%lu", settings.heaterTemp1);
					SetWindowTextA(hEditHeaterTemp1, cInputText);
					sprintf(cInputText, "%lu", settings.heaterTemp2);
					SetWindowTextA(hEditHeaterTemp2, cInputText);
					sprintf(cInputText, "%lu", settings.heaterTemp3);
					SetWindowTextA(hEditHeaterTemp3, cInputText);
					sprintf(cInputText, "%lu", settings.preheatTime / 30);
					SetWindowTextA(hEditPreheatTime, cInputText);
					// Redraw
				    InvalidateRect(hEditHeaterTemp1, 0, 1);
				    InvalidateRect(hEditHeaterTemp2, 0, 1);
				    InvalidateRect(hEditHeaterTemp3, 0, 1);
				    InvalidateRect(hEditPreheatTime, 0, 1);
				} else {
					// TODO: Error
				}
			} else {
				// TODO: Conversion or Error
			}
		}
		CloseHandle(hFile);
	}
}

void PrinterSettings::resetHeaterTemperature() {
	settings.heaterTempOverride1 = false;
	settings.heaterTempOverride2 = false;
	settings.heaterTempOverride3 = false;
}

void PrinterSettings::updatePreheatTimer(ULONG newTime) {
	lPreheatTimer = newTime;
}

LRESULT CALLBACK PrinterSettings::PrinterSetTabWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return Core::PrinterSettings::getInstance()->handlePrintSetTabWndMessage(hWnd, message, wParam, lParam);
}

LRESULT	PrinterSettings::handlePrintSetTabWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE) {
				CHAR*			tmpInputText = new CHAR[32];
				stringstream 	stream;
				UP_PRINT_SET_STRUCT* printSet = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex);
				if (printSet != NULL) {
					switch( LOWORD(wParam) )
					{
						case IDC_INPUT_NOZZLE_DIAMETER:
							GetWindowTextA(hEditNozzleDiameter, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->nozzle_diameter;
							break;
						case IDC_INPUT_LAYER_THICKNESS:
							GetWindowTextA(hEditLayerThickness, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->layer_thickness;
							break;
						case IDC_INPUT_SCAN_WIDTH:
							GetWindowTextA(hEditScanWidth, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->scan_width;
							break;
						case IDC_INPUT_SCAN_TIMES:
							GetWindowTextA(hEditScanTimes, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->scan_times;
							break;
						case IDC_INPUT_HATCH_WIDTH:
							GetWindowTextA(hEditHatchWidth, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->hatch_width;
							break;
						case IDC_INPUT_HATCH_SPACE:
							GetWindowTextA(hEditHatchSpace, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->hatch_space;
							break;
						case IDC_INPUT_HATCH_LAYER:
							GetWindowTextA(hEditHatchLayer, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->hatch_layer;
							break;
						case IDC_INPUT_SUPPORT_WIDTH:
							GetWindowTextA(hEditSupportWidth, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->support_width;
							break;
						case IDC_INPUT_SUPPORT_SPACE:
							GetWindowTextA(hEditSupportSpace, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->support_space;
							break;
						case IDC_INPUT_SUPPORT_LAYER:
							GetWindowTextA(hEditSupportLayer, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->support_layer;
							break;
						case IDC_INPUT_SCAN_SPEED:
							GetWindowTextA(hEditScanSpeed, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->scan_speed;
							break;
						case IDC_INPUT_HATCH_SPEED:
							GetWindowTextA(hEditHatchSpeed, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->hatch_speed;
							break;
						case IDC_INPUT_SUPPORT_SPEED:
							GetWindowTextA(hEditSupportSpeed, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->support_speed;
							break;
						case IDC_INPUT_JUMP_SPEED:
							GetWindowTextA(hEditJumpSpeed, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->jump_speed;
							break;
						case IDC_INPUT_SCAN_SCALE:
							GetWindowTextA(hEditScanScale, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->scan_scale;
							break;
						case IDC_INPUT_HATCH_SCALE:
							GetWindowTextA(hEditHatchScale, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->hatch_scale;
							break;
						case IDC_INPUT_SUPPORT_SCALE:
							GetWindowTextA(hEditSupportScale, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->support_scale;
							break;
						case IDC_INPUT_FEED_SCALE:
							GetWindowTextA(hEditFeedScale, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->feed_scale;
							break;
					}
				}
			}
			break;
	}

    return CallWindowProc(origWndProc, hWnd, message, wParam, lParam);
}

void PrinterSettings::updatePrintSet(unsigned int index, UP_PRINT_SET_STRUCT* printSet) {
	CHAR* cTemp = new CHAR[32];
	if (index >= iPrintSetCount) {
		TCITEM	tabSet;
		tabSet.mask = TCIF_TEXT;
		tabSet.iImage = -1;
		tabSet.pszText = printSet->set_name;
		TabCtrl_InsertItem(hTabPrinterSets, index, &tabSet);
		iPrintSetCount = index + 1;
	}
	if (iPrintSetIndex == index) {
		// Update input fields
		sprintf(cTemp, "%f", printSet->nozzle_diameter);
		SetWindowTextA(hEditNozzleDiameter, cTemp);
		sprintf(cTemp, "%f", printSet->layer_thickness);
		SetWindowTextA(hEditLayerThickness, cTemp);
		sprintf(cTemp, "%f", printSet->scan_width);
		SetWindowTextA(hEditScanWidth, cTemp);
		sprintf(cTemp, "%f", printSet->scan_times);
		SetWindowTextA(hEditScanTimes, cTemp);
		sprintf(cTemp, "%f", printSet->hatch_width);
		SetWindowTextA(hEditHatchWidth, cTemp);
		sprintf(cTemp, "%f", printSet->hatch_space);
		SetWindowTextA(hEditHatchSpace, cTemp);
		sprintf(cTemp, "%f", printSet->hatch_layer);
		SetWindowTextA(hEditHatchLayer, cTemp);
		sprintf(cTemp, "%f", printSet->support_width);
		SetWindowTextA(hEditSupportWidth, cTemp);
		sprintf(cTemp, "%f", printSet->support_space);
		SetWindowTextA(hEditSupportSpace, cTemp);
		sprintf(cTemp, "%f", printSet->support_layer);
		SetWindowTextA(hEditSupportLayer, cTemp);
		sprintf(cTemp, "%f", printSet->scan_speed);
		SetWindowTextA(hEditScanSpeed, cTemp);
		sprintf(cTemp, "%f", printSet->hatch_speed);
		SetWindowTextA(hEditHatchSpeed, cTemp);
		sprintf(cTemp, "%f", printSet->support_speed);
		SetWindowTextA(hEditSupportSpeed, cTemp);
		sprintf(cTemp, "%f", printSet->jump_speed);
		SetWindowTextA(hEditJumpSpeed, cTemp);
		sprintf(cTemp, "%f", printSet->scan_scale);
		SetWindowTextA(hEditScanScale, cTemp);
		sprintf(cTemp, "%f", printSet->hatch_scale);
		SetWindowTextA(hEditHatchScale, cTemp);
		sprintf(cTemp, "%f", printSet->support_scale);
		SetWindowTextA(hEditSupportScale, cTemp);
		sprintf(cTemp, "%f", printSet->feed_scale);
		SetWindowTextA(hEditFeedScale, cTemp);
	}
	delete[] cTemp;
}

void PrinterSettings::updateWindowTitle() {
	if (hWindow != NULL) {
		stringstream	ssWindowTitle;
		ssWindowTitle << "FixUp3D";
		if (lPreheatTimer > 0) {
			ssWindowTitle << " - Preheating " << (lPreheatTimer / 30) << "Min";
		}
		ssWindowTitle << flush;
		SetWindowTextA(hWindow, ssWindowTitle.str().c_str());
	}
}

void PrinterSettings::writeSettingsToConfig() {
	char	sHomeDir[MAX_PATH];
	char	sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		sprintf(sFilename, "%s\\FixUp3D", sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Open/create config file
		sprintf(sFilename, "%s\\FixUp3D\\config.cfg", sHomeDir);
		HANDLE hFile = CreateFile(sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			MessageBox(NULL, TEXT("Failed to write configuration (Could not create file)"), TEXT("FixUp3D"), MB_OK);
			return;
		}
		DWORD	dwVersion = PRINTER_SETTING_VERSION;
		DWORD	dwBytesWritten = 0;
		if (WriteFile(hFile, &dwVersion, sizeof(DWORD), &dwBytesWritten, NULL)) {
			if (WriteFile(hFile, &settings, sizeof(settings), &dwBytesWritten, NULL)) {
				// Success!
			} else {
				MessageBox(NULL, TEXT("Failed to write configuration"), TEXT("FixUp3D"), MB_OK);
			}
		} else {
			MessageBox(NULL, TEXT("Failed to write configuration"), TEXT("FixUp3D"), MB_OK);
		}
		CloseHandle(hFile);
	}
}

} /* namespace Core */
