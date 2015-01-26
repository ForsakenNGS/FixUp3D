/*
 * UpPrinterData.cpp
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#include <windows.h>
#include <inttypes.h>
#include "UpPrinterData.h"

UpPrinterData* UpPrinterData::instance = NULL;

UpPrinterData* UpPrinterData::getInstance() {
	if( !instance )
		instance = new UpPrinterData();
	return instance;
}

UpPrinterData::UpPrinterData() :
	respstate(PDRS_7),
	printsetsexpected(0)
{
}

UpPrinterData::~UpPrinterData()
{
}

void UpPrinterData::PrinterDataReset()
{
	respstate = PDRS_7;
	UpPrintSets::getInstance()->ResetPrintSets();
}

bool UpPrinterData::PrinterDataFromUpResponse(unsigned char* pData, unsigned int len)
{
	static unsigned char setdata[16+60+60];

	switch( respstate )
	{
		case PDRS_7:
		{
			if( (1 == len) && (7==pData[0]) )
				respstate = PDRS_HDR_BLOB;
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_HDR_BLOB:
		{
			if( (28 == len) )
				respstate = PDRS_NAME_BLOB;
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_NAME_BLOB:
		{
			if( (63 == len) )
				respstate = PDRS_DATA_BLOB;
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_DATA_BLOB:
		{
			if( (56 == len) )
			{
				printsetsexpected = *((uint32_t*)&pData[0x34]);
				respstate = PDRS_SET_NAME;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_SET_NAME:
		{
			if( 0 == printsetsexpected )
			{
				respstate = PDRS_6;
				break;
			}

			if( (16 == len) )
			{
				memset( setdata, 0, sizeof(setdata) );
				memcpy( setdata, pData, 16);
				respstate = PDRS_SET_DAT1;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_SET_DAT1:
		{
			if( (60 == len) )
			{
				memcpy( setdata+16, pData, 60);
				respstate = PDRS_SET_DAT2;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_SET_DAT2:
		{
			if( (60 == len) )
			{
				memcpy( setdata+16+60, pData, 60);
				UpPrintSets::getInstance()->AddPrintSet((UP_PRINT_SET_STRUCT*)setdata);
				printsetsexpected--;
				respstate = PDRS_SET_NAME;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_6:
		{
			if( (1 == len) && (6==pData[0]) )
				respstate = PDRS_OK;
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_OK:
		case PDRS_ERROR:
			break;

		default:
			respstate = PDRS_ERROR;
			break;
	}

	if( PDRS_ERROR == respstate )
		return false;

	if( PDRS_OK == respstate )
		return false;

	return true;
}

