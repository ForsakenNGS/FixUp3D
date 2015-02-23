/*
 * PrinterSettings.cpp
 *
 *  Created on: 16.01.2015
 *      Author: Forsaken
 */

#include "PrinterSettings.h"
#include "PrinterIntercept.h"
#include "UpProgram.h"
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
	for (USHORT printSetIndex = 0; printSetIndex < 8; printSetIndex++) {
		ZeroMemory(&settings.customPrintSets[printSetIndex], sizeof(UP_PRINT_SET_STRUCT));
		strcpy(settings.customPrintSets[printSetIndex].set_name, "NOT_SET");
	}
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
	hButtonImport = NULL;
	hButtonExport = NULL;
	hButtonSetTemp = NULL;
	hButtonSendGcode = NULL;
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
			hButtonImport = CreateWindow("Button", "Import config", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 4, 88, 100, 22, hWnd, (HMENU)IDC_BUTTON_IMPORT, hInstDll, 0);
			hButtonExport = CreateWindow("Button", "Export config", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 108, 88, 100, 22, hWnd, (HMENU)IDC_BUTTON_EXPORT, hInstDll, 0);
			hButtonSendGcode = CreateWindow("Button", "Send Gcode", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 212, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_SEND_GCODE, hInstDll, 0);
			hButtonSetTemp = CreateWindow("Button", "Send Temp", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 296, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_SET_TEMP, hInstDll, 0);
			hButtonPrintAgain = CreateWindow("Button", "Print again", WS_CHILD|WS_VISIBLE|WS_TABSTOP, 380, 88, 80, 22, hWnd, (HMENU)IDC_BUTTON_PRINT_AGAIN, hInstDll, 0);
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
					UP_PRINT_SET_STRUCT* printSet = &settings.customPrintSets[iPrintSetIndex];
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
				if (LOWORD(wParam) == IDC_BUTTON_IMPORT) {
					OPENFILENAME	fileOpenStruct;
					ZeroMemory(&fileOpenStruct, sizeof(fileOpenStruct));
					fileOpenStruct.lStructSize = sizeof(fileOpenStruct);
					fileOpenStruct.hwndOwner = hWnd;
					fileOpenStruct.nMaxFile = MAX_PATH;
					fileOpenStruct.lpstrFile = new TCHAR[MAX_PATH];
					fileOpenStruct.lpstrFile[0] = 0;
					fileOpenStruct.lpstrFilter = "Config files\0*.cfg\0";
					fileOpenStruct.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					if (GetOpenFileName(&fileOpenStruct))  {
						readSettingsFromConfig(hWnd, fileOpenStruct.lpstrFile);
					}
					delete[] fileOpenStruct.lpstrFile;
				}
				if (LOWORD(wParam) == IDC_BUTTON_EXPORT) {
					OPENFILENAME	fileOpenStruct;
					ZeroMemory(&fileOpenStruct, sizeof(fileOpenStruct));
					fileOpenStruct.lStructSize = sizeof(fileOpenStruct);
					fileOpenStruct.hwndOwner = hWnd;
					fileOpenStruct.nMaxFile = MAX_PATH;
					fileOpenStruct.lpstrFile = new TCHAR[MAX_PATH];
					fileOpenStruct.lpstrFile[0] = 0;
					fileOpenStruct.lpstrFilter = "Config files\0*.cfg\0";
					fileOpenStruct.Flags = OFN_PATHMUSTEXIST;
					if (GetSaveFileName(&fileOpenStruct))  {
						if (strstr(fileOpenStruct.lpstrFile, ".cfg") == NULL) {
							// Append extension
							sprintf(fileOpenStruct.lpstrFile, "%s%s", fileOpenStruct.lpstrFile, ".cfg");
						}
						writeSettingsToConfig(fileOpenStruct.lpstrFile);
					}
					delete[] fileOpenStruct.lpstrFile;
				}
				if (LOWORD(wParam) == IDC_BUTTON_SEND_GCODE) {
					UpProgram		testProgram(250, 30, 105);
					{
						// Layer 0
						UpProgramLayer& layer1 = testProgram.addLayer(1, 20, 10, 0.0f);
						layer1.jumpToZ(-60.04f, -10000.0f, -10.0f, 10000.0f);
						layer1.jumpToXY(-52.0f, 123.0f, 20.0f, 123.0f);
						layer1.jumpToZ(-61.04f, -10000.0f, 10.0f, 10000.0f);
						layer1.unknown5(500);
						layer1.jumpTo(-32.0f, 123.0f, 80.0f, 123.0f, -61.04f, 10000.0f, 0.0f, 10000.0f);
						layer1.unknown5(1);
						layer1.setParam(FIXUP3D_MEM_PARAM_UNKNOWN11, 1);
						layer1.setParam(FIXUP3D_MEM_PARAM_MOTORS_OFF, 0);
						layer1.setParam(FIXUP3D_MEM_PARAM_TIME_REMAINING, 0);
						layer1.setParam(FIXUP3D_MEM_PARAM_TIME_PERCENT, 100);
					}
					testProgram.writeToPrinter();
					testProgram.clearLayers();
					// TODO: Convert gcode
					/*
					OPENFILENAME	fileOpenStruct;
					ZeroMemory(&fileOpenStruct, sizeof(fileOpenStruct));
					fileOpenStruct.lStructSize = sizeof(fileOpenStruct);
					fileOpenStruct.hwndOwner = hWnd;
					fileOpenStruct.nMaxFile = MAX_PATH;
					fileOpenStruct.lpstrFile = new TCHAR[MAX_PATH];
					fileOpenStruct.lpstrFile[0] = 0;
					fileOpenStruct.lpstrFilter = "GCode files\0*.gcode\0";
					fileOpenStruct.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					if (GetOpenFileName(&fileOpenStruct))  {
						PrinterIntercept::getInstance()->sendGcode(fileOpenStruct.lpstrFile);
					}
					delete[] fileOpenStruct.lpstrFile;
					*/
				}
				if (LOWORD(wParam) == IDC_BUTTON_SET_TEMP) {
					// Manual temperature set requested
					PrinterIntercept::getInstance()->setNozzle1Temp( settings.heaterTemp3 );
				}
				if (LOWORD(wParam) == IDC_BUTTON_PRINT_AGAIN) {
					// Stop the current print job
					PrinterIntercept::getInstance()->printAgain();
				}
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				char tmpInputText[32];
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
							pOverride = &settings.heaterTempOverride2;
							break;
						case IDC_INPUT_HEATER_TEMP3:
							iLayer = 3;
							hWndHeaterTemp = hEditHeaterTemp3;
							pHeaterTemp = &settings.heaterTemp3;
							pOverride = &settings.heaterTempOverride3;
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
			}
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

