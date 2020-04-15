#ifndef WFCTAEVENTDECODE_H
#define WFCTAEVENTDECODE_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "TObject.h"

class WFCTAEventDecode//: public TObject
{
public:
	static const int s_pack_len;
    WFCTAEventDecode();
    WFCTAEventDecode(uint8_t *s_ptr, unsigned long buff_len);
    ~WFCTAEventDecode();

	void dump_pack();
	void nextpack();
	int packtype();
	void Find_SiPMs();
	std::map<short, int>& GetSiPM_Position() {return m_sipm_position;};
	short PackCheck() {return packCheck;};

	//event pack
	uint64_t eventId();
	int32_t bigpackLen();
	int16_t nFired();
	uint64_t RabbitTime();
	double Rabbittime();

	uint64_t eventId_in_channel(short isipm);
	int16_t zipMode(short isipm);
	bool GetOver_Single_Mark(short isipm);
	bool GetOver_Record_Mark(short isipm);
	float Getwinsum(short isipm);
	void GetWaveForm(short isipm, int *pulseh, int *pulsel);
	void GeteSaturation(short isipm, int *esath, int *esatl);


	//status pack
	uint8_t NextStatusLittlePack();
	void Getthresh(short *single_thresh, short *record_thresh);//Deal21Package
	void Deal22Pack(long *single_count);////Deal22Package get single count [channel 1-8]
	int GetDBNumber();
	int GetDBVersion();
	void Deal23Pack(long *single_count, long *single_time);
	void GetHV(float *HV);
	void GetPreTemp(float *PreTemp);
	void GetBigRes(float *BigResistence);
	void GetSmallRes(float *SmallResistence);
	int GetClbNumber();
	int GetClbVersion();
	void GetClbTemp(float *ClbTemp);
	int GetF9Mode();
	int GetPattern();
	long GetPat_Full();
	long GetPat_noFull();
	long GetnoPat_Full();
	long GetnoPat_noFull();
	int GetF9Version();
	int GetF9PlusVersion();
	uint64_t GetclbInitialTime();
	double GetclbInitialtime();
	int GetFiredTube();
	uint64_t GetStatusReadbackTime();
	double GetStatusReadbacktime();
	int GetF18Version();
	void GetMask(uint8_t f_board, int *mask);


	unsigned long buffer_len_remain();

private:
	unsigned long buffer_len;
	unsigned long finished_len;
	uint64_t pack_len;
	uint8_t *ptr0;
	uint8_t *ptr;
	uint8_t *ptr1;
	int pack_type;
	

	short packCheck;
	std::map<short, int> m_sipm_position;
	std::map<short, int>::iterator m_sipm_position_iter;

	void waveform(short isipm);
	int pulsehigh[28];
	int pulselow[28];
	int saturationH[28];
	int saturationL[28];

};

#endif // WFCTAEVENTDECODE_H
