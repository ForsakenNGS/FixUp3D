/*
 * PrinterSettings.h
 *
 *  Created on: 16.01.2015
 *      Author: Forsaken
 */

#ifndef PRINTERSETTINGS_H_
#define PRINTERSETTINGS_H_

#include <Windows.h>
#include "winusbhelper.h"

#define IDC_LABEL_HEATER_TEMP	0x101
#define IDC_INPUT_HEATER_TEMP	0x201
#define IDC_CHECK_HEATER_TEMP	0x301
#define IDC_BUTTON_SET_TEMP		0x401

#define	PRINTER_SETTING_VERSION	0x0100

namespace Core {

struct PrinterSettingsStruct {
	ULONG		heaterTemp;
	BOOL		heaterTempOverride;
};

class PrinterSettings {
private:
    static PrinterSettings			*instance;

	HINSTANCE						hInstDll;
	WINUSB_INTERFACE_HANDLE			hUsbInterface;
	// Current setup
	struct PrinterSettingsStruct	settings;
	// Printer status

	// Window elements
	HWND							hLabelHeaterTemp;
	HWND							hEditHeaterTemp;
	HWND							hCheckHeaterTemp;
	HWND							hButtonSetTemp;
public:
	PrinterSettings(HINSTANCE hInstance);
	virtual ~PrinterSettings();
	static PrinterSettings* getInstance();
	static PrinterSettings* getInstanceNew(HINSTANCE hInstance);

	LRESULT	handleWndMessage(HWND hWnd, UINT message, WPARAM lParam, LPARAM wParam);

	void	applyHeaterTemperature();
	USHORT	getHeaterTemperature();
	void	setHeaterTemperature(USHORT newTemp);
	void	setHeaterTemperature(USHORT newTemp, BOOL override);
	void	setUsbHandle(WINUSB_INTERFACE_HANDLE newHandle);
	void	readSettingsFromConfig(HWND hWnd);
	void	resetHeaterTemperature();
	void	writeSettingsToConfig();
};

} /* namespace Core */

#endif /* PRINTERSETTINGS_H_ */
