#ifndef WFCTADECODE_H
#define WFCTADECODE_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "TObject.h"

using namespace std;

class WFCTADecode//: public TObject
{
public:
    WFCTADecode();
    ~WFCTADecode();

    bool StatusPack(uint8_t *begin, int bufsize, int64_t packStart);
    uint8_t StatusPackCheck(uint8_t *begin, int bufsize, int64_t packStart);
    int statusPackCheck(uint8_t *begin, int bufsize,int type);//change "StatusPackCheck" to "statusPackCheck" by youzhiyong --- 2019 08 22

	//zip slice pack
    bool ZipDataFragment(uint8_t *begin);
	int32_t z_sliceLength(uint8_t *begin, int feedatahead);

	//time slice pack
    bool FEEDataFragment(uint8_t *begin);
    int feeDataHead() {return FEEDataHead;};
    int bigPackCheck(uint8_t *begin, int bufsize, int64_t packStart);
    void Find_SiPMs(uint8_t *begin);//, int packStart);

    int64_t PackSize() {return packSize;};
    short PackCheck() {return packCheck;};
    int32_t bigpackLen() {return big_pack_len;};
    map<short, int>& GetSiPM_Position() {return m_sipm_position;};


    int32_t sliceLength(uint8_t *begin,int feedatahead);
    short Telid(uint8_t *begin,int feedatahead);
    uint64_t eventId(uint8_t *begin);
    int16_t nFired(uint8_t *begin);
    uint64_t RabbitTime(uint8_t *begin);
    //uint64_t Rabbittime(uint8_t *begin);
    double Rabbittime(uint8_t *begin);

    uint64_t eventId_in_channel(uint8_t *begin, short isipm);
    int16_t zipMode(uint8_t *begin, short isipm);
    bool GetOver_Single_Mark(uint8_t *begin, short isipm);
    bool GetOver_Record_Mark(uint8_t *begin, short isipm);
    //uint8_t GetPeak(uint8_t *begin, short isipm);
    //bool Getgain_marker(uint8_t *begin, short isipm);
    uint8_t GetPeakPosH(uint8_t *begin, short isipm);
    uint8_t GetPeakPosL(uint8_t *begin, short isipm);
    int32_t GetPeakAmH(uint8_t *begin, short isipm);
    int32_t GetPeakAmL(uint8_t *begin, short isipm);
    //uint16_t GetSingle_Thresh(uint8_t *begin, short isipm);
    //uint16_t GetRecord_Thresh(uint8_t *begin, short isipm);
    //float GetADC_Cut(uint8_t *begin, short isipm);
    //float AdcHigh(uint8_t *begin, short isipm);
    //float AdcLow(uint8_t *begin, short isipm);
    //float BaseHigh(uint8_t *begin, short isipm);
    //float BaseLow(uint8_t *begin, short isipm);
    float Getwinsum(uint8_t *begin, short isipm);
    float GetwaveImageBaseHigh(uint8_t *begin, short isipm);
    float GetwaveImageBaseLow(uint8_t *begin, short isipm);
    float GetwaveImageAdcHigh(uint8_t *begin, short isipm);
    float GetwaveImageAdcLow(uint8_t *begin, short isipm);
    bool eSaturationHigh(uint8_t *begin, short isipm);
    bool eSaturationLow(uint8_t *begin, short isipm);
    void GetWaveForm(uint8_t *begin, short isipm, int *pulseh, int *pulsel);
    void GeteSaturation(uint8_t *begin, short isipm, int *esath, int *esatl);
    

    void Getthresh(uint8_t *begin, int packsize, short *single_thresh, short *record_thresh);//Deal21Package
    void Deal22Pack(uint8_t *begin, int packsize, long *single_count);//Deal22Package get single count [channel 1-8]
    void Deal23Pack(uint8_t *begin, int packsize, long *single_count, long *single_time);//Deal23Package get single [channel 9-16] count and single time
    void GetHV(uint8_t *begin, int packsize, float *HV);//Deal81Package
    void GetPreTemp(uint8_t *begin, int packsize, float *PreTemp);//Deal82Package
    void GetBigRes(uint8_t *begin, int packsize, float *BigResistence);//Deal83Package
    void GetSmallRes(uint8_t *begin, int packsize, float *SmallResistence);//Deal84Package
    void GetClbTemp(uint8_t *begin, int packsize, float *ClbTemp);//Deal85pack
    void GetClbTime(uint8_t *begin, int packsize, long *ClbTime);//Deal85pack

	int GetF9Mode(uint8_t *begin, int packsize);
	int GetPattern(uint8_t *begin, int packsize);
	long GetPat_Full(uint8_t *begin, int packsize);
	long GetPat_noFull(uint8_t *begin, int packsize);
	long GetnoPat_Full(uint8_t *begin, int packsize);
	long GetnoPat_noFull(uint8_t *begin, int packsize);
	int GetF9Version(uint8_t *begin, int packsize);
	int GetF9PlusVersion(uint8_t *begin, int packsize);
    uint64_t GetclbInitialTime(uint8_t *begin, int packsize);
    double GetclbInitialtime(uint8_t *begin, int packsize);
    int GetFiredTube(uint8_t *begin, int packsize);
    uint64_t GetStatusReadbackTime(uint8_t *begin, int packsize);
    double GetStatusReadbacktime(uint8_t *begin, int packsize);

	int GetDBNumber(uint8_t *begin, int packsize);
	int GetClbNumber(uint8_t *begin, int packsize);
	int GetF18Version(uint8_t *begin, int packsize);
	void GetMask(uint8_t *begin, int packsize, uint8_t f_board, int *mask);
	int GetDBVersion(uint8_t *begin, int packsize);
	int GetClbVersion(uint8_t *begin, int packsize);

private:

    void wavepeak(uint8_t *begin, short isipm);
    void waveform(uint8_t *begin, short isipm);
    void Calc_Q_Base(uint8_t *begin, short isipm);
    void Stauration(uint8_t *begin, short isipm);

    //uint8_t *m_buffer;
    //size_t m_size;
    int64_t head,tail;
    int64_t readPos;
    int FEEDataHead;

    uint8_t status_pack_mark;
    int64_t packSize;
    map<short, int> m_sipm_position;
    map<short, int>::iterator m_sipm_position_iter;

    //char m_peak;
	short packCheck;
	uint64_t evtid;
    float m_adc_high;
    float m_adc_low;
    //float m_base_high;
    //float m_base_low;

    float m_Basehigh;
    float m_Baselow;
    float m_Adchigh;
    float m_Adclow;

    bool eSatH;
    bool eSatL;

    int32_t big_pack_len;
    int32_t peakAmpH;
    int32_t peakAmpL;
    uint8_t m_wavepeakH;
    uint8_t m_wavepeakL;

    int pulsehigh[28];
    int pulselow[28];
    int saturationH[28];
    int saturationL[28];

  ClassDef(WFCTADecode,1);
};

#endif // WFCTADECODE_H
