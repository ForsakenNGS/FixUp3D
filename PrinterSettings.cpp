/*
 * PrinterSettings.cpp
 *
 *  Created on: 16.01.2015
 *      Author: Forsaken
 */

#include "PrinterSettings.h"
#include "PrinterIntercept.h"
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
	hUsbInterface = NULL;
	// Current setup
	settings.heaterTemp = 0;
	settings.heaterTempOverride = false;
	// Window elements
	hLabelHeaterTemp = NULL;
	hEditHeaterTemp = NULL;
	hCheckHeaterTemp = NULL;
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
			hCheckHeaterTemp = CreateWindow("Button", "Override", BS_CHECKBOX|WS_CHILD|WS_VISIBLE, 260, 4, 80, 22, hWnd, (HMENU)IDC_CHECK_HEATER_TEMP, hInstDll, 0);
			hButtonSetTemp = CreateWindow("Button", "Send Temp", WS_CHILD|WS_VISIBLE, 4, 32, 80, 22, hWnd, (HMENU)IDC_BUTTON_SET_TEMP, hInstDll, 0);
			// Read config
			readSettingsFromConfig(hWnd);
			break;
		}
		case WM_COMMAND:
		{
			/*
			char	debug[64];
			sprintf(debug, "WM_COMMAND: %x (%i / %i), %x (%i / %i)", wParam, HIWORD(wParam), LOWORD(wParam), lParam, HIWORD(lParam), LOWORD(lParam));
			MessageBox(NULL, debug, "UpUsbIntercept", NULL);
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
				if (LOWORD(wParam) == IDC_BUTTON_SET_TEMP) {
					// Manual temperature set requested
					PrinterIntercept::getInstance()->setNozzle1Temp( settings.heaterTemp );
				}
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				if (LOWORD(wParam) == IDC_INPUT_HEATER_TEMP) {
					// Heater temp changed
					CHAR*			newHeaterTempText = new CHAR[16];
					ULONG			newHeaterTemp = 0;
					stringstream 	stream;
					GetWindowTextA(hEditHeaterTemp, newHeaterTempText, 16);
					stream << newHeaterTempText << flush;
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
			}
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
void PrinterSettings::applyHeaterTemperature() {
	UCHAR*	cmdBuffer = new UCHAR[6]{ 0x56, 0x39, 0xd2, 0x00, 0x00, 0x00 };
	ULONG	cmdBufferLen = 6;
	ULONG	transferLen = 0;
	*((PULONG)(cmdBuffer+2)) = settings.heaterTemp;
	if (!WinUsb_WritePipe(hUsbInterface, 1, cmdBuffer, cmdBufferLen, &transferLen, NULL)) {
		MessageBox(NULL, "Failed to apply heater temperature! (Write error)", "UpUsbIntercept", NULL);
	} else {
		UCHAR*	respBuffer = new UCHAR[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
		ULONG	respBufferLen = 6;
		if (!WinUsb_ReadPipe(hUsbInterface, 1, respBuffer, respBufferLen, &transferLen, NULL)) {
			MessageBox(NULL, "Failed to apply heater temperature! (Read error)", "UpUsbIntercept", NULL);
		}
	}
}
*/

USHORT PrinterSettings::getHeaterTemperature() {
	return settings.heaterTemp;
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
			sprintf(cHeaterTemp, "%i", settings.heaterTemp);
			// Update input field
			SetWindowText(hEditHeaterTemp, cHeaterTemp);
		}
	}
	// Set override flag if required
	settings.heaterTempOverride = (override ? true : settings.heaterTempOverride);
}

void PrinterSettings::setUsbHandle(WINUSB_INTERFACE_HANDLE newHandle) {
	hUsbInterface = newHandle;
}

void PrinterSettings::readSettingsFromConfig(HWND hWnd) {
	char	sHomeDir[MAX_PATH];
	char	sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		sprintf(sFilename, "%s\\UpUsbIntercept", sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Open/create config file
		sprintf(sFilename, "%s\\UpUsbIntercept\\config.cfg", sHomeDir);
		HANDLE hFile = CreateFile(sFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			// Config file does not exist
			MessageBox(NULL, "Failed to read configuration! (Could not open file)", "UpUsbIntercept", NULL);
			return;
		}
		DWORD	dwVersion = 0;
		DWORD	dwBytesRead = 0;
		if (ReadFile(hFile, &dwVersion, sizeof(DWORD), &dwBytesRead, NULL)) {
			if (dwVersion == PRINTER_SETTING_VERSION) {
				if (ReadFile(hFile, &settings, sizeof(settings), &dwBytesRead, NULL)) {
					// Success!
					CHAR* cHeaterTemp = new CHAR[16];
					sprintf(cHeaterTemp, "%i", settings.heaterTemp);
					SetWindowTextA(hEditHeaterTemp, cHeaterTemp);
					CheckDlgButton(hWnd, IDC_CHECK_HEATER_TEMP, (settings.heaterTempOverride ? BST_CHECKED : BST_UNCHECKED));
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

void PrinterSettings::writeSettingsToConfig() {
	char	sHomeDir[MAX_PATH];
	char	sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		sprintf(sFilename, "%s\\UpUsbIntercept", sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Open/create config file
		sprintf(sFilename, "%s\\UpUsbIntercept\\config.cfg", sHomeDir);
		HANDLE hFile = CreateFile(sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			MessageBox(NULL, "Failed to write configuration! (Could not create file)", "UpUsbIntercept", NULL);
			return;
		}
		DWORD	dwVersion = PRINTER_SETTING_VERSION;
		DWORD	dwBytesWritten = 0;
		if (WriteFile(hFile, &dwVersion, sizeof(DWORD), &dwBytesWritten, NULL)) {
			if (WriteFile(hFile, &settings, sizeof(settings), &dwBytesWritten, NULL)) {
				// Success!
			} else {
				MessageBox(NULL, "Failed to write configuration!", "UpUsbIntercept", NULL);
			}
		} else {
			MessageBox(NULL, "Failed to write configuration!", "UpUsbIntercept", NULL);
		}
		CloseHandle(hFile);
	}
}

} /* namespace Core */
