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
#include "WFCTAEventDecode.h"

#define BUF_LEN 120000000
#define STATUS_BUF_LEN 120000000
#define EV_FILE_HEAD 256
#define DAQ_EVT_ALL_HEAD 104

int main(int argc, char**argv)
{
	if(argc!=3)
	{
		printf("Use %s iptpath/inputfile outpath/outfile\n",argv[0]);
		return 0;
	}


	short iTel;
	int fpgaVersion[10];
	int f9mode;
	int f9pattern;
	long pat_full;
	long pat_nofull;
	long nopat_full;
	long nopat_nofull;
	int DbVersion[2][89];
	int ClbVersion[2][89];
	long clb_initial_Time;
	double clb_initial_time;
	int fired_tube;
	long status_readback_Time;
	double status_readback_time;
	int sipm[1024];  for(int i=0;i<1024;i++) {sipm[i]=i;}
	int mask[1024];
	short single_thresh[1024];
	short record_thresh[1024];
	long single_count[1024];
	long single_time[1024];
	float DbTemp[1024];
	float HV[1024];
	float PreTemp[1024];
	float BigResistence[1024];
	float SmallResistence[1024];
	long ClbTime[1024];
	float ClbTemp[1024];

	char Name1[300]="root://eos01.ihep.ac.cn/";
	char Name2[300];
	strcpy(Name2,Name1);
	strcat(Name2,argv[2]);
	//strcpy(Name2,argv[2]);
	TFile *rootfile = TFile::Open(Name2,"recreate");
	TTree *Status = new TTree("Status","Status Tree");
	Status -> Branch("iTel",&iTel,"iTel/S");
	Status -> Branch("fpgaVersion",fpgaVersion,"fpgaVersion[10]/I");
	Status -> Branch("f9mode",&f9mode,"f9mode/I");
	Status -> Branch("f9pattern",&f9pattern,"f9pattern/I");
	Status -> Branch("pat_full",&pat_full,"pat_full/L");
	Status -> Branch("pat_nofull",&pat_nofull,"pat_nofull/L");
	Status -> Branch("nopat_full",&nopat_full,"nopat_full/L");
	Status -> Branch("nopat_nofull",&nopat_nofull,"nopat_nofull/L");
	Status -> Branch("DbVersion",DbVersion,"DbVersion[2][89]/I");
	Status -> Branch("ClbVersion",ClbVersion,"ClbVersion[2][89]/I");
	Status -> Branch("clb_initial_Time",&clb_initial_Time,"clb_initial_Time/L");
	Status -> Branch("clb_initial_time",&clb_initial_time,"clb_initial_time/D");
	Status -> Branch("fired_tube",&fired_tube,"fired_tube/I");
	Status -> Branch("status_readback_Time",&status_readback_Time,"status_readback_Time/L");
	Status -> Branch("status_readback_time",&status_readback_time,"status_readback_time/D");
	Status -> Branch("sipm",sipm,"sipm[1024]/I");
	Status -> Branch("mask",mask,"mask[1024]/I");
	Status -> Branch("single_thresh",single_thresh,"single_thresh[1024]/S");
	Status -> Branch("record_thresh",record_thresh,"record_thresh[1024]/S");
	Status -> Branch("single_count",single_count,"single_count[1024]/L");
	Status -> Branch("single_time",single_time,"single_time[1024]/L");
	Status -> Branch("DbTemp",DbTemp,"DbTemp[1024]/F");
	Status -> Branch("HV",HV,"HV[1024]/F");
	Status -> Branch("PreTemp",PreTemp,"PreTemp[1024]/F");
	Status -> Branch("BigResistence",BigResistence,"BigResistence[1024]/F");
	Status -> Branch("SmallResistence",SmallResistence,"SmallResistence[1024]/F");
	Status -> Branch("ClbTime",ClbTime,"ClbTime[1024]/L");
	Status -> Branch("ClbTemp",ClbTemp,"ClbTemp[1024]/F");

	int dbnumber;
	int clbnumber;
	int mask5[1024]={0};
	short single_thresh5[1024]={0};
	short record_thresh5[1024]={0};
	long single_count5[1024]={0};
	long single_time5[1024]={0};
	float DbTemp5[1024]={0};
	float HV5[1024]={0};
	float PreTemp5[1024]={0};
	float BigResistence5[1024]={0};
	float SmallResistence5[1024]={0};
	long ClbTime5[1024]={0};
	float ClbTemp5[1024]={0};
	//initial
	f9mode = -1000;
	f9pattern = -1000;
	pat_full=-1000;
	pat_nofull=-1000;
	nopat_full=-1000;
	nopat_nofull=-1000;
	clb_initial_Time = -1000;
	clb_initial_time = -1000;
	fired_tube = -1000;
	status_readback_Time = -1000;
	status_readback_time = -1000;
	for(int i=0;i<1024;i++){
		mask[i] = -1000;
		single_thresh[i] = -1000;
		record_thresh[i] = -1000;
		single_count[i] = -1000;
		single_time[i] = -1000;
		DbTemp[i] = -1000;
		HV[i] = -1000;
		PreTemp[i] = -1000;
		BigResistence[i] = -1000;
		SmallResistence[i] = -1000;
		ClbTime[i] = -1000;
		ClbTemp[i] = -1000;
	}
	for(int i=0;i<10;i++){
		fpgaVersion[i] = -1000;
	}
	for(int i=0;i<2;i++){
		for(int j=0;j<89;j++){
			DbVersion[i][j] = -1000;
			ClbVersion[i][j] = -1000;
		}
	}


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
	uint8_t status_pack_marker;
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
				if(1==wfctaDecode.packtype())
				{
					bool statuspackloop = true;
					while(statuspackloop)
					{
						//wfctaDecode.dump_pack();
						status_pack_marker = wfctaDecode.NextStatusLittlePack();
						printf("status_pack_marker: %d\n",status_pack_marker);
						switch(status_pack_marker){
							case 0x21:
								wfctaDecode.Getthresh(single_thresh, record_thresh);
								break;
							case 0x22:
								wfctaDecode.Deal22Pack(single_count);
								break;
							case 0x23:
								dbnumber = wfctaDecode.GetDBNumber();
								DbVersion[0][dbnumber] = dbnumber;
								DbVersion[1][dbnumber] = wfctaDecode.GetDBVersion();
								wfctaDecode.Deal23Pack(single_count,single_time);
								break;
							case 0x81:
								wfctaDecode.GetHV(HV);
								break;
							case 0x82:
								wfctaDecode.GetPreTemp(PreTemp);
								break;
							case 0x83:
								wfctaDecode.GetBigRes(BigResistence);
								break;
							case 0x84:
								wfctaDecode.GetSmallRes(SmallResistence);
								break;
							case 0x85:
								clbnumber = wfctaDecode.GetClbNumber();
								ClbVersion[0][clbnumber] = clbnumber;
								ClbVersion[1][clbnumber] = wfctaDecode.GetClbVersion();
								wfctaDecode.GetClbTemp(ClbTemp);
								break;
							case 0x9:
								iTel = ITEL;
								printf("itel:%d-----\n\n",iTel);
								f9mode		=	wfctaDecode.GetF9Mode();
								f9pattern	=	wfctaDecode.GetPattern();
								pat_full	=	wfctaDecode.GetPat_Full();
								pat_nofull	=	wfctaDecode.GetPat_noFull();
								nopat_full	=	wfctaDecode.GetnoPat_Full();
								nopat_nofull=	wfctaDecode.GetnoPat_noFull();
								fpgaVersion[9] = wfctaDecode.GetF9Version();
								fpgaVersion[0] = wfctaDecode.GetF9PlusVersion();
								clb_initial_Time = wfctaDecode.GetclbInitialTime();
								clb_initial_time = wfctaDecode.GetclbInitialtime();
								fired_tube = wfctaDecode.GetFiredTube();
								status_readback_Time = wfctaDecode.GetStatusReadbackTime();
								status_readback_time = wfctaDecode.GetStatusReadbacktime();
								if(iTel==5)
								{
									for(int i=0;i<1024;i++){
										mask5[i] = mask[1023-i];
										single_thresh5[i] = single_thresh[1023-i];
										record_thresh5[i] = record_thresh[1023-i];
										single_count5[i] = single_count[1023-i];
										single_time5[i] = single_time[1023-i];
										DbTemp5[i] = DbTemp[1023-i];
										HV5[i] = HV[1023-i];
										PreTemp5[i] = PreTemp[1023-i];
										BigResistence5[i] = BigResistence[1023-i];
										SmallResistence5[i] = SmallResistence[1023-i];
										ClbTime5[i] = ClbTime[1023-i];
										ClbTemp5[i] = ClbTemp[1023-i];
									}
									for(int i=0;i<1024;i++){
										mask[i] = mask5[i];
										single_thresh[i] = single_thresh5[i];
										record_thresh[i] = record_thresh5[i];
										single_count[i] = single_count5[i];
										single_time[i] = single_time5[i];
										DbTemp[i] = DbTemp5[i];
										HV[i] = HV5[i];
										PreTemp[i] = PreTemp5[i];
										BigResistence[i] = BigResistence5[i];
										SmallResistence[i] = SmallResistence5[i];
										ClbTime[i] = ClbTime5[i];
										ClbTemp[i] = ClbTemp5[i];
									}
								}
								Status->Fill();
								f9mode = -1000;
								f9pattern = -1000;
								pat_full=-1000;
								pat_nofull=-1000;
								nopat_full=-1000;
								nopat_nofull=-1000;
								clb_initial_Time = -1000;
								clb_initial_time = -1000;
								fired_tube = -1000;
								status_readback_Time = -1000;
								status_readback_time = -1000;
								for(int i=0;i<1024;i++){
									mask[i] = -1000;
									single_thresh[i] = -1000;
									record_thresh[i] = -1000;
									single_count[i] = -1000;
									single_time[i] = -1000;
									DbTemp[i] = -1000;
									HV[i] = -1000;
									PreTemp[i] = -1000;
									BigResistence[i] = -1000;
									SmallResistence[i] = -1000;
									ClbTime[i] = -1000;
									ClbTemp[i] = -1000;
								}
								for(int i=0;i<10;i++){
									fpgaVersion[i] = -1000;
								}
								for(int i=0;i<2;i++){
									for(int j=0;j<89;j++){
										DbVersion[i][j] = -1000;
										ClbVersion[i][j] = -1000;
									}
								}
								statuspackloop = false;
								break;
							case 100:
								iTel = ITEL;
								if(iTel==5)
								{
									for(int i=0;i<1024;i++){
										mask5[i] = mask[1023-i];
										single_thresh5[i] = single_thresh[1023-i];
										record_thresh5[i] = record_thresh[1023-i];
										single_count5[i] = single_count[1023-i];
										single_time5[i] = single_time[1023-i];
										DbTemp5[i] = DbTemp[1023-i];
										HV5[i] = HV[1023-i];
										PreTemp5[i] = PreTemp[1023-i];
										BigResistence5[i] = BigResistence[1023-i];
										SmallResistence5[i] = SmallResistence[1023-i];
										ClbTime5[i] = ClbTime[1023-i];
										ClbTemp5[i] = ClbTemp[1023-i];
									}
									for(int i=0;i<1024;i++){
										mask[i] = mask5[i];
										single_thresh[i] = single_thresh5[i];
										record_thresh[i] = record_thresh5[i];
										single_count[i] = single_count5[i];
										single_time[i] = single_time5[i];
										DbTemp[i] = DbTemp5[i];
										HV[i] = HV5[i];
										PreTemp[i] = PreTemp5[i];
										BigResistence[i] = BigResistence5[i];
										SmallResistence[i] = SmallResistence5[i];
										ClbTime[i] = ClbTime5[i];
										ClbTemp[i] = ClbTemp5[i];
									}
								}
								Status->Fill();
								f9mode = -1000;
								f9pattern = -1000;
								pat_full=-1000;
								pat_nofull=-1000;
								nopat_full=-1000;
								nopat_nofull=-1000;
								clb_initial_Time = -1000;
								clb_initial_time = -1000;
								fired_tube = -1000;
								status_readback_Time = -1000;
								status_readback_time = -1000;
								for(int i=0;i<1024;i++){
									mask[i] = -1000;
									single_thresh[i] = -1000;
									record_thresh[i] = -1000;
									single_count[i] = -1000;
									single_time[i] = -1000;
									DbTemp[i] = -1000;
									HV[i] = -1000;
									PreTemp[i] = -1000;
									BigResistence[i] = -1000;
									SmallResistence[i] = -1000;
									ClbTime[i] = -1000;
									ClbTemp[i] = -1000;
								}
								for(int i=0;i<10;i++){
									fpgaVersion[i] = -1000;
								}
								for(int i=0;i<2;i++){
									for(int j=0;j<89;j++){
										DbVersion[i][j] = -1000;
										ClbVersion[i][j] = -1000;
									}
								}
								statuspackloop = false;
								printf("statuspackloop:%d itel:%d---\n\n",statuspackloop,iTel);
								break;
							default:
								fpgaVersion[status_pack_marker] = wfctaDecode.GetF18Version();
								wfctaDecode.GetMask(status_pack_marker, (int *)mask);
								break;
						}
					}
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
		if(1==wfctaDecode.packtype())
		{
			bool statuspackloop = true;
			while(statuspackloop)
			{
				//wfctaDecode.dump_pack();
				status_pack_marker = wfctaDecode.NextStatusLittlePack();
				printf("status_pack_marker: %d\n",status_pack_marker);
				switch(status_pack_marker){
					case 0x21:
						wfctaDecode.Getthresh(single_thresh, record_thresh);
						break;
					case 0x22:
						wfctaDecode.Deal22Pack(single_count);
						break;
					case 0x23:
						dbnumber = wfctaDecode.GetDBNumber();
						DbVersion[0][dbnumber] = dbnumber;
						DbVersion[1][dbnumber] = wfctaDecode.GetDBVersion();
						wfctaDecode.Deal23Pack(single_count,single_time);
						break;
					case 0x81:
						wfctaDecode.GetHV(HV);
						break;
					case 0x82:
						wfctaDecode.GetPreTemp(PreTemp);
						break;
					case 0x83:
						wfctaDecode.GetBigRes(BigResistence);
						break;
					case 0x84:
						wfctaDecode.GetSmallRes(SmallResistence);
						break;
					case 0x85:
						clbnumber = wfctaDecode.GetClbNumber();
						ClbVersion[0][clbnumber] = clbnumber;
						ClbVersion[1][clbnumber] = wfctaDecode.GetClbVersion();
						wfctaDecode.GetClbTemp(ClbTemp);
						break;
					case 0x9:
						iTel = ITEL;
						printf("itel:%d-----\n\n",iTel);
						f9mode		=	wfctaDecode.GetF9Mode();
						f9pattern	=	wfctaDecode.GetPattern();
						pat_full	=	wfctaDecode.GetPat_Full();
						pat_nofull	=	wfctaDecode.GetPat_noFull();
						nopat_full	=	wfctaDecode.GetnoPat_Full();
						nopat_nofull=	wfctaDecode.GetnoPat_noFull();
						fpgaVersion[9] = wfctaDecode.GetF9Version();
						fpgaVersion[0] = wfctaDecode.GetF9PlusVersion();
						clb_initial_Time = wfctaDecode.GetclbInitialTime();
						clb_initial_time = wfctaDecode.GetclbInitialtime();
						fired_tube = wfctaDecode.GetFiredTube();
						status_readback_Time = wfctaDecode.GetStatusReadbackTime();
						status_readback_time = wfctaDecode.GetStatusReadbacktime();
						if(iTel==5)
						{
							for(int i=0;i<1024;i++){
								mask5[i] = mask[1023-i];
								single_thresh5[i] = single_thresh[1023-i];
								record_thresh5[i] = record_thresh[1023-i];
								single_count5[i] = single_count[1023-i];
								single_time5[i] = single_time[1023-i];
								DbTemp5[i] = DbTemp[1023-i];
								HV5[i] = HV[1023-i];
								PreTemp5[i] = PreTemp[1023-i];
								BigResistence5[i] = BigResistence[1023-i];
								SmallResistence5[i] = SmallResistence[1023-i];
								ClbTime5[i] = ClbTime[1023-i];
								ClbTemp5[i] = ClbTemp[1023-i];
							}
							for(int i=0;i<1024;i++){
								mask[i] = mask5[i];
								single_thresh[i] = single_thresh5[i];
								record_thresh[i] = record_thresh5[i];
								single_count[i] = single_count5[i];
								single_time[i] = single_time5[i];
								DbTemp[i] = DbTemp5[i];
								HV[i] = HV5[i];
								PreTemp[i] = PreTemp5[i];
								BigResistence[i] = BigResistence5[i];
								SmallResistence[i] = SmallResistence5[i];
								ClbTime[i] = ClbTime5[i];
								ClbTemp[i] = ClbTemp5[i];
							}
						}
						Status->Fill();
						f9mode = -1000;
						f9pattern = -1000;
						pat_full=-1000;
						pat_nofull=-1000;
						nopat_full=-1000;
						nopat_nofull=-1000;
						clb_initial_Time = -1000;
						clb_initial_time = -1000;
						fired_tube = -1000;
						status_readback_Time = -1000;
						status_readback_time = -1000;
						for(int i=0;i<1024;i++){
							mask[i] = -1000;
							single_thresh[i] = -1000;
							record_thresh[i] = -1000;
							single_count[i] = -1000;
							single_time[i] = -1000;
							DbTemp[i] = -1000;
							HV[i] = -1000;
							PreTemp[i] = -1000;
							BigResistence[i] = -1000;
							SmallResistence[i] = -1000;
							ClbTime[i] = -1000;
							ClbTemp[i] = -1000;
						}
						for(int i=0;i<10;i++){
							fpgaVersion[i] = -1000;
						}
						for(int i=0;i<2;i++){
							for(int j=0;j<89;j++){
								DbVersion[i][j] = -1000;
								ClbVersion[i][j] = -1000;
							}
						}
						statuspackloop = false;
						break;
					case 100:
						iTel = ITEL;
						if(iTel==5)
						{
							for(int i=0;i<1024;i++){
								mask5[i] = mask[1023-i];
								single_thresh5[i] = single_thresh[1023-i];
								record_thresh5[i] = record_thresh[1023-i];
								single_count5[i] = single_count[1023-i];
								single_time5[i] = single_time[1023-i];
								DbTemp5[i] = DbTemp[1023-i];
								HV5[i] = HV[1023-i];
								PreTemp5[i] = PreTemp[1023-i];
								BigResistence5[i] = BigResistence[1023-i];
								SmallResistence5[i] = SmallResistence[1023-i];
								ClbTime5[i] = ClbTime[1023-i];
								ClbTemp5[i] = ClbTemp[1023-i];
							}
							for(int i=0;i<1024;i++){
								mask[i] = mask5[i];
								single_thresh[i] = single_thresh5[i];
								record_thresh[i] = record_thresh5[i];
								single_count[i] = single_count5[i];
								single_time[i] = single_time5[i];
								DbTemp[i] = DbTemp5[i];
								HV[i] = HV5[i];
								PreTemp[i] = PreTemp5[i];
								BigResistence[i] = BigResistence5[i];
								SmallResistence[i] = SmallResistence5[i];
								ClbTime[i] = ClbTime5[i];
								ClbTemp[i] = ClbTemp5[i];
							}
						}
						Status->Fill();
						f9mode = -1000;
						f9pattern = -1000;
						pat_full=-1000;
						pat_nofull=-1000;
						nopat_full=-1000;
						nopat_nofull=-1000;
						clb_initial_Time = -1000;
						clb_initial_time = -1000;
						fired_tube = -1000;
						status_readback_Time = -1000;
						status_readback_time = -1000;
						for(int i=0;i<1024;i++){
							mask[i] = -1000;
							single_thresh[i] = -1000;
							record_thresh[i] = -1000;
							single_count[i] = -1000;
							single_time[i] = -1000;
							DbTemp[i] = -1000;
							HV[i] = -1000;
							PreTemp[i] = -1000;
							BigResistence[i] = -1000;
							SmallResistence[i] = -1000;
							ClbTime[i] = -1000;
							ClbTemp[i] = -1000;
						}
						for(int i=0;i<10;i++){
							fpgaVersion[i] = -1000;
						}
						for(int i=0;i<2;i++){
							for(int j=0;j<89;j++){
								DbVersion[i][j] = -1000;
								ClbVersion[i][j] = -1000;
							}
						}
						statuspackloop = false;
						printf("statuspackloop:%d itel:%d---\n\n",statuspackloop,iTel);
						break;
					default:
						fpgaVersion[status_pack_marker] = wfctaDecode.GetF18Version();
						wfctaDecode.GetMask(status_pack_marker, (int *)mask);
						break;
				}
			}
		}

		buffer_len = wfctaDecode.buffer_len_remain();
		if(buffer_len<=keep_size)
			break;
	}
	//delete[] buffertemp;
	//delete[] buffer;
	//delete[] FileHeadBuffer;
	fdata.close();
	delete[] bufferRaw;

	// ****************************************************************************** //
	rootfile->Write();
	rootfile->Close();


}
