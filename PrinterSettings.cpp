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
	iPrintSetIndex = 0;
	iPrintSetCount = 0;
	// Current setup
	settings.heaterTemp = 0;
	settings.heaterTempOverride = false;
	// Window elements
	hLabelHeaterTemp = NULL;
	hEditHeaterTemp = NULL;
	hCheckHeaterTemp = NULL;
	hLabelPreheatTime = NULL;
	hEditPreheatTime = NULL;
	hCheckPreheatTime = NULL;
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
			hLabelHeaterTemp = CreateWindow("Static", "Heater Temp. (°C)", WS_CHILD|WS_VISIBLE, 4, 4, 160, 22, hWnd, (HMENU)IDC_LABEL_HEATER_TEMP, hInstDll, 0);
			hEditHeaterTemp = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 172, 4, 80, 22, hWnd, (HMENU)IDC_INPUT_HEATER_TEMP, hInstDll, 0);
			hCheckHeaterTemp = CreateWindow("Button", "Override", BS_CHECKBOX|WS_CHILD|WS_VISIBLE, 260, 4, 128, 22, hWnd, (HMENU)IDC_CHECK_HEATER_TEMP, hInstDll, 0);
			hLabelPreheatTime = CreateWindow("Static", "Preheat Timer (Min)", WS_CHILD|WS_VISIBLE, 4, 32, 160, 22, hWnd, (HMENU)IDC_LABEL_PREHEAT_TIME, hInstDll, 0);
			hEditPreheatTime = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 172, 32, 80, 22, hWnd, (HMENU)IDC_INPUT_PREHEAT_TIME, hInstDll, 0);
			hCheckPreheatTime = CreateWindow("Button", "Delay print job", BS_CHECKBOX|WS_CHILD|WS_VISIBLE, 260, 32, 128, 22, hWnd, (HMENU)IDC_CHECK_PREHEAT_TIME, hInstDll, 0);
			hButtonSetTemp = CreateWindow("Button", "Send Temp", WS_CHILD|WS_VISIBLE, 4, 60, 80, 22, hWnd, (HMENU)IDC_BUTTON_SET_TEMP, hInstDll, 0);
			hButtonStopPrint = CreateWindow("Button", "Stop Print", WS_CHILD|WS_VISIBLE, 88, 60, 80, 22, hWnd, (HMENU)IDC_BUTTON_STOP_PRINT, hInstDll, 0);
			hButtonPrintAgain = CreateWindow("Button", "Print again", WS_CHILD|WS_VISIBLE, 172, 60, 80, 22, hWnd, (HMENU)IDC_BUTTON_PRINT_AGAIN, hInstDll, 0);
			hTabPrinterSets = CreateWindow(WC_TABCONTROL, "Print sets", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 92, 476, 292, hWnd, (HMENU)IDC_TAB_PRINTER_SETS, hInstDll, 0);
			// Print sets elements
			hLabelNozzleDiameter = CreateWindow("Static", "Nozzle Diameter", WS_CHILD|WS_VISIBLE, 4, 32, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditNozzleDiameter = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 32, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_NOZZLE_DIAMETER, hInstDll, 0);
			hLabelLayerThickness = CreateWindow("Static", "Layer Thickness", WS_CHILD|WS_VISIBLE, 240, 32, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditLayerThickness = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 32, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_LAYER_THICKNESS, hInstDll, 0);

			hLabelScanWidth = CreateWindow("Static", "Scan Width", WS_CHILD|WS_VISIBLE, 4, 56, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 56, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_WIDTH, hInstDll, 0);
			hLabelScanTimes = CreateWindow("Static", "Scan Times", WS_CHILD|WS_VISIBLE, 240, 56, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanTimes = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 56, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_TIMES, hInstDll, 0);

			hLabelHatchWidth = CreateWindow("Static", "Hatch Width", WS_CHILD|WS_VISIBLE, 4, 80, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 80, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_WIDTH, hInstDll, 0);
			hLabelHatchSpace = CreateWindow("Static", "Hatch Space", WS_CHILD|WS_VISIBLE, 240, 80, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchSpace = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 80, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SPACE, hInstDll, 0);

			hLabelHatchLayer = CreateWindow("Static", "Hatch Layer", WS_CHILD|WS_VISIBLE, 4, 104, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchLayer = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 104, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_LAYER, hInstDll, 0);
			hLabelSupportWidth = CreateWindow("Static", "Support Width", WS_CHILD|WS_VISIBLE, 240, 104, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportWidth = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 104, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_WIDTH, hInstDll, 0);

			hLabelSupportSpace = CreateWindow("Static", "Support Space", WS_CHILD|WS_VISIBLE, 4, 128, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportSpace = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 128, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SPACE, hInstDll, 0);
			hLabelSupportLayer = CreateWindow("Static", "Support Layer", WS_CHILD|WS_VISIBLE, 240, 128, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportLayer = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 128, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_LAYER, hInstDll, 0);

			hLabelScanSpeed = CreateWindow("Static", "Scan Speed", WS_CHILD|WS_VISIBLE, 4, 152, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 152, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_SPEED, hInstDll, 0);
			hLabelHatchSpeed = CreateWindow("Static", "Hatch Speed", WS_CHILD|WS_VISIBLE, 240, 152, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 152, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SPEED, hInstDll, 0);

			hLabelSupportSpeed = CreateWindow("Static", "Support Speed", WS_CHILD|WS_VISIBLE, 4, 176, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 176, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SPEED, hInstDll, 0);
			hLabelJumpSpeed = CreateWindow("Static", "Jump Speed", WS_CHILD|WS_VISIBLE, 240, 176, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditJumpSpeed = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 176, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_JUMP_SPEED, hInstDll, 0);

			hLabelScanScale = CreateWindow("Static", "Scan Scale", WS_CHILD|WS_VISIBLE, 4, 200, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditScanScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 200, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SCAN_SCALE, hInstDll, 0);
			hLabelHatchScale = CreateWindow("Static", "Hatch Scale", WS_CHILD|WS_VISIBLE, 240, 200, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditHatchScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 200, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_HATCH_SCALE, hInstDll, 0);

			hLabelSupportScale = CreateWindow("Static", "Support Scale", WS_CHILD|WS_VISIBLE, 4, 224, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditSupportScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 156, 224, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_SUPPORT_SCALE, hInstDll, 0);
			hLabelFeedScale = CreateWindow("Static", "Feed Scale", WS_CHILD|WS_VISIBLE, 240, 224, 144, 22, hTabPrinterSets, NULL, hInstDll, 0);
			hEditFeedScale = CreateWindow("Edit", "0", WS_BORDER|WS_CHILD|WS_VISIBLE, 384, 224, 80, 22, hTabPrinterSets, (HMENU)IDC_INPUT_FEED_SCALE, hInstDll, 0);
			// Read config
			readSettingsFromConfig(hWnd);
			break;
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
				if (LOWORD(wParam) == IDC_CHECK_HEATER_TEMP) {
					// Heater temp override changed
					BOOL			newOverride = !IsDlgButtonChecked(hWnd, IDC_CHECK_HEATER_TEMP);
					if (newOverride) {
						CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, BST_CHECKED);
					} else {
						CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, BST_UNCHECKED);
					}
					settings.heaterTempOverride = newOverride;
				}
				if (LOWORD(wParam) == IDC_CHECK_PREHEAT_TIME) {
					// Heater temp override changed
					BOOL			newDelay = !IsDlgButtonChecked(hWnd, IDC_CHECK_PREHEAT_TIME);
					if (newDelay) {
						CheckDlgButton(hWnd, IDC_CHECK_PREHEAT_TIME, BST_CHECKED);
					} else {
						CheckDlgButton(hWnd, IDC_CHECK_PREHEAT_TIME, BST_UNCHECKED);
					}
					settings.preheatDelay = newDelay;
				}
				if (LOWORD(wParam) == IDC_BUTTON_SET_TEMP) {
					// Manual temperature set requested
					PrinterIntercept::getInstance()->setNozzle1Temp( settings.heaterTemp );
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
				if ((LOWORD(wParam) & 0xF00) == 0x600) {
					// Printer set input field
					UP_PRINT_SET_STRUCT* printSet = UpPrintSets::getInstance()->GetPrintSet(iPrintSetIndex);
					if (printSet != NULL) {
						if (LOWORD(wParam) == IDC_INPUT_NOZZLE_DIAMETER) {
							GetWindowTextA(hEditNozzleDiameter, tmpInputText, 32);
							stream << tmpInputText << flush;
							stream >> printSet->nozzle_diameter;
						}
					}
				}
				if (LOWORD(wParam) == IDC_INPUT_HEATER_TEMP) {
					// Heater temp changed
					ULONG			newHeaterTemp = 0;
					GetWindowTextA(hEditHeaterTemp, tmpInputText, 32);
					stream << tmpInputText << flush;
					stream >> newHeaterTemp;
					if (newHeaterTemp != settings.heaterTemp) {
						// Temperature changed
						if ((newHeaterTemp > 30) && (newHeaterTemp < 300)) {
							setHeaterTemperature(newHeaterTemp);
							// Set override if not already done
							CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, BST_CHECKED);
						} else {
							// Invalid input, disable override if active
							if (settings.heaterTempOverride) {
								settings.heaterTempOverride = false;
								CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, BST_UNCHECKED);
							}
						}
					}
				}
				if (LOWORD(wParam) == IDC_INPUT_PREHEAT_TIME) {
					UINT preheatMinutes = 0;
					GetWindowTextA(hEditPreheatTime, tmpInputText, 32);
					stream << tmpInputText << flush;
					stream >> preheatMinutes;
					settings.preheatTime = preheatMinutes * 30;
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

USHORT PrinterSettings::getHeaterTemperature() {
	return settings.heaterTemp;
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

void PrinterSettings::setHeaterTemperature(USHORT newTemp) {
	setHeaterTemperature(newTemp, true);
}

void PrinterSettings::setHeaterTemperature(USHORT newTemp, BOOL override) {
	if (override || !settings.heaterTempOverride) {
		// No custom temperature is set / Setting new custom temperature
		settings.heaterTemp = newTemp;
		if (!override) {
			CHAR* cHeaterTemp = new CHAR[16];
			sprintf(cHeaterTemp, "%lu", settings.heaterTemp);
			// Update input field
			SetWindowText(hEditHeaterTemp, cHeaterTemp);
			// Cleanup
			delete[] cHeaterTemp;
		}
	}
	// Set override flag if required
	settings.heaterTempOverride = (override ? true : settings.heaterTempOverride);
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
					sprintf(cInputText, "%lu", settings.heaterTemp);
					SetWindowTextA(hEditHeaterTemp, cInputText);
					sprintf(cInputText, "%lu", settings.preheatTime / 30);
					SetWindowTextA(hEditPreheatTime, cInputText);
					CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, (settings.heaterTempOverride ? BST_CHECKED : BST_UNCHECKED));
					CheckDlgButton(hWnd, IDC_CHECK_PREHEAT_TIME, (settings.preheatDelay ? BST_CHECKED : BST_UNCHECKED));
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
	settings.heaterTempOverride = false;
}

void PrinterSettings::updatePreheatTimer(ULONG newTime) {
	lPreheatTimer = newTime;
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
