/*
 * UpPrinterData.h
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#ifndef UPPRINTERDATA_H_
#define UPPRINTERDATA_H_

#include "UpPrintSets.h"

enum PDAT_RESP_STATE
{
	PDRS_7,
	PDRS_HDR_BLOB,
	PDRS_NAME_BLOB,
	PDRS_DATA_BLOB,
	PDRS_SET_NAME,
	PDRS_SET_DAT1,
	PDRS_SET_DAT2,
	PDRS_6,

	PDRS_OK,

	PDRS_ERROR
};


class UpPrinterData {
public:
	UpPrinterData();
	virtual ~UpPrinterData();
	static UpPrinterData* getInstance();

	void PrinterDataReset();
	bool PrinterDataFromUpResponse(unsigned char* pData, unsigned int len);
private:
	static UpPrinterData *instance;

	PDAT_RESP_STATE respstate;
	unsigned int printsetsexpected;

};

#endif /* UPPRINTERDATA_H_ */
