/*
 * UpPrinterData.h
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#ifndef UPPRINTERDATA_H_
#define UPPRINTERDATA_H_

#include <stdint.h>
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

#include <pshpack1.h>

typedef struct UP_PDAT_HDR_STRUCT   //28 byte
{
  float    flt_unk1;            //1.0
  uint32_t u32_printerid;       //00002777 => UP Mini(M)
  uint32_t u32_unk3;            //1
  uint32_t u32_unk4;            //102
  float    f_rom_version;       //6.1
  uint32_t u32_printerserial;   //208460
  uint32_t u32_unk7;            //100
} UP_PDAT_HDR_STRUCT;

typedef struct UP_PDAT_NAME_STRUCT  //63 byte
{
  unsigned char printer_name[63];
} UP_PDAT_NAME_STRUCT;

typedef struct UP_PDAT_DATA_STRUCT  //56 byte
{
  float    flt_unk1;     //-120.0  X?
  float    flt_unk2;     //0.0
  float    flt_unk3;     //0.0
  float    flt_unk4;     //120.0   Y?
  float    flt_unk5;     //130.0   Z?
  float    flt_unk6;     //850.0   VMAX-X mm/s?
  float    flt_unk7;     //850.0   VMAX-Y mm/s?
  float    flt_unk8;     //850.0   VMAX-Z mm/s?
  float    flt_unk9;     //40.0    VMAX-E mm/s?
  float    flt_unk10;    //0.0
  float    flt_unk11;    //0.0
  float    flt_unk12;    //0.0
  float    flt_unk13;    //1.0
  uint32_t u32_NumSets;  //4
} UP_PDAT_DATA_STRUCT;

#include <poppack.h>

class UpPrinterData {
public:
	UpPrinterData();
	virtual ~UpPrinterData();
	static UpPrinterData* getInstance();

	void PrinterDataReset();
	bool PrinterDataFromUpResponse(unsigned char* pData, unsigned int len);

	bool PrinterDataAvalibale();
	void PrinterDataEmulationInit();
	void GetPrinterDataEmulationReset();
	uint32_t GetPrinterDataEmulation(unsigned char* pDataOut, unsigned int lenOutAvail);

private:
	static UpPrinterData *instance;

	PDAT_RESP_STATE respstate;
	uint32_t printsetsexpected;

	PDAT_RESP_STATE getstate;
	uint32_t getindex;

	UP_PDAT_HDR_STRUCT  pd_header;
	UP_PDAT_NAME_STRUCT pd_name;
	UP_PDAT_DATA_STRUCT pd_data;

};

#endif /* UPPRINTERDATA_H_ */
