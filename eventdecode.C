#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <zlib.h>
#include <assert.h>
#include "dumpPack.h"
#include "WFCTAEvent.h"
#include "WFCTAMerge.h"
#include "WFCTAEventDecode.h"

#define BUF_LEN 120000000
#define STATUS_BUF_LEN 120000000
#define EV_FILE_HEAD 256
#define DAQ_EVT_ALL_HEAD 104

//using namespace std;

void event_decode(TTree* tree, uint8_t *ptr, unsigned long& buffer_len, int decode_type)
{
	unsigned long keep_size = 0;
	if(0==decode_type)
		keep_size=5000000;

	WFCTAEventDecode wfctaDecode(ptr, buffer_len);
	while(1)
	{
		wfctaDecode.nextpack();
		if(0==wfctaDecode.packtype())
		{
			tree->Fill();
			//tree->Fill();
		}

		buffer_len = wfctaDecode.buffer_len_remain();
		//printf("buffer_len_remain: %llu\n",buffer_len);
		if(buffer_len<=keep_size)
			break;
	}
}

int main(int argc, char**argv)
{
	if(argc!=3)
	{
		printf("Use %s iptpath/inputfile outpath/outfile\n",argv[0]);
		return 0;
	}

	char Name1[300]="root://eos01.ihep.ac.cn/";
    char Name2[300];
	strcpy(Name2,Name1);
	strcat(Name2,argv[2]);
	//strcpy(Name2,argv[2]);
	TFile *rootfile = TFile::Open(Name2,"recreate");
	WFCTAEvent *wfctaEvent = new WFCTAEvent();
	TTree *eventShow = new TTree("eventShow","info of evnets");
	wfctaEvent -> CreateBranch(eventShow,1);
	wfctaEvent->EventInitial();

	WFCTAMerge merge_evt;
	std::vector<WFCTAMerge> merge_evts;
	std::vector<WFCTAMerge>::iterator merge_evts_iter;
	merge_evts.clear();

	char* datafile = argv[1];
	std::ifstream fdata;
	fdata.open(datafile,std::ios::in|std::ios::binary);
	if(!fdata.good())
	{
		std::cout<<"cannot open file"<<std::endl;
	}

	uint8_t* bufferRaw = new uint8_t[BUF_LEN];
	uint8_t* buffertemp = new uint8_t[BUF_LEN];
	uint8_t* buffer = new uint8_t[BUF_LEN];
	
	uint8_t* FileHeadBuffer = new uint8_t[300];
	fdata.read((char*)FileHeadBuffer,EV_FILE_HEAD);
	dumpPacket(FileHeadBuffer,EV_FILE_HEAD,16);
	if(!fdata)
	{
	}
	uint32_t headersize = 20;
	unsigned long buffer_len = 0;
	float adch;
	float adcl;
	uint16_t ITEL;
	long Time0=0,Time1;
	double time0=0,time1;
	int nevent[20]={0};
	while(true)
	{
		size_t sizeRead = 0;
		uint8_t* ptr1 = bufferRaw;
		fdata.read((char*)ptr1, headersize);
		if(!fdata) {
			break;
		}
		sizeRead += headersize;
		uint32_t m_totalSize= *(uint32_t*)(ptr1+12);
		assert(m_totalSize <= BUF_LEN);

		fdata.read((char*)ptr1+sizeRead, m_totalSize-sizeRead);
		if(!fdata)
		{   
			break;
		}
		//dumpPacket(ptr1,m_totalSize,16);
		//break;

		//uncompress and fill data to buffer
		uint8_t *ptrtemp = buffertemp;
		unsigned long len;
		int Compress=1;
		if(Compress)
		{
			len = BUF_LEN-headersize;
			if(uncompress(buffertemp, &len, bufferRaw+headersize, m_totalSize-sizeRead) != Z_OK)
			{
				printf("uncompress failed!\n");
				return 1;
			}
			std::cout << "uncompressed OK, uncompressed length is: " << len << std::endl;
		}
		else
		{
			len = m_totalSize;
			memcpy(buffertemp,bufferRaw,len);
		}
		//dumpPacket(buffertemp,len,16);

		if(len>DAQ_EVT_ALL_HEAD)
		{
			uint32_t event_pack_len = *(uint32_t*)(ptrtemp+88);
			ITEL = *(uint16_t*)(ptrtemp+92);
			//std::cout << "evt_pack_len: " << event_pack_len << " | iTel: " << ITEL << std::endl;
			memcpy(buffer+buffer_len,buffertemp+DAQ_EVT_ALL_HEAD,len-DAQ_EVT_ALL_HEAD);
			//dumpPacket(buffer+buffer_len,len-DAQ_EVT_ALL_HEAD,16);
			buffer_len += len-DAQ_EVT_ALL_HEAD;
		}
		//printf("event_buffer len: %llu\n",buffer_len);
		unsigned long buffer_len_all = buffer_len;
		if(buffer_len>100000000)
		{
			//deal pack
			uint8_t *ptr = buffer;
			WFCTAEventDecode wfctaDecode(ptr, buffer_len);
			unsigned long keep_size = 50000000;
			map<short, int>* sipm_position;
			map<short, int>::iterator sipm_position_iter;
			while(1)
			{
				wfctaDecode.nextpack();
				if(0==wfctaDecode.packtype())
				{
					Time1 = wfctaDecode.RabbitTime();
					time1 = wfctaDecode.Rabbittime();

					merge_evt.EventInitial();
					merge_evt.eEvent=wfctaDecode.eventId();
					merge_evt.rabbitTime=wfctaDecode.RabbitTime();
					merge_evt.rabbittime=wfctaDecode.Rabbittime();
					merge_evt.big_pack_lenth = wfctaDecode.bigpackLen();
					merge_evt.n_fired = wfctaDecode.nFired();
					wfctaDecode.Find_SiPMs();
					sipm_position = &(wfctaDecode.GetSiPM_Position());
					merge_evt.packCheck = wfctaDecode.PackCheck();
					printf("bigpackCheck:%d eEvent:%lld\n\n",wfctaDecode.PackCheck(),merge_evt.eEvent);
					for(sipm_position_iter=sipm_position->begin(); sipm_position_iter!=sipm_position->end(); sipm_position_iter++){
						merge_evt.n_Channel++;
						short ISIPM = sipm_position_iter->first;
						merge_evt.IsData[ISIPM] = 1;
						merge_evt.eevent[ISIPM] = wfctaDecode.eventId_in_channel(sipm_position_iter->first);
						merge_evt.zipmod[ISIPM] = wfctaDecode.zipMode(sipm_position_iter->first);
						merge_evt.Over_Single_Marker[ISIPM] = wfctaDecode.GetOver_Single_Mark(sipm_position_iter->first);
						merge_evt.Over_Record_Marker[ISIPM] = wfctaDecode.GetOver_Record_Mark(sipm_position_iter->first);
						merge_evt.winsum[ISIPM] = wfctaDecode.Getwinsum(sipm_position_iter->first);
						wfctaDecode.GetWaveForm(ISIPM,(int *)(merge_evt.pulsehigh), (int *)(merge_evt.pulselow));
						wfctaDecode.GeteSaturation(ISIPM,(int *)(merge_evt.saturationH), (int *)(merge_evt.saturationL));
					}

					//if(merge_evts.size()>0 && (Time1-Time0)*1000000000+(time1-time0)*20>1600)
					if(merge_evts.size()>0 && ((Time1-Time0)*1000000000+(time1-time0)*20>1600 || (Time1-Time0)*1000000000+(time1-time0)*20<1600))
					{
						wfctaEvent->iTel = ITEL;
						wfctaEvent->merge_size = merge_evts.size();
						nevent[ITEL]++;
						wfctaEvent->iEvent=nevent[ITEL];
						wfctaEvent->eEvent=WFCTAMerge::GeteEvent(merge_evts);
						wfctaEvent->rabbitTime=WFCTAMerge::RabbitTime(merge_evts);
						wfctaEvent->rabbittime=WFCTAMerge::Rabbittime(merge_evts);
						wfctaEvent->big_pack_lenth=WFCTAMerge::GetBigPackLen(merge_evts);
						wfctaEvent->n_fired=WFCTAMerge::GetNFired(merge_evts);
						wfctaEvent->n_Channel=WFCTAMerge::GetNChannel(merge_evts);
						for(merge_evts_iter=merge_evts.begin(); merge_evts_iter!=merge_evts.end(); merge_evts_iter++)
						{
							wfctaEvent->packCheck.push_back( (*merge_evts_iter).packCheck );
						}
						for(int isipm=0;isipm<1024;isipm++)
						{
							if(!WFCTAMerge::IsData_Merge(isipm,merge_evts)){continue;}
							if(WFCTAMerge::WimSum_Merge(isipm,merge_evts)<100){continue;}
							if(ITEL==5) {
								wfctaEvent->iSiPM.push_back( 1023 - isipm );
							}
							else        {
								wfctaEvent->iSiPM.push_back( isipm );
							}
							wfctaEvent->eevent.push_back( WFCTAMerge::eevent_Merge(isipm,merge_evts) );
							wfctaEvent->zipmod.push_back( WFCTAMerge::zipmod_Merge(isipm,merge_evts) );
							wfctaEvent->Over_Single_Marker.push_back( WFCTAMerge::OvSigMarker_Merge(isipm,merge_evts) );
							wfctaEvent->Over_Record_Marker.push_back( WFCTAMerge::OvRecMarker_Merge(isipm,merge_evts) );
							wfctaEvent->winsum.push_back( WFCTAMerge::WimSum_Merge(isipm,merge_evts) );
							wfctaEvent->eSatH.push_back( WFCTAMerge::eSatH_Merge(isipm,merge_evts) );
							wfctaEvent->eSatL.push_back( WFCTAMerge::eSatL_Merge(isipm,merge_evts) );
							wfctaEvent->PeakPosH.push_back( WFCTAMerge::GetPeakPosH(isipm,merge_evts) );
							wfctaEvent->PeakPosL.push_back( WFCTAMerge::GetPeakPosL(isipm,merge_evts) );
							wfctaEvent->PeakAmH.push_back( WFCTAMerge::GetPeakAmpH(isipm,merge_evts) );
							wfctaEvent->PeakAmL.push_back( WFCTAMerge::GetPeakAmpL(isipm,merge_evts) );
							WFCTAMerge::Calc_Q_Base(isipm,merge_evts,0);
							wfctaEvent->BaseH.push_back( WFCTAMerge::GetBaseH(isipm,merge_evts) );
							wfctaEvent->BaseL.push_back( WFCTAMerge::GetBaseL(isipm,merge_evts) );
							wfctaEvent->BaseHRMS.push_back( WFCTAMerge::GetBaseHRMS(isipm,merge_evts) );
							wfctaEvent->BaseLRMS.push_back( WFCTAMerge::GetBaseLRMS(isipm,merge_evts) );
							adch = WFCTAMerge::GetAdcH(isipm,merge_evts);
							adcl = WFCTAMerge::GetAdcL(isipm,merge_evts);
							wfctaEvent->AdcH.push_back( adch );
							wfctaEvent->AdcL.push_back( adcl );
							if(adch>6000){  wfctaEvent->SatH.push_back(1);}
							else         {  wfctaEvent->SatH.push_back(0);}
							if(adcl>6000){    wfctaEvent->SatL.push_back(1);}
							else         {    wfctaEvent->SatL.push_back(0);}
							WFCTAMerge::Calc_Q_Base(isipm,merge_evts,1);
							wfctaEvent->LaserBaseH.push_back( WFCTAMerge::GetLaserBaseH(isipm,merge_evts) );
							wfctaEvent->LaserBaseL.push_back( WFCTAMerge::GetLaserBaseL(isipm,merge_evts) );
							wfctaEvent->LaserBaseHRMS.push_back( WFCTAMerge::GetLaserBaseHRMS(isipm,merge_evts) );
							wfctaEvent->LaserBaseLRMS.push_back( WFCTAMerge::GetLaserBaseLRMS(isipm,merge_evts) );
							wfctaEvent->LaserAdcH.push_back( WFCTAMerge::GetLaserAdcH(isipm,merge_evts) );
							wfctaEvent->LaserAdcL.push_back( WFCTAMerge::GetLaserAdcL(isipm,merge_evts) );
						}

						eventShow->Fill();
						merge_evts.clear();
						wfctaEvent->EventInitial();
					}
					merge_evts.push_back(merge_evt);
					Time0 = Time1;
					time0 = time1;
				}

				buffer_len = wfctaDecode.buffer_len_remain();
				if(buffer_len<=keep_size)
					break;
			}

			memcpy(buffer,ptr+buffer_len_all-buffer_len,buffer_len);
		}
	}

	//deal remainder pack
	uint8_t *ptr = buffer;
	WFCTAEventDecode wfctaDecode(ptr, buffer_len);
	unsigned long keep_size = 0;
	map<short, int>* sipm_position;
	map<short, int>::iterator sipm_position_iter;
	while(1)
	{
		wfctaDecode.nextpack();
		if(0==wfctaDecode.packtype())
		{
			Time1 = wfctaDecode.RabbitTime();
			time1 = wfctaDecode.Rabbittime();

			merge_evt.EventInitial();
			merge_evt.eEvent=wfctaDecode.eventId();
			merge_evt.rabbitTime=wfctaDecode.RabbitTime();
			merge_evt.rabbittime=wfctaDecode.Rabbittime();
			merge_evt.big_pack_lenth = wfctaDecode.bigpackLen();
			merge_evt.n_fired = wfctaDecode.nFired();
			wfctaDecode.Find_SiPMs();
			sipm_position = &(wfctaDecode.GetSiPM_Position());
			merge_evt.packCheck = wfctaDecode.PackCheck();
			printf("bigpackCheck:%d eEvent:%lld\n\n",wfctaDecode.PackCheck(),merge_evt.eEvent);
			for(sipm_position_iter=sipm_position->begin(); sipm_position_iter!=sipm_position->end(); sipm_position_iter++){
				merge_evt.n_Channel++;
				short ISIPM = sipm_position_iter->first;
				merge_evt.IsData[ISIPM] = 1;
				merge_evt.eevent[ISIPM] = wfctaDecode.eventId_in_channel(sipm_position_iter->first);
				merge_evt.zipmod[ISIPM] = wfctaDecode.zipMode(sipm_position_iter->first);
				merge_evt.Over_Single_Marker[ISIPM] = wfctaDecode.GetOver_Single_Mark(sipm_position_iter->first);
				merge_evt.Over_Record_Marker[ISIPM] = wfctaDecode.GetOver_Record_Mark(sipm_position_iter->first);
				merge_evt.winsum[ISIPM] = wfctaDecode.Getwinsum(sipm_position_iter->first);
				wfctaDecode.GetWaveForm(ISIPM,(int *)(merge_evt.pulsehigh), (int *)(merge_evt.pulselow));
				wfctaDecode.GeteSaturation(ISIPM,(int *)(merge_evt.saturationH), (int *)(merge_evt.saturationL));
			}

			if(merge_evts.size()>0 && ((Time1-Time0)*1000000000+(time1-time0)*20>1600 || (Time1-Time0)*1000000000+(time1-time0)*20<1600))
			{
				wfctaEvent->iTel = ITEL;
				wfctaEvent->merge_size = merge_evts.size();
				nevent[ITEL]++;
				wfctaEvent->iEvent=nevent[ITEL];
				wfctaEvent->eEvent=WFCTAMerge::GeteEvent(merge_evts);
				wfctaEvent->rabbitTime=WFCTAMerge::RabbitTime(merge_evts);
				wfctaEvent->rabbittime=WFCTAMerge::Rabbittime(merge_evts);
				wfctaEvent->big_pack_lenth=WFCTAMerge::GetBigPackLen(merge_evts);
				wfctaEvent->n_fired=WFCTAMerge::GetNFired(merge_evts);
				wfctaEvent->n_Channel=WFCTAMerge::GetNChannel(merge_evts);
				for(merge_evts_iter=merge_evts.begin(); merge_evts_iter!=merge_evts.end(); merge_evts_iter++)
				{
					wfctaEvent->packCheck.push_back( (*merge_evts_iter).packCheck );
				}
				for(int isipm=0;isipm<1024;isipm++)
				{
					if(!WFCTAMerge::IsData_Merge(isipm,merge_evts)){continue;}
					if(WFCTAMerge::WimSum_Merge(isipm,merge_evts)<100){continue;}
					if(ITEL==5) {
						wfctaEvent->iSiPM.push_back( 1023 - isipm );
					}
					else        {
						wfctaEvent->iSiPM.push_back( isipm );
					}
					wfctaEvent->eevent.push_back( WFCTAMerge::eevent_Merge(isipm,merge_evts) );
					wfctaEvent->zipmod.push_back( WFCTAMerge::zipmod_Merge(isipm,merge_evts) );
					wfctaEvent->Over_Single_Marker.push_back( WFCTAMerge::OvSigMarker_Merge(isipm,merge_evts) );
					wfctaEvent->Over_Record_Marker.push_back( WFCTAMerge::OvRecMarker_Merge(isipm,merge_evts) );
					wfctaEvent->winsum.push_back( WFCTAMerge::WimSum_Merge(isipm,merge_evts) );
					wfctaEvent->eSatH.push_back( WFCTAMerge::eSatH_Merge(isipm,merge_evts) );
					wfctaEvent->eSatL.push_back( WFCTAMerge::eSatL_Merge(isipm,merge_evts) );
					wfctaEvent->PeakPosH.push_back( WFCTAMerge::GetPeakPosH(isipm,merge_evts) );
					wfctaEvent->PeakPosL.push_back( WFCTAMerge::GetPeakPosL(isipm,merge_evts) );
					wfctaEvent->PeakAmH.push_back( WFCTAMerge::GetPeakAmpH(isipm,merge_evts) );
					wfctaEvent->PeakAmL.push_back( WFCTAMerge::GetPeakAmpL(isipm,merge_evts) );
					WFCTAMerge::Calc_Q_Base(isipm,merge_evts,0);
					wfctaEvent->BaseH.push_back( WFCTAMerge::GetBaseH(isipm,merge_evts) );
					wfctaEvent->BaseL.push_back( WFCTAMerge::GetBaseL(isipm,merge_evts) );
					wfctaEvent->BaseHRMS.push_back( WFCTAMerge::GetBaseHRMS(isipm,merge_evts) );
					wfctaEvent->BaseLRMS.push_back( WFCTAMerge::GetBaseLRMS(isipm,merge_evts) );
					adch = WFCTAMerge::GetAdcH(isipm,merge_evts);
					adcl = WFCTAMerge::GetAdcL(isipm,merge_evts);
					wfctaEvent->AdcH.push_back( adch );
					wfctaEvent->AdcL.push_back( adcl );
					if(adch>6000){  wfctaEvent->SatH.push_back(1);}
					else         {  wfctaEvent->SatH.push_back(0);}
					if(adcl>6000){    wfctaEvent->SatL.push_back(1);}
					else         {    wfctaEvent->SatL.push_back(0);}
					WFCTAMerge::Calc_Q_Base(isipm,merge_evts,1);
					wfctaEvent->LaserBaseH.push_back( WFCTAMerge::GetLaserBaseH(isipm,merge_evts) );
					wfctaEvent->LaserBaseL.push_back( WFCTAMerge::GetLaserBaseL(isipm,merge_evts) );
					wfctaEvent->LaserBaseHRMS.push_back( WFCTAMerge::GetLaserBaseHRMS(isipm,merge_evts) );
					wfctaEvent->LaserBaseLRMS.push_back( WFCTAMerge::GetLaserBaseLRMS(isipm,merge_evts) );
					wfctaEvent->LaserAdcH.push_back( WFCTAMerge::GetLaserAdcH(isipm,merge_evts) );
					wfctaEvent->LaserAdcL.push_back( WFCTAMerge::GetLaserAdcL(isipm,merge_evts) );
				}

				eventShow->Fill();
				merge_evts.clear();
				wfctaEvent->EventInitial();
			}
			merge_evts.push_back(merge_evt);
			Time0 = Time1;
			time0 = time1;
		}

		buffer_len = wfctaDecode.buffer_len_remain();
		if(buffer_len<=keep_size)
			break;
	}
	//delete wfctaEvent;
	//delete[] bufferRaw;
	//delete[] buffertemp;
	//delete[] buffer;
	//delete[] FileHeadBuffer;
	fdata.close();
	delete wfctaEvent;
	delete[] bufferRaw;

	// ****************************************************************************** //
	rootfile->Write();
	rootfile->Close();


}
