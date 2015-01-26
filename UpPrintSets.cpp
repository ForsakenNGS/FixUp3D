/*
 * UpPrintSet.cpp
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#include <windows.h>
#include <string.h>
#include "UpPrintSets.h"

UpPrintSets* UpPrintSets::instance = NULL;

UpPrintSets* UpPrintSets::getInstance() {
	if( !instance )
		instance = new UpPrintSets();
	return instance;
}


UpPrintSets::UpPrintSets() :
  print_sets_valid(0)
{
	memset( print_sets,0,sizeof(print_sets) );
}

UpPrintSets::~UpPrintSets()
{
}

bool UpPrintSets::AddPrintSet(UP_PRINT_SET_STRUCT* pPrintSet)
{
	if( print_sets_valid >= UP_MAX_PRINT_SETS )
		return false;

	memcpy( &print_sets[print_sets_valid], pPrintSet, sizeof(UP_PRINT_SET_STRUCT) );
	print_sets_valid++;

	return true;
}

bool UpPrintSets::AddPrintSet(char set_name[16], float nozzle_diameter,
		float layer_thickness, float scan_width, float scan_times,
		float hatch_width, float hatch_space, float hatch_layer,
		float support_width, float support_space, float support_layer,
		float scan_speed, float hatch_speed, float support_speed,
		float jump_speed, float scan_scale, float hatch_scale,
		float support_scale, float feed_scale,
		float OP1, float OP2, float OP3, float OP4, float OP5, float OP6 )

{
	if( print_sets_valid >= UP_MAX_PRINT_SETS )
		return false;

	memset( &print_sets[print_sets_valid], 0, sizeof(UP_PRINT_SET_STRUCT) );
	memcpy( print_sets[print_sets_valid].set_name, set_name, 15 );
	print_sets[print_sets_valid].nozzle_diameter = nozzle_diameter;
	print_sets[print_sets_valid].layer_thickness = layer_thickness;
	print_sets[print_sets_valid].scan_width = scan_width;
	print_sets[print_sets_valid].scan_times = scan_times;
	print_sets[print_sets_valid].hatch_width = hatch_width;
	print_sets[print_sets_valid].hatch_space = hatch_space;
	print_sets[print_sets_valid].hatch_layer = hatch_layer;
	print_sets[print_sets_valid].support_width = support_width;
	print_sets[print_sets_valid].support_space = support_space;
	print_sets[print_sets_valid].support_layer = support_layer;
	print_sets[print_sets_valid].scan_speed = scan_speed;
	print_sets[print_sets_valid].hatch_speed = hatch_speed;
	print_sets[print_sets_valid].support_speed = support_speed;
	print_sets[print_sets_valid].jump_speed = jump_speed;
	print_sets[print_sets_valid].scan_scale = scan_scale;
	print_sets[print_sets_valid].hatch_scale = hatch_scale;
	print_sets[print_sets_valid].support_scale = support_scale;
	print_sets[print_sets_valid].feed_scale = feed_scale;
	print_sets[print_sets_valid].other_param_1 = OP1;
	print_sets[print_sets_valid].other_param_2 = OP2;
	print_sets[print_sets_valid].other_param_3 = OP3;
	print_sets[print_sets_valid].other_param_4 = OP4;
	print_sets[print_sets_valid].other_param_5 = OP5;
	print_sets[print_sets_valid].other_param_6 = OP6;
	print_sets_valid++;

	return true;
}

unsigned int UpPrintSets::GetPrintSetData(unsigned char** ppSetData)
{
	*ppSetData = (unsigned char*)print_sets;
	return print_sets_valid;
}

void UpPrintSets::ResetPrintSets() {
	print_sets_valid = 0;
}
