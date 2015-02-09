/*
 * UpPrinterData.cpp
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#include <windows.h>
#include <inttypes.h>
#include <memory.h>
#include "UpPrinterData.h"

UpPrinterData* UpPrinterData::instance = NULL;

UpPrinterData* UpPrinterData::getInstance() {
	if( !instance )
		instance = new UpPrinterData();
	return instance;
}

UpPrinterData::UpPrinterData() :
	respstate(PDRS_7),
	printsetsexpected(0),
	getstate(PDRS_ERROR),
	getindex(0)
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
			if( (sizeof(pd_header) == len) ) //28
			{
				memcpy( &pd_header, pData, sizeof(pd_header) );
				respstate = PDRS_NAME_BLOB;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_NAME_BLOB:
		{
			if( (sizeof(pd_name) == len) ) //63
			{
				memcpy( &pd_name, pData, sizeof(pd_name) );
				respstate = PDRS_DATA_BLOB;
			}
			else
				respstate = PDRS_ERROR;
		}
		break;

		case PDRS_DATA_BLOB:
		{
			if( (sizeof(pd_data) == len) ) //56
			{
				memcpy( &pd_data, pData, sizeof(pd_data) );
				printsetsexpected = pd_data.u32_NumSets;
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

void UpPrinterData::GetPrinterDataEmulationReset()
{
	getstate = PDRS_7;
}

uint32_t UpPrinterData::GetPrinterDataEmulation(unsigned char* pDataOut, unsigned int lenOutAvail)
{
	switch( getstate )
	{
		case PDRS_7:
			if( lenOutAvail>= 1 )
			{
				pDataOut[0] = 0x07;
				getstate = PDRS_HDR_BLOB;
				return 1;
			}
			break;

		case PDRS_HDR_BLOB:
			if( lenOutAvail>= sizeof(pd_header) )
			{
				memcpy( pDataOut, &pd_header, sizeof(pd_header) );
				getstate = PDRS_NAME_BLOB;
				return sizeof(pd_header);
			}
			break;

		case PDRS_NAME_BLOB:
			if( lenOutAvail>= sizeof(pd_name) )
			{
				memcpy( pDataOut, &pd_name, sizeof(pd_name) );
				getstate = PDRS_DATA_BLOB;
				return sizeof(pd_name);
			}
			break;

		case PDRS_DATA_BLOB:
			if( lenOutAvail>= sizeof(pd_name) )
			{
				memcpy( pDataOut, &pd_data, sizeof(pd_data) );

				getindex = 0;

				if( UpPrintSets::getInstance()->GetPrintSetsAvail()>0 )
					getstate = PDRS_SET_NAME;
				else
					getstate = PDRS_6;

				return sizeof(pd_data);
			}
			break;

		case PDRS_SET_NAME:
			if( lenOutAvail>= 16 )
			{
				memcpy( pDataOut, ((unsigned char*)UpPrintSets::getInstance()->GetPrintSet(getindex)), 16 );
				getstate = PDRS_SET_DAT1;
				return 16;
			}
			break;

		case PDRS_SET_DAT1:
			if( lenOutAvail>= 60 )
			{
				memcpy( pDataOut, ((unsigned char*)UpPrintSets::getInstance()->GetPrintSet(getindex))+16, 60 );
				getstate = PDRS_SET_DAT2;
				return 60;
			}
			break;

		case PDRS_SET_DAT2:
			if( lenOutAvail>= 60 )
			{
				memcpy( pDataOut, ((unsigned char*)UpPrintSets::getInstance()->GetPrintSet(getindex))+16+60, 60 );

				getindex++;
				if( getindex < UpPrintSets::getInstance()->GetPrintSetsAvail() )
					getstate = PDRS_SET_NAME;
				else
					getstate = PDRS_6;
				return 60;
			}
			break;

		case PDRS_6:
			if( lenOutAvail>= 1 )
			{
				pDataOut[0] = 0x06;
				getstate = PDRS_OK;
				return 1;
			}
			break;

		case PDRS_OK:
		case PDRS_ERROR:
		default:
			break;
	}

	getstate = PDRS_ERROR;
	return 0;
}
