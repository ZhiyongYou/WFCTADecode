#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include "include/dumpPack.h"
#include "include/WFCTAEvent.h"
#include "include/WFCTAMerge.h"
#include "include/WFCTADecode.h"
#include <TFile.h>
#include <TTree.h>

#define BUF_LEN 2000000
#define STATUS_BUF_LEN 2000000

using namespace std;

int main(int argc, char**argv)
{
	if(argc!=3)
	{
		printf("Use %s iptpath/inputfile outpath/outfile\n",argv[0]);
		return 0;
	}
	//int eventTEST=atoi(argv[3]);
	FILE *fp;
	uint8_t *buf = NULL;// = new uint8_t[BUF_LEN];
	int32_t slicelength = 2000000;
	size_t size_of_read;
	short ITEL;
	int FEEDataHead;
	long FEEPos;

	int64_t packStart = 0;
	map<short, int>* sipm_position;
	map<short, int>::iterator sipm_position_iter;

	char Name1[300]="root://eos01.ihep.ac.cn/";
	char Name2[300];
	strcpy(Name2,Name1);
	strcat(Name2,argv[2]);
	TFile *rootfile = TFile::Open(Name2,"recreate");
	//TFile *rootfile = new TFile(argv[2],"recreate");
	//int nPulse;
	vector<int>* nPulse = new vector<int>();
	vector<int>* sipmId = new vector<int>();
	vector<int>* iPulse = new vector<int>();
	vector<int>* pulseH = new vector<int>();
	vector<int>* pulseL = new vector<int>();
	WFCTAEvent *wfctaEvent = new WFCTAEvent();
	/*********************************************************************/
	TTree *eventShow = new TTree("eventShow","info of evnets");
	wfctaEvent -> CreateBranch(eventShow,1);
	//eventShow -> Branch("nPulse",&nPulse,"nPulse/I");
	eventShow -> Branch("nPulse","vector<int>",&nPulse);
	eventShow -> Branch("sipmId","vector<int>",&sipmId);
	eventShow -> Branch("iPulse","vector<int>",&iPulse);
	eventShow -> Branch("pulseH","vector<int>",&pulseH);
	eventShow -> Branch("pulseL","vector<int>",&pulseL);
	//eventShow -> Branch("iPulse",iPulse,"iPulse[nPulse]/I");
	//eventShow -> Branch("pulseH",pulseH,"pulseH[1024][nPulse]/I");
	//eventShow -> Branch("pulseL",pulseL,"pulseL[1024][nPulse]/I");
	//eventShow -> Branch("iPulse",iPulse,"iPulse[1400]/I");
	//eventShow -> Branch("pulseH",pulseH,"pulseH[1024][1400]/I");
	//eventShow -> Branch("pulseL",pulseL,"pulseL[1024][1400]/I");
	/*********************************************************************/

	WFCTADecode *wfctaDecode = new WFCTADecode();
	//Events Initial//
	wfctaEvent->EventInitial();
	//nPulse=0;
	nPulse->clear();
	sipmId->clear();
	iPulse->clear();
	pulseH->clear();
	pulseL->clear();
	/*
	for(int i=0;i<1024;i++)
	{
		for(int ipulse=0;ipulse<1400;ipulse++)
		{
			nPulse=0;
			iPulse[i] = 0;
			pulseH[i][ipulse]=0;
			pulseL[i][ipulse]=0;
		}
	}
	*/

	fp = fopen(argv[1],"rb");
	vector<long>* rb_Time = new vector<long>();
	vector<double>* rb_time = new vector<double>();
	vector<long>* pack_pos = new vector<long>();
	vector<long>* pack_len = new vector<long>();
	rb_Time->clear();
	rb_time->clear();
	pack_pos->clear();
	pack_len->clear();
	while(true)
	{
		buf = new uint8_t[40];
		size_of_read = fread((uint8_t *)buf,1,40,fp);
		if(size_of_read==0){break;}

		if(wfctaDecode->FEEDataFragment(buf))
		{
			FEEDataHead = wfctaDecode->feeDataHead();
			slicelength = wfctaDecode->sliceLength(buf,FEEDataHead); 
			ITEL = wfctaDecode->Telid(buf,FEEDataHead);
			fseek(fp,-size_of_read+FEEDataHead,1);
			FEEPos = ftell(fp);
			delete[] buf;
			buf = new uint8_t[slicelength];
			size_of_read = fread((uint8_t *)buf,1,slicelength,fp);
			packStart = 0;
			while(1)
			{
				int bigpackcheck = wfctaDecode->bigPackCheck(buf,int(size_of_read),packStart);
				if(bigpackcheck == 1)
				{
					//get info rabbit_time and position in file//
					rb_Time->push_back( wfctaDecode->RabbitTime(buf) );
					rb_time->push_back( wfctaDecode->Rabbittime(buf) );
					pack_pos->push_back( FEEPos + packStart );
					pack_len->push_back( wfctaDecode->bigpackLen() );
					packStart = wfctaDecode->PackSize();
				}
				else if(bigpackcheck == 2)
				{
					printf("bigpack is wrong\n");
					packStart = wfctaDecode->PackSize();
				}
				else
				{
					break;
				}
			} 
			delete[] buf;
		}
		else
		{
			delete[] buf;
			fseek(fp,-size_of_read+20,1);
		}
	}
	fclose(fp);

	long rb_TimeMin = *min_element(rb_Time->begin(),rb_Time->end());
	printf("TimeMin:%lld\n\n",rb_TimeMin);
	map<long, long> time_position;
	map<long, long>::iterator time_position_iter;
	map<long, long>::iterator next_time_position_iter;
	for(int ii=0;ii<rb_Time->size();ii++){
		time_position.insert( pair<long,long>( long((rb_Time->at(ii)-rb_TimeMin)*1000000000+rb_time->at(ii)*20), pack_pos->at(ii) ) );
	}

	float adch;
	float adcl;
	long deltaTime;
	int nevent[20]={0};
	short ISIPM;
	WFCTAMerge merge_ev;
	vector<WFCTAMerge> merge_evs;
	vector<WFCTAMerge>::iterator merge_evs_iter;
	merge_evs.clear();

	slicelength = 2000000;
	fp = fopen(argv[1],"rb");
	next_time_position_iter=time_position.begin();
	//next_time_position_iter++;
	int lastevent=0;
	int allevents=time_position.size();
	int thisevents=0;
	for(time_position_iter=time_position.begin(); time_position_iter!=time_position.end(); time_position_iter++)
	{
		thisevents++;
		if(next_time_position_iter!=time_position.end())
		{
			next_time_position_iter++;
		}
		if(thisevents==allevents)
		{
			printf("last event\n");
			lastevent=1;
		}

		fseek(fp,time_position_iter->second,0);
		buf = new uint8_t[slicelength];
		fread((uint8_t *)buf,1,slicelength,fp);

		//dumpPacket(buf,36,16);
		merge_ev.EventInitial();
		wfctaDecode->bigPackCheck(buf,int(slicelength),0);
		merge_ev.eEvent=wfctaDecode->eventId(buf);
		//if(merge_ev.eEvent!=eventTEST){continue;}
		merge_ev.rabbitTime=wfctaDecode->RabbitTime(buf);
		merge_ev.rabbittime=wfctaDecode->Rabbittime(buf);
		merge_ev.big_pack_lenth = wfctaDecode->bigpackLen();
		merge_ev.n_fired = wfctaDecode->nFired(buf);
		//find sipms and their position in this pack//
		wfctaDecode->Find_SiPMs(buf);//,0);
		sipm_position = &(wfctaDecode->GetSiPM_Position());
		merge_ev.packCheck = wfctaDecode->PackCheck();
		printf("bigpackCheck:%d eEvent:%lld\n\n",wfctaDecode->PackCheck(),merge_ev.eEvent);
		//get info of each sipm: q, base, peakposition...//
		for(sipm_position_iter=sipm_position->begin(); sipm_position_iter!=sipm_position->end(); sipm_position_iter++){
			merge_ev.n_Channel++;
			ISIPM = sipm_position_iter->first;
			merge_ev.IsData[ISIPM] = 1;
			merge_ev.eevent[ISIPM] = wfctaDecode->eventId_in_channel(buf,sipm_position_iter->first);
			merge_ev.zipmod[ISIPM] = wfctaDecode->zipMode(buf,sipm_position_iter->first);
			merge_ev.Over_Single_Marker[ISIPM] = wfctaDecode->GetOver_Single_Mark(buf,sipm_position_iter->first);
			merge_ev.Over_Record_Marker[ISIPM] = wfctaDecode->GetOver_Record_Mark(buf,sipm_position_iter->first);
			merge_ev.winsum[ISIPM] = wfctaDecode->Getwinsum(buf,sipm_position_iter->first);
			//wfctaDecode->GetWaveForm(buf,sipm_position_iter->first,(int *)(merge_ev.pulsehigh), (int *)(merge_ev.pulselow));
			//wfctaDecode->GeteSaturation(buf,sipm_position_iter->first,(int *)(merge_ev.saturationH), (int *)(merge_ev.saturationL));
			wfctaDecode->GetWaveForm(buf,short(ISIPM),(int *)(merge_ev.pulsehigh), (int *)(merge_ev.pulselow));
			wfctaDecode->GeteSaturation(buf,short(ISIPM),(int *)(merge_ev.saturationH), (int *)(merge_ev.saturationL));
		}

		merge_evs.push_back(merge_ev);
		deltaTime = next_time_position_iter->first - time_position_iter->first;
		//printf("TimeNext:%lld    TimeThis:%lld    deltaTime:%lld\n",next_time_position_iter->first,time_position_iter->first,deltaTime);

		if(deltaTime>1600 || 1==lastevent)
		{
			//printf("bigpackCheck:%d eEvent:%lld lastevent:%d\n\n",wfctaDecode->PackCheck(),merge_ev.eEvent,lastevent);
			wfctaEvent->iTel = ITEL;
			wfctaEvent->merge_size = merge_evs.size();
			nevent[ITEL]++;
			wfctaEvent->iEvent=nevent[ITEL];
			wfctaEvent->eEvent=WFCTAMerge::GeteEvent(merge_evs);
			wfctaEvent->rabbitTime=WFCTAMerge::RabbitTime(merge_evs);
			wfctaEvent->rabbittime=WFCTAMerge::Rabbittime(merge_evs);
			wfctaEvent->big_pack_lenth=WFCTAMerge::GetBigPackLen(merge_evs);
			wfctaEvent->n_fired=WFCTAMerge::GetNFired(merge_evs);
			wfctaEvent->n_Channel=WFCTAMerge::GetNChannel(merge_evs);
			for(merge_evs_iter=merge_evs.begin(); merge_evs_iter!=merge_evs.end(); merge_evs_iter++)
			{
				wfctaEvent->packCheck.push_back( (*merge_evs_iter).packCheck );
			}
			for(int isipm=0;isipm<1024;isipm++)
			{
				if(!WFCTAMerge::IsData_Merge(isipm,merge_evs)){continue;}
				if(WFCTAMerge::WimSum_Merge(isipm,merge_evs)<100){continue;}
				if(ITEL==5)	{
					//printf("tel5 reverse: event:%d  isipm:%d 1023-isipm:%d\n",wfctaEvent->iEvent,isipm,1023-isipm);
					wfctaEvent->iSiPM.push_back( 1023 - isipm );
				}
				else		{
					//printf("tel%d event:%d  isipm:%d\n",ITEL,wfctaEvent->iEvent,isipm);
					wfctaEvent->iSiPM.push_back( isipm );
				}
				wfctaEvent->eevent.push_back( WFCTAMerge::eevent_Merge(isipm,merge_evs) );
				wfctaEvent->zipmod.push_back( WFCTAMerge::zipmod_Merge(isipm,merge_evs) );
				wfctaEvent->Over_Single_Marker.push_back( WFCTAMerge::OvSigMarker_Merge(isipm,merge_evs) );
				wfctaEvent->Over_Record_Marker.push_back( WFCTAMerge::OvRecMarker_Merge(isipm,merge_evs) );
				wfctaEvent->winsum.push_back( WFCTAMerge::WimSum_Merge(isipm,merge_evs) );
				wfctaEvent->eSatH.push_back( WFCTAMerge::eSatH_Merge(isipm,merge_evs) );
				wfctaEvent->eSatL.push_back( WFCTAMerge::eSatL_Merge(isipm,merge_evs) );
				wfctaEvent->PeakPosH.push_back( WFCTAMerge::GetPeakPosH(isipm,merge_evs) );
				wfctaEvent->PeakPosL.push_back( WFCTAMerge::GetPeakPosL(isipm,merge_evs) );
				wfctaEvent->PeakAmH.push_back( WFCTAMerge::GetPeakAmpH(isipm,merge_evs) );
				wfctaEvent->PeakAmL.push_back( WFCTAMerge::GetPeakAmpL(isipm,merge_evs) );
				WFCTAMerge::Calc_Q_Base(isipm,merge_evs,0);
				wfctaEvent->BaseH.push_back( WFCTAMerge::GetBaseH(isipm,merge_evs) );
				wfctaEvent->BaseL.push_back( WFCTAMerge::GetBaseL(isipm,merge_evs) );
				wfctaEvent->BaseHRMS.push_back( WFCTAMerge::GetBaseHRMS(isipm,merge_evs) );
				wfctaEvent->BaseLRMS.push_back( WFCTAMerge::GetBaseLRMS(isipm,merge_evs) );
				adch = WFCTAMerge::GetAdcH(isipm,merge_evs);
				adcl = WFCTAMerge::GetAdcL(isipm,merge_evs);
				wfctaEvent->AdcH.push_back( adch );
				wfctaEvent->AdcL.push_back( adcl );
				if(adch>6000){  wfctaEvent->SatH.push_back(1);}
				else         {  wfctaEvent->SatH.push_back(0);}
				if(adcl>6000){    wfctaEvent->SatL.push_back(1);}
				else         {    wfctaEvent->SatL.push_back(0);}
				WFCTAMerge::Calc_Q_Base(isipm,merge_evs,1);
				wfctaEvent->LaserBaseH.push_back( WFCTAMerge::GetLaserBaseH(isipm,merge_evs) );
				wfctaEvent->LaserBaseL.push_back( WFCTAMerge::GetLaserBaseL(isipm,merge_evs) );
				wfctaEvent->LaserBaseHRMS.push_back( WFCTAMerge::GetLaserBaseHRMS(isipm,merge_evs) );
				wfctaEvent->LaserBaseLRMS.push_back( WFCTAMerge::GetLaserBaseLRMS(isipm,merge_evs) );
				wfctaEvent->LaserAdcH.push_back( WFCTAMerge::GetLaserAdcH(isipm,merge_evs) );
				wfctaEvent->LaserAdcL.push_back( WFCTAMerge::GetLaserAdcL(isipm,merge_evs) );
				int npulse = WFCTAMerge::GetWaveForm(isipm, *iPulse, *pulseH, *pulseL, merge_evs);
				nPulse->push_back(npulse);
				if(ITEL==5) {
					for(int ipot=0;ipot<npulse;ipot++)
					{
						sipmId->push_back(1023-isipm);
					}
				}
				else        {
					for(int ipot=0;ipot<npulse;ipot++)
					{
						sipmId->push_back(isipm);
					}
				}
				//if(isipm==1){
				//	printf("winsum_merge:%f\n",WFCTAMerge::WimSum_Merge(isipm,merge_evs));
				//}
			}
			//printf("merge event:%d\n\n",merge_evs.size());
			eventShow->Fill();
			merge_evs.clear();
			wfctaEvent->EventInitial();
			nPulse->clear();
			sipmId->clear();
			iPulse->clear();
			pulseH->clear();
			pulseL->clear();
			/*
			   for(int i=0;i<1024;i++)
			   {
			   for(int ipulse=0;ipulse<1400;ipulse++)
			   {
			   nPulse=0;
			   iPulse[i] = 0;
			   pulseH[i][ipulse]=0;
			   pulseL[i][ipulse]=0;
			   }
			   }
			   */
		}

		delete[] buf;
	} 
	fclose(fp);
	/******************************************************************************/
	rootfile->Write();
	rootfile->Close();

	printf("decode finished\n");

}
