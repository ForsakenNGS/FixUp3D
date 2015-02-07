/*
 * PrinterSettings.h
 *
 *  Created on: 16.01.2015
 *      Author: Forsaken
 */

#ifndef PRINTERSETTINGS_H_
#define PRINTERSETTINGS_H_

#include <Windows.h>
#include <commctrl.h>
#include "winusbhelper.h"
#include "UpPrintSets.h"

#define IDC_LABEL_HEATER_TEMP	0x101
#define IDC_INPUT_HEATER_TEMP	0x201
#define IDC_CHECK_HEATER_TEMP	0x301
#define IDC_LABEL_PREHEAT_TIME	0x102
#define IDC_INPUT_PREHEAT_TIME	0x202
#define IDC_CHECK_PREHEAT_TIME	0x302
#define IDC_BUTTON_SET_TEMP		0x401
#define IDC_BUTTON_STOP_PRINT	0x402
#define IDC_BUTTON_PRINT_AGAIN	0x403
#define IDC_TAB_PRINTER_SETS	0x501

#define IDC_INPUT_NOZZLE_DIAMETER	0x601
#define IDC_INPUT_LAYER_THICKNESS	0x602
#define IDC_INPUT_SCAN_WIDTH		0x603
#define IDC_INPUT_SCAN_TIMES		0x604
#define IDC_INPUT_HATCH_WIDTH		0x605
#define IDC_INPUT_HATCH_SPACE		0x606
#define IDC_INPUT_HATCH_LAYER		0x607
#define IDC_INPUT_SUPPORT_WIDTH		0x608
#define IDC_INPUT_SUPPORT_SPACE		0x609
#define IDC_INPUT_SUPPORT_LAYER		0x60A
#define IDC_INPUT_SCAN_SPEED		0x60B
#define IDC_INPUT_HATCH_SPEED		0x60C
#define IDC_INPUT_SUPPORT_SPEED		0x60D
#define IDC_INPUT_JUMP_SPEED		0x60E
#define IDC_INPUT_SCAN_SCALE		0x60F
#define IDC_INPUT_HATCH_SCALE		0x610
#define IDC_INPUT_SUPPORT_SCALE		0x611
#define IDC_INPUT_FEED_SCALE		0x612

#define	PRINTER_SETTING_VERSION	0x0101

namespace Core {

struct PrinterSettingsStruct {
	ULONG		heaterTemp;
	BOOL		heaterTempOverride;
	ULONG		preheatTime;
	BOOL		preheatDelay;
};

class PrinterSettings {
private:
    static PrinterSettings			*instance;

	HINSTANCE						hInstDll;
	WINUSB_INTERFACE_HANDLE			hUsbInterface;
	unsigned int					iPrintSetIndex;
	unsigned int					iPrintSetCount;
	// Current setup
	struct PrinterSettingsStruct	settings;
	// Printer status
	// Window elements
	HWND							hLabelHeaterTemp;
	HWND							hEditHeaterTemp;
	HWND							hCheckHeaterTemp;
	HWND							hLabelPreheatTime;
	HWND							hEditPreheatTime;
	HWND							hCheckPreheatTime;
	HWND							hButtonSetTemp;
	HWND							hButtonStopPrint;
	HWND							hButtonPrintAgain;
	HWND							hTabPrinterSets;

	HWND							hLabelNozzleDiameter;
	HWND							hEditNozzleDiameter;
	HWND							hLabelLayerThickness;
	HWND							hEditLayerThickness;
	HWND							hLabelScanWidth;
	HWND							hEditScanWidth;
	HWND							hLabelScanTimes;
	HWND							hEditScanTimes;
	HWND							hLabelHatchWidth;
	HWND							hEditHatchWidth;
	HWND							hLabelHatchSpace;
	HWND							hEditHatchSpace;
	HWND							hLabelHatchLayer;
	HWND							hEditHatchLayer;
	HWND							hLabelSupportWidth;
	HWND							hEditSupportWidth;
	HWND							hLabelSupportSpace;
	HWND							hEditSupportSpace;
	HWND							hLabelSupportLayer;
	HWND							hEditSupportLayer;
	HWND							hLabelScanSpeed;
	HWND							hEditScanSpeed;
	HWND							hLabelHatchSpeed;
	HWND							hEditHatchSpeed;
	HWND							hLabelSupportSpeed;
	HWND							hEditSupportSpeed;
	HWND							hLabelJumpSpeed;
	HWND							hEditJumpSpeed;
	HWND							hLabelScanScale;
	HWND							hEditScanScale;
	HWND							hLabelHatchScale;
	HWND							hEditHatchScale;
	HWND							hLabelSupportScale;
	HWND							hEditSupportScale;
	HWND							hLabelFeedScale;
	HWND							hEditFeedScale;
public:
	PrinterSettings(HINSTANCE hInstance);
	virtual ~PrinterSettings();
	static PrinterSettings* getInstance();
	static PrinterSettings* getInstanceNew(HINSTANCE hInstance);

	LRESULT	handleWndMessage(HWND hWnd, UINT message, WPARAM lParam, LPARAM wParam);

	void	applyHeaterTemperature();
	USHORT	getHeaterTemperature();
	ULONG	getPreheatTime();
	BOOL	getPreheatDelayPrint();
	void	setHeaterTemperature(USHORT newTemp);
	void	setHeaterTemperature(USHORT newTemp, BOOL override);
	void	setPreheatTime(ULONG preheatSeconds);
	void	setUsbHandle(WINUSB_INTERFACE_HANDLE newHandle);
	void	readSettingsFromConfig(HWND hWnd);
	void	resetHeaterTemperature();
	void	updatePrintSet(unsigned int index, UP_PRINT_SET_STRUCT* printSet);
	void	writeSettingsToConfig();
};

} /* namespace Core */

#endif /* PRINTERSETTINGS_H_ */
