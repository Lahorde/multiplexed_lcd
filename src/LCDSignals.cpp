/*
 * LCDSignals.cpp
 *
 *  Created on: 13 janv. 2014
 *      Author: remi
 */

#include <logger.h>
#include "LCDSignals.h"
#include "Arduino.h"

LCDSignals::LCDSignals(const unsigned int arg_au16CommonLinesPins[], const unsigned int arg_au16SegmentLinesPins[], unsigned int arg_u16NbBackplanes,unsigned int arg_u16NbSegments, unsigned int arg_u16SignalPeriodMs) :
_u16_min_level2_val(0),
_u16_max_level2_val(0),
_u16Multiplexing(arg_u16NbBackplanes),
_u16NbSegments(arg_u16NbSegments),
_u16SignalPeriodMs(arg_u16SignalPeriodMs)
{
	int loc_u16SegmentIndex, loc_u16BackplaneIndex;

	for(loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		_au16CommonLinesPins[loc_u16BackplaneIndex] = arg_au16CommonLinesPins[loc_u16BackplaneIndex];
		/* Configure common lines as analog input */
	}

	for(loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
	{
		_au16SegmentLinesPins[loc_u16SegmentIndex] = arg_au16SegmentLinesPins[loc_u16SegmentIndex];
		/*- Configure segments lines as digital inputs */
		pinMode(_au16SegmentLinesPins[loc_u16SegmentIndex], INPUT);
	}

	for(loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < MAX_BACKPLANES; loc_u16BackplaneIndex++)
	{
		for(loc_u16SegmentIndex = 0; loc_u16SegmentIndex < MAX_SEGMENTS; loc_u16SegmentIndex++)
		{
			_au16LCDSegLevel[loc_u16BackplaneIndex][loc_u16SegmentIndex] = UNINITALIZED_SEGMENT;
		}
	}
	getLCDSignalsLevels();
}

LCDSignals::~LCDSignals() {
}

void LCDSignals::capture(void)
{
	intializeLCDSegmentsValues();
	while(!isSignalCaptured())
	{
		captureLCDSegmentsValues();
	}
	LOG_DEBUG(F("Capture done \n"));
}

bool LCDSignals::isSegmentOn(int arg_u16CommonLine, int arg_u16Segment)
{
	return _au16LCDSegLevel[arg_u16CommonLine][arg_u16Segment];
}

void LCDSignals::intializeLCDSegmentsValues(void)
{
	for(int loc_u16ComIndex = 0; loc_u16ComIndex < _u16Multiplexing; loc_u16ComIndex++)
	{
		_au16LCDSegLevel[loc_u16ComIndex][0] = UNINITALIZED_SEGMENT;
	}
}

void LCDSignals::getLCDSignalsLevels(void)
{
	/* Get max level on Com0 - same level for other backplanes signals */
	unsigned int loc_u32Com0Level = analogRead(_au16CommonLinesPins[0]);
	unsigned long loc_u16MaxLevel = loc_u32Com0Level;

	for(int i = 0; i < 50; i++)
	{
		loc_u32Com0Level = analogRead(_au16CommonLinesPins[0]);
		if(loc_u32Com0Level > loc_u16MaxLevel)
		{
			loc_u16MaxLevel = loc_u32Com0Level;
		}
		delay(_u16SignalPeriodMs / 2);
	}
	LOG_INFO(F("Max LCD signal level is %d\n"), loc_u16MaxLevel);

	if (loc_u16MaxLevel == 0)
	{
		LOG_ERROR(" Cannot get value from lines : 0 level detected \n");

		/** TODO : handle error */
	}
	else
	{
		/* Set signal levels 10% minored */
		for(int loc_u16BiasIndex = 0; loc_u16BiasIndex < NB_BIAS; loc_u16BiasIndex++)
		{
			_au16SignalLevel[loc_u16BiasIndex] = ((loc_u16BiasIndex +1) * loc_u16MaxLevel)/(NB_BIAS);
			LOG_INFO(F("LCD signal level %d = %d\n"), loc_u16BiasIndex+1, _au16SignalLevel[loc_u16BiasIndex]);
		}

		_u16_min_level2_val = ((unsigned long) _au16SignalLevel[NB_BIAS-2]*90) /100;
		_u16_max_level2_val = ((unsigned long) _au16SignalLevel[NB_BIAS-2]*110) /100;
	}
}

bool LCDSignals::isBackPlaneLow(int arg_u16BackplaneIndex, unsigned int arg_au16CommonLevels[])
{
	/* Backplane must be low and other backplanes must be at level 2 */
	if(arg_au16CommonLevels[arg_u16BackplaneIndex] != 0)
	{
		return false;
	}

	for(int loc_u16BackPlaneIndex = 0; loc_u16BackPlaneIndex < _u16Multiplexing; loc_u16BackPlaneIndex++)
	{
		if(loc_u16BackPlaneIndex != arg_u16BackplaneIndex)
		{
			if(arg_au16CommonLevels[loc_u16BackPlaneIndex] > _u16_min_level2_val
				   && arg_au16CommonLevels[loc_u16BackPlaneIndex] < _u16_max_level2_val)
			{
				/** backplane @ level 2 - continue to check other backplanes */
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

void LCDSignals::captureLCDSegmentsValues(void)
{
	unsigned int loc_au16LCDSegLevel[_u16NbSegments];
	unsigned int loc_au16LCDCommonLevelBefore[_u16Multiplexing];
	unsigned int loc_au16LCDCommonLevelAfter[_u16Multiplexing];
	int loc_u16SegmentIndex, loc_u16BackplaneIndex;

    /* Read common lines before segment capture */
	for(int loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		loc_au16LCDCommonLevelBefore[loc_u16BackplaneIndex] = analogRead(_au16CommonLinesPins[loc_u16BackplaneIndex]);
	}

	/* Read segments */
	for(int loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
	{
		loc_au16LCDSegLevel[loc_u16SegmentIndex] = digitalRead(_au16SegmentLinesPins[loc_u16SegmentIndex]);
	}

    /* Read common lines after segment capture */
	for(int loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		loc_au16LCDCommonLevelAfter[loc_u16BackplaneIndex] = analogRead(_au16CommonLinesPins[loc_u16BackplaneIndex]);
	}

	/* Be sure lines have not changed during capture */
	for(int loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		if(loc_au16LCDCommonLevelBefore[loc_u16BackplaneIndex] != loc_au16LCDCommonLevelAfter[loc_u16BackplaneIndex])
		{
			/* Another capture must be done */
			return;
		}
	}


	/* Print values */
	for(int loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		LOG_DEBUG(F("Analog value %d: %d\n"), loc_u16BackplaneIndex, loc_au16LCDCommonLevelBefore[loc_u16BackplaneIndex]);
	}

	LOG_DEBUG(F("SEG "));
	for(int loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
	{
		LOG_DEBUG(F("%d "), loc_au16LCDSegLevel[loc_u16SegmentIndex]);
	}
	LOG_DEBUG(F("\n"));

	/* LCD backplane LOW AND others backplane to level 1 => Capture can be done on
	 * segment,  ON segments are signals to HIGH level for this backplane */

	for(int loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		if(isBackPlaneLow(loc_u16BackplaneIndex, loc_au16LCDCommonLevelBefore))
		{
			LOG_DEBUG(F("LCD Com %d Low\n"), loc_u16BackplaneIndex);
			LOG_DEBUG(F("SEG "));
			for(int loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
			{
				_au16LCDSegLevel[loc_u16BackplaneIndex][loc_u16SegmentIndex] = loc_au16LCDSegLevel[loc_u16SegmentIndex];
				LOG_DEBUG(F("%d "), loc_au16LCDSegLevel[loc_u16SegmentIndex]);
			}
			break;
		}
	}
}

bool LCDSignals::isSignalCaptured(void)
{
	for(int loc_u16ComIndex = 0; loc_u16ComIndex < _u16Multiplexing; loc_u16ComIndex++)
	{
		if(_au16LCDSegLevel[loc_u16ComIndex][0] == UNINITALIZED_SEGMENT)
		{
			return false;
		}
	}
	return true;
}

void LCDSignals::printSegments(void)
{
	int loc_u16SegmentIndex, loc_u16BackplaneIndex;

	LOG_DEBUG(F("\nSegments values :\nSEG Id     "));
	for(loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
	{
		LOG_DEBUG(F("%d   "), loc_u16SegmentIndex);
	}
	LOG_DEBUG(F("\n"));

	for(loc_u16BackplaneIndex = 0; loc_u16BackplaneIndex < _u16Multiplexing; loc_u16BackplaneIndex++)
	{
		LOG_DEBUG(F("COM %d     "), loc_u16BackplaneIndex);
		for(loc_u16SegmentIndex = 0; loc_u16SegmentIndex < _u16NbSegments; loc_u16SegmentIndex++)
		{
			if(loc_u16SegmentIndex < 10)
			{
				LOG_DEBUG(F("%d   "), _au16LCDSegLevel[loc_u16BackplaneIndex][loc_u16SegmentIndex]);
			}
			else
			{
				LOG_DEBUG(F("%d      "), _au16LCDSegLevel[loc_u16BackplaneIndex][loc_u16SegmentIndex]);
			}
		}
		LOG_DEBUG(F("\n"));
	}
	LOG_DEBUG(F("\n"));
}


