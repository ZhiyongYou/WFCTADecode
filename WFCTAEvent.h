#ifndef WFCTAEVENT_H
#define WFCTAEVENT_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "TTree.h"
#include "TBranch.h"
#include "TObject.h"

//using namespace std;
const int MAXPMT=1024;
//class WFCTAEvent : public TSelector
class WFCTAEvent : public TObject
{
	protected:
		static WFCTAEvent* _Head;
		static TTree* _Tree;
		static const char * _Name;
		static TBranch* bAll;

		int l_npix;

		static const int cleanPix;
		int eLG_Sat;
		int LG1_Sat;
		int LG2_Sat;
		int LG3_Sat;
		int HG_Sat;
		int ratioh2;
		int ratioh3;
		int ratiol2;
		int ratiol3;
		double totalsize;
		double top6;
		double ratio_ave;
		double ratio_rms;

		double mjd;
		int DNpix;
		double DSize;
		double DMeanX;
		double DMeanY;
		double Dslope;
		double Dintercept;
		double DLength;
		double DWidth;
		std::vector<double> RawImagePe;
		std::vector<int> RawImageSiPM;
		std::vector<double> RawImageX;
		std::vector<double> RawImageY;
		std::vector<double> FullImagePe;
		std::vector<int> FullImageSiPM;
		std::vector<double> FullImageX;
		std::vector<double> FullImageY;
		double ImageX[1024];
		double ImageY[1024];
		std::vector<int> fNpixfriends;
		float led_fac_h[1024];
		float led_fac_l[1024];
		std::vector<double> adc;
		std::vector<double> ratio;
		std::vector<int> maskSiPM;

	public:
		short iTel;
		short merge_size;
		long iEvent;
		long eEvent;
		long rabbitTime;
		double rabbittime;
		int big_pack_lenth;
		short n_fired;
		short n_Channel;
		std::vector<short> packCheck;

		std::vector<long> eevent;
		std::vector<short> zipmod;
		std::vector<short> iSiPM;
		std::vector<float> winsum;
		std::vector<bool> eSatH;
		std::vector<bool> eSatL;
		std::vector<float> BaseH;
		std::vector<float> BaseL;
		std::vector<float> LaserBaseH;
		std::vector<float> LaserBaseL;
		std::vector<float> AdcH;
		std::vector<float> AdcL;
		std::vector<float> LaserAdcH;
		std::vector<float> LaserAdcL;
		std::vector<bool> SatH;
		std::vector<bool> SatL;
		std::vector<short> PeakPosH;
		std::vector<short> PeakPosL;
		std::vector<int> PeakAmH;
		std::vector<int> PeakAmL;
		std::vector<bool> Over_Single_Marker;
		std::vector<bool> Over_Record_Marker;


	public:
		WFCTAEvent();
		~WFCTAEvent();
		void EventInitial();

		Int_t L_Npix()	{return l_npix;}
		Int_t eLG_SAT()	{return eLG_Sat;}
		Int_t LG1_SAT()	{return LG1_Sat;}
		Int_t LG2_SAT()	{return LG2_Sat;}
		Int_t LG3_SAT()	{return LG3_Sat;}
		Int_t HG_SAT()	{return HG_Sat;}
		Int_t RatioH2()	{return ratioh2;}
		Int_t RatioH3()	{return ratioh3;}
		Int_t RatioL2()	{return ratiol2;}
		Int_t RatioL3()	{return ratiol3;}
		Double_t TotalSize() {return totalsize;}
		Double_t Top6() {return top6;}
		Double_t Ratio_Ave() {return ratio_ave;}
		Double_t Ratio_Rms() {return ratio_rms;}

		Double_t MJD() {return mjd;}
		Short_t ITel() { return iTel;}
		Int_t Npix() {return DNpix;}
		Double_t Size() {return DSize;}
		Double_t MEANX() {return DMeanX;}
		Double_t MEANY() {return DMeanY;}
		Short_t Merge_Size() { return merge_size; } 
		Long_t IEvent() { return iEvent; }
		Long_t EEvent() { return eEvent; }
		Long_t RabbitTime() { return rabbitTime; }
		Double_t Rabbittime() { return rabbittime; }
		Int_t Big_Pack_Len() { return big_pack_lenth; }
		Short_t N_fired() { return n_fired; }
		Short_t N_channel() { return n_Channel; }
		std::vector<short> PackCheck() { return packCheck; }

		std::vector<long> Eevent() { return eevent; }
		std::vector<short> Zipmod() { return zipmod; }
		std::vector<short> SiPMNO() { return iSiPM; }
		std::vector<float> Win4Sum() { return winsum; }
		std::vector<bool> eSath() { return eSatH; }
		std::vector<bool> eSatl() { return eSatL; }
		std::vector<float> Baseh() { return BaseH; }
		std::vector<float> Basel() { return BaseL; }
		std::vector<float> LaserBaseh() { return LaserBaseH; }
		std::vector<float> LaserBasel() { return LaserBaseL; }
		std::vector<float> Adch() { return AdcH; }
		std::vector<float> Adcl() { return AdcL; }
		std::vector<float> LaserAdch() { return LaserAdcH; }
		std::vector<float> LaserAdcl() { return LaserAdcL; }
		std::vector<bool> Sath() { return SatH; }
		std::vector<bool> Satl() { return SatL; }
		std::vector<short> PeakPosh() { return PeakPosH; }
		std::vector<short> PeakPosl() { return PeakPosL; }
		std::vector<int> PeakAmh() { return PeakAmH; }
		std::vector<int> PeakAml() { return PeakAmL; }
		std::vector<bool> Single_Marker() { return Over_Single_Marker; }
		std::vector<bool> Record_Marker() { return Over_Record_Marker; }

		void InitImage();
		void SetImage();

		void SetMaskPix(std::vector<int> &masksipm);
		void SetLedFac(float *led_h, float *led_l);
		void CalcLedPar();
		int IsLedEvent();
		int IsLaserEvent();
		void rabbittime2lt();
		void AdcToPe(float *deltag_20, float *correct_PreTemp, int eventType);

		ClassDef(WFCTAEvent,1);
};

ClassImp(WFCTAEvent);

#endif // WFCTAEVENT_H