UP_PRINT_SET_STRUCT* PrinterSettings::getCustomPrintSet(USHORT index) {
	return &settings.customPrintSets[index];
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
			char cHeaterTemp[16];
			sprintf(cHeaterTemp, "%lu", *pHeaterTemp);
			// Update input field
			SetWindowText(hHeaterTempEdit, cHeaterTemp);
		}
	}
	// Set override flag if required
	*pHeaterOverride = (override ? true : *pHeaterOverride);
}

void PrinterSettings::setPreheatTimer(ULONG preheatSeconds) {
	settings.preheatTime = preheatSeconds;
	// Update input field
	char cPreheatMinutes[16];
	sprintf(cPreheatMinutes, "%lu", settings.preheatTime / 30);
	SetWindowText(hEditPreheatTime, cPreheatMinutes);
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
		readSettingsFromConfig(hWnd, sFilename);
	}
}

void PrinterSettings::readSettingsFromConfig(HWND hWnd, char* sFilename) {
	HANDLE hFile = CreateFile(sFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		// Config file does not exist
		//MessageBox(NULL, TEXT("Failed to read configuration (Could not open file)"), TEXT("FixUp3D"), MB_OK);
		return;
	}
	DWORD	dwVersion = 0;
	DWORD	dwBytesRead = 0;
	if (ReadFile(hFile, &dwVersion, sizeof(DWORD), &dwBytesRead, NULL)) {
		if (dwVersion == PRINTER_SETTING_VERSION) {
			if (ReadFile(hFile, &settings, sizeof(settings), &dwBytesRead, NULL)) {
				// Success!
				char cInputText[16];
				sprintf(cInputText, "%lu", settings.heaterTemp1);
				SetWindowTextA(hEditHeaterTemp1, cInputText);
				sprintf(cInputText, "%lu", settings.heaterTemp2);
				SetWindowTextA(hEditHeaterTemp2, cInputText);
				sprintf(cInputText, "%lu", settings.heaterTemp3);
				SetWindowTextA(hEditHeaterTemp3, cInputText);
				sprintf(cInputText, "%lu", settings.preheatTime / 30);
				SetWindowTextA(hEditPreheatTime, cInputText);
				// Printer set
				updatePrintSet(0, &settings.customPrintSets[0]);
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
				char tmpInputText[32];
				stringstream 	stream;
				UP_PRINT_SET_STRUCT* printSet = &settings.customPrintSets[iPrintSetIndex];
				if ((printSet != NULL) && (strcmp(printSet->set_name, "NOT_SET") != 0)) {
					switch( LOWORD(wParam) )
					{
						case IDC_INPUT_NOZZLE_DIAMETER:
							GetWindowTextA(hEditNozzleDiameter, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->nozzle_diameter = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->nozzle_diameter;
								stream << printSet->nozzle_diameter << flush;
								SetWindowTextA(hEditNozzleDiameter, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->nozzle_diameter;
							}
						    InvalidateRect(hEditNozzleDiameter, 0, 1);
							break;
						case IDC_INPUT_LAYER_THICKNESS:
							GetWindowTextA(hEditLayerThickness, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->layer_thickness = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->layer_thickness;
								stream << printSet->layer_thickness << flush;
								SetWindowTextA(hEditLayerThickness, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->layer_thickness;
							}
						    InvalidateRect(hEditLayerThickness, 0, 1);
							break;
						case IDC_INPUT_SCAN_WIDTH:
							GetWindowTextA(hEditScanWidth, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->scan_width = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->scan_width;
								stream << printSet->scan_width << flush;
								SetWindowTextA(hEditScanWidth, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->scan_width;
							}
						    InvalidateRect(hEditScanWidth, 0, 1);
							break;
						case IDC_INPUT_SCAN_TIMES:
							GetWindowTextA(hEditScanTimes, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->scan_times = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->scan_times;
								stream << printSet->scan_times << flush;
								SetWindowTextA(hEditScanTimes, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->scan_times;
							}
						    InvalidateRect(hEditScanTimes, 0, 1);
							break;
						case IDC_INPUT_HATCH_WIDTH:
							GetWindowTextA(hEditHatchWidth, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->hatch_width = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->hatch_width;
								stream << printSet->hatch_width << flush;
								SetWindowTextA(hEditHatchWidth, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->hatch_width;
							}
						    InvalidateRect(hEditHatchWidth, 0, 1);
							break;
						case IDC_INPUT_HATCH_SPACE:
							GetWindowTextA(hEditHatchSpace, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->hatch_space = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->hatch_space;
								stream << printSet->hatch_space << flush;
								SetWindowTextA(hEditHatchSpace, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->hatch_space;
							}
						    InvalidateRect(hEditHatchSpace, 0, 1);
							break;
						case IDC_INPUT_HATCH_LAYER:
							GetWindowTextA(hEditHatchLayer, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->hatch_layer = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->hatch_layer;
								stream << printSet->hatch_layer << flush;
								SetWindowTextA(hEditHatchLayer, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->hatch_layer;
							}
						    InvalidateRect(hEditHatchLayer, 0, 1);
							break;
						case IDC_INPUT_SUPPORT_WIDTH:
							GetWindowTextA(hEditSupportWidth, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->support_width = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->support_width;
								stream << printSet->support_width << flush;
								SetWindowTextA(hEditSupportWidth, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->support_width;
							}
						    InvalidateRect(hEditSupportWidth, 0, 1);
							break;
						case IDC_INPUT_SUPPORT_SPACE:
							GetWindowTextA(hEditSupportSpace, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->support_space = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->support_space;
								stream << printSet->support_space << flush;
								SetWindowTextA(hEditSupportSpace, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->support_space;
							}
						    InvalidateRect(hEditSupportSpace, 0, 1);
							break;
						case IDC_INPUT_SUPPORT_LAYER:
							GetWindowTextA(hEditSupportLayer, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->support_layer = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->support_layer;
								stream << printSet->support_layer << flush;
								SetWindowTextA(hEditSupportLayer, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->support_layer;
							}
						    InvalidateRect(hEditSupportLayer, 0, 1);
							break;
						case IDC_INPUT_SCAN_SPEED:
							GetWindowTextA(hEditScanSpeed, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->scan_speed = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->scan_speed;
								stream << printSet->scan_speed << flush;
								SetWindowTextA(hEditScanSpeed, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->scan_speed;
							}
						    InvalidateRect(hEditScanSpeed, 0, 1);
							break;
						case IDC_INPUT_HATCH_SPEED:
							GetWindowTextA(hEditHatchSpeed, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->hatch_speed = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->hatch_speed;
								stream << printSet->hatch_speed << flush;
								SetWindowTextA(hEditHatchSpeed, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->hatch_speed;
							}
						    InvalidateRect(hEditHatchSpeed, 0, 1);
							break;
						case IDC_INPUT_SUPPORT_SPEED:
							GetWindowTextA(hEditSupportSpeed, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->support_speed = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->support_speed;
								stream << printSet->support_speed << flush;
								SetWindowTextA(hEditSupportSpeed, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->support_speed;
							}
						    InvalidateRect(hEditSupportSpeed, 0, 1);
							break;
						case IDC_INPUT_JUMP_SPEED:
							GetWindowTextA(hEditJumpSpeed, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->jump_speed = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->jump_speed;
								stream << printSet->jump_speed << flush;
								SetWindowTextA(hEditJumpSpeed, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->jump_speed;
							}
						    InvalidateRect(hEditJumpSpeed, 0, 1);
							break;
						case IDC_INPUT_SCAN_SCALE:
							GetWindowTextA(hEditScanScale, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->scan_scale = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->scan_scale;
								stream << printSet->scan_scale << flush;
								SetWindowTextA(hEditScanScale, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->scan_scale;
							}
						    InvalidateRect(hEditScanScale, 0, 1);
							break;
						case IDC_INPUT_HATCH_SCALE:
							GetWindowTextA(hEditHatchScale, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->hatch_scale = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->hatch_scale;
								stream << printSet->hatch_scale << flush;
								SetWindowTextA(hEditHatchScale, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->hatch_scale;
							}
						    InvalidateRect(hEditHatchScale, 0, 1);
							break;
						case IDC_INPUT_SUPPORT_SCALE:
							GetWindowTextA(hEditSupportScale, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->support_scale = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->support_scale;
								stream << printSet->support_scale << flush;
								SetWindowTextA(hEditSupportScale, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->support_scale;
							}
						    InvalidateRect(hEditSupportScale, 0, 1);
							break;
						case IDC_INPUT_FEED_SCALE:
							GetWindowTextA(hEditFeedScale, tmpInputText, 32);
							if (strcmp(tmpInputText, "") == 0) {
								// Empty / Value deleted
								printSet->feed_scale = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true)->feed_scale;
								stream << printSet->feed_scale << flush;
								SetWindowTextA(hEditFeedScale, stream.str().c_str());
							} else {
								// Not empty
								stream << tmpInputText << flush;
								stream >> printSet->feed_scale;
							}
						    InvalidateRect(hEditFeedScale, 0, 1);
							break;
					}
				}
			}
		break;

		case WM_CTLCOLOREDIT:
		{
			HDC		hdc = (HDC)wParam;
			HWND	hWndEdit = (HWND)lParam;
			UP_PRINT_SET_STRUCT* printSetDefault = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex, true);
			if (printSetDefault != NULL) {
				UP_PRINT_SET_STRUCT* printSetCustom = &settings.customPrintSets[iPrintSetIndex];
				if (hWndEdit == hEditNozzleDiameter) {
					SetBkColor(hdc, (printSetCustom->nozzle_diameter != printSetDefault->nozzle_diameter ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditLayerThickness) {
					SetBkColor(hdc, (printSetCustom->layer_thickness != printSetDefault->layer_thickness ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditScanWidth) {
					SetBkColor(hdc, (printSetCustom->scan_width != printSetDefault->scan_width ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditScanTimes) {
					SetBkColor(hdc, (printSetCustom->scan_times != printSetDefault->scan_times ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditHatchWidth) {
					SetBkColor(hdc, (printSetCustom->hatch_width != printSetDefault->hatch_width ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditHatchSpace) {
					SetBkColor(hdc, (printSetCustom->hatch_space != printSetDefault->hatch_space ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditHatchLayer) {
					SetBkColor(hdc, (printSetCustom->hatch_layer != printSetDefault->hatch_layer ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditSupportWidth) {
					SetBkColor(hdc, (printSetCustom->support_width != printSetDefault->support_width ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditSupportSpace) {
					SetBkColor(hdc, (printSetCustom->support_space != printSetDefault->support_space ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditSupportLayer) {
					SetBkColor(hdc, (printSetCustom->support_layer != printSetDefault->support_layer ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditScanSpeed) {
					SetBkColor(hdc, (printSetCustom->scan_speed != printSetDefault->scan_speed ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditHatchSpeed) {
					SetBkColor(hdc, (printSetCustom->hatch_speed != printSetDefault->hatch_speed ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditSupportSpeed) {
					SetBkColor(hdc, (printSetCustom->support_speed != printSetDefault->support_speed ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditJumpSpeed) {
					SetBkColor(hdc, (printSetCustom->jump_speed != printSetDefault->jump_speed ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditScanScale) {
					SetBkColor(hdc, (printSetCustom->scan_scale != printSetDefault->scan_scale ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditHatchScale) {
					SetBkColor(hdc, (printSetCustom->hatch_scale != printSetDefault->hatch_scale ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditSupportScale) {
					SetBkColor(hdc, (printSetCustom->support_scale != printSetDefault->support_scale ? backgroundEditChanged : backgroundEditDefault));
				}
				if (hWndEdit == hEditFeedScale) {
					SetBkColor(hdc, (printSetCustom->feed_scale != printSetDefault->feed_scale ? backgroundEditChanged : backgroundEditDefault));
				}
			}
			return (LRESULT)GetStockObject(HOLLOW_BRUSH);
		}
		break;

		case WM_APP:
		{
			updatePrintSetGUI( lParam );
		}
		break;
	}

    return CallWindowProc(origWndProc, hWnd, message, wParam, lParam);
}

void PrinterSettings::updatePrintSet() {
	if (strcmp(settings.customPrintSets[iPrintSetIndex].set_name, "NOT_SET") != 0) {
		updatePrintSet(iPrintSetIndex, &settings.customPrintSets[iPrintSetIndex]);
	}
}

void PrinterSettings::updatePrintSet(unsigned int index, UP_PRINT_SET_STRUCT* printSet) {
	if (strcmp(settings.customPrintSets[index].set_name, "NOT_SET") == 0) {
		// No custom set defined
		memcpy(&settings.customPrintSets[index], printSet, sizeof(UP_PRINT_SET_STRUCT));
	}
	PostMessage( hTabPrinterSets, WM_APP, 0, index );
}

void PrinterSettings::updatePrintSetGUI(unsigned int index) {

	UP_PRINT_SET_STRUCT* printSet = &settings.customPrintSets[index];
	if (index >= iPrintSetCount) {
		// Create tab
		TCITEM	tabSet;
		tabSet.mask = TCIF_TEXT;
		tabSet.iImage = -1;
		tabSet.pszText = printSet->set_name;
		TabCtrl_InsertItem(hTabPrinterSets, index, &tabSet);
		iPrintSetCount = index + 1;
	} else {
		// Update text
		TCITEM	tabSet;
		TabCtrl_GetItem(hTabPrinterSets, index, &tabSet);
		tabSet.pszText = printSet->set_name;
		TabCtrl_SetItem(hTabPrinterSets, index, &tabSet);
	}

	if (iPrintSetIndex == index) {
		char cTemp[32];
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

    InvalidateRect(hTabPrinterSets, 0, 1);
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
		writeSettingsToConfig(sFilename);
	}
}

void PrinterSettings::writeSettingsToConfig(char* sFilename) {
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

} /* namespace Core */
