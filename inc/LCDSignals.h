/*
 * LCDSignals.h
 *
 *  Created on: 13 janv. 2014
 *      Author: remi
 */

#ifndef LCDSIGNALS_H_
#define LCDSIGNALS_H_

/**
 * Get segment signals from LCD lines. LCD is multiplexed and has 1/3 bias
 */
class LCDSignals {
	/* Attributes */
private :
	static const int UNINITALIZED_SEGMENT = 2;
	static const int MAX_SEGMENTS = 16;
	static const int MAX_BACKPLANES = 4;
	static const int NB_BIAS = 3;
	static const int NOT_INITIALIZED_PIN = 999;

	/** Analog lines levels */
	int _au16SignalLevel[NB_BIAS];

	/** Signal period */
	unsigned int _u16SignalPeriodMs;

	/** LCD config */
	unsigned int _u16Multiplexing;
	unsigned int _u16NbSegments;

	/** Lines Pins */
	unsigned int _au16CommonLinesPins[MAX_BACKPLANES];
	unsigned int _au16SegmentLinesPins[MAX_SEGMENTS];

	/** Segment signal : HIGH or LOW */
	unsigned int _au16LCDSegLevel[MAX_BACKPLANES][MAX_SEGMENTS];

	/** Criterion value to check backplanes lines @ level 2 */
	unsigned int _u16_min_level2_val;
	unsigned int _u16_max_level2_val;

	/* Methods */
public:
	/**
	 * Constructs objects from given LCD physical lines PINS
	 * @param arg_au16CommonLinesPins
	 * @param arg_au16SegmentLinesPins
	 * @param arg_u16NbBackplanes
	 * @param arg_u16NbSegments
	 * @param arg_u16SignalPeriodMs
	 */
	LCDSignals(const unsigned int arg_au16CommonLinesPins[], const unsigned int arg_au16SegmentLinesPins[], unsigned int arg_u16NbBackplanes,unsigned int arg_u16NbSegments, unsigned int arg_u16SignalPeriodMs);
    virtual ~LCDSignals();

	/** Do a signal capture. After a capture, signals can be get */
	void capture(void);

	/**
	 * Returns true if given segment active for given common line (backplane).
	 * A capture must be done before calling method
	 * @param arg_u16CommonLine common line index
	 * @param arg_u16Segment common segment index
	 * @return true if segment ON
	 */
	bool isSegmentOn(int arg_u16CommonLine, int arg_u16Segment);

	void printSegments(void);

private:
	void intializeLCDSegmentsValues(void);
	void getLCDSignalsLevels(void);
	bool isBackPlaneLow(int arg_u16BackplaneIndex, unsigned int arg_au16CommonLevels[]);
	void captureLCDSegmentsValues(void);
	bool isSignalCaptured(void);
};

#endif /* LCDSIGNALS_H_ */
