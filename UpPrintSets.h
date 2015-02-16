/*
 * UpPrintSet.h
 *
 *  Created on: 26.01.2015
 *      Author: stohn
 */

#ifndef UPPRINTSETS_H_
#define UPPRINTSETS_H_

#define UP_MAX_PRINT_SETS 8

#include <pshpack1.h>
typedef struct UP_PRINT_SET_STRUCT
{
  char  set_name[16];

  float nozzle_diameter;

  float layer_thickness;

  float scan_width;
  float scan_times;

  float hatch_width;
  float hatch_space;
  float hatch_layer;

  float support_width;
  float support_space;
  float support_layer;

  float scan_speed;
  float hatch_speed;
  float support_speed;
  float jump_speed;

  float scan_scale;
  float hatch_scale;
  float support_scale;
  float feed_scale;

  float other_param_1;
  float other_param_2;
  float other_param_3;
  float other_param_4;
  float other_param_5;
  float other_param_6;

  float unused_1;
  float unused_2;
  float unused_3;
  float unused_4;
  float unused_5;
  float unused_6;

} UP_PRINT_SET_STRUCT;
#include <poppack.h>

class UpPrintSets {
public:
	UpPrintSets();
	virtual ~UpPrintSets();
	static UpPrintSets* getInstance();

	void ResetPrintSets();

	bool AddPrintSet( UP_PRINT_SET_STRUCT* pPrintSet);

	bool AddPrintSet( char set_name[16],
					  float nozzle_diameter,
					  float layer_thickness,
					  float scan_width,
					  float scan_times,
					  float hatch_width,
					  float hatch_space,
					  float hatch_layer,
					  float support_width,
					  float support_space,
					  float support_layer,
					  float scan_speed,
					  float hatch_speed,
					  float support_speed,
					  float jump_speed,
					  float scan_scale,
					  float hatch_scale,
					  float support_scale,
					  float feed_scale,
					  float OP1,
					  float OP2,
					  float OP3,
					  float OP4,
					  float OP5,
					  float OP6
					);

	unsigned int GetPrintSetsAvail();
	UP_PRINT_SET_STRUCT* GetPrintSet(unsigned int index);
	UP_PRINT_SET_STRUCT* GetPrintSet(unsigned int index, bool forceDefault);

private:
    static UpPrintSets   *instance;
	unsigned int        print_sets_valid;
	UP_PRINT_SET_STRUCT print_sets[UP_MAX_PRINT_SETS];
};

#endif /* UPPRINTSETS_H_ */
