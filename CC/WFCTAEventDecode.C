#include <stdlib.h>
#include <iostream>
#include "WFCTAEventDecode.h"
#include "dumpPack.h"
#include "camera.h"

using namespace std;
const int WFCTAEventDecode::s_pack_len=128;

WFCTAEventDecode::WFCTAEventDecode()
{
}

WFCTAEventDecode::WFCTAEventDecode(uint8_t *s_ptr, unsigned long buff_len)
{
	ptr0 = s_ptr;
	ptr1 = ptr0;
	buffer_len = buff_len;
	finished_len = 0;
}

WFCTAEventDecode::~WFCTAEventDecode()
{
}

void WFCTAEventDecode::dump_pack()
{
	dumpPacket(ptr0,pack_len,16);
}

void WFCTAEventDecode::nextpack()
{
	pack_type = -1;
	if(*(ptr1+0)==0xcc && *(ptr1+1)==0xcc 
			&& *(ptr1+2)==0xdd && *(ptr1+3)==0xdd 
			&& *(ptr1+4)==0xee && *(ptr1+5)==0xee 
			&& *(ptr1+6)==0xff && *(ptr1+7)==0xff)
	{
		ptr0 = ptr1;
		pack_len = s_pack_len*nFired()+40;
		ptr1 = ptr0 + pack_len;

		if(*(ptr1-8)==0x11 && *(ptr1-7)==0x11
				&& *(ptr1-6)==0x22 && *(ptr1-5)==0x22
				&& *(ptr1-4)==0x33 && *(ptr1-3)==0x33
				&& *(ptr1-2)==0x44 && *(ptr1-1)==0x44)
		{
			pack_type = 0;
			packCheck = 1;
		}
		else
		{
			pack_type = 0;
			packCheck = 2;
		}
	}
	else if(*(ptr1+0)==0xbb && *(ptr1+1)==0xbb
			&& *(ptr1+2)==0xbb && *(ptr1+3)==0xbb
			&& *(ptr1+4)==0xee && *(ptr1+5)==0xee
			&& *(ptr1+6)==0xee && *(ptr1+7)==0xee
			&& *(ptr1+41544)==0xaa && *(ptr1+41545)==0xaa
			&& *(ptr1+41546)==0xaa && *(ptr1+41547)==0xaa
			&& *(ptr1+41548)==0xdd && *(ptr1+41549)==0xdd
			&& *(ptr1+41550)==0xdd && *(ptr1+41551)==0xdd)
	{
		ptr0 = ptr1;
		pack_len = 41552;
		ptr1 = ptr0 + pack_len;
		ptr = ptr0;
		pack_type = 1;
	}
	//dumpPacket(ptr0,pack_len,16);
	//printf("pack_type: %d | pack_len: %llu\n",pack_type,pack_len);
	finished_len += pack_len;

}
unsigned long WFCTAEventDecode::buffer_len_remain()
{
	if(buffer_len > finished_len)
		return buffer_len - finished_len;
	else
		return 0;
}
int WFCTAEventDecode::packtype()
{
	return pack_type;
}


//event decode
uint64_t WFCTAEventDecode::RabbitTime()
{
	uint64_t rab_Time = *(ptr1-16)<<30 | *(ptr1-15)<<22 | *(ptr1-14)<<14 | *(ptr1-13)<<6 | (*(ptr1-12)>>2&0x3f);
	return rab_Time;
}

double WFCTAEventDecode::Rabbittime()
{
	//double rab_time = (*(ptr1-12)&0x03)<<24 | *(ptr1-11)<<16 | *(ptr1-10)<<8 | *(ptr1-9);
	double rab_time = (double)( (((uint64_t)ptr0[pack_len-1-11]&0x03)<<24)|
			((uint64_t)ptr0[pack_len-1-10]<<16)|
			((uint64_t)ptr0[pack_len-1-9]<<8)|
			((uint64_t)ptr0[pack_len-1-8]<<0) );
	return rab_time;
}

uint64_t WFCTAEventDecode::eventId()
{
	uint64_t evtid = *(ptr0+12)<<24 | *(ptr0+13)<<16| *(ptr0+14)<<8| *(ptr0+15);
	return evtid;
}

int32_t WFCTAEventDecode::bigpackLen()
{
	int32_t big_pack_len = pack_len;
	return big_pack_len;
}

int16_t WFCTAEventDecode::nFired()
{
	int16_t nfired = *(ptr0+22)<<8 | *(ptr0+23);
	return nfired;
}



/**********************
 * **   find sipms   **
 * ********************/
void WFCTAEventDecode::Find_SiPMs()
{
	short fpga,db;
	short sc,channel,sipm;
	int littlePackHead=0;
	m_sipm_position.clear();
	int sipmCount[1024]={0};
	short sipmRepeat=0;
	short sipmNumber=0;
	short sipmAddress=0;

	int16_t fire_tube = nFired();
	//dumpPacket(ptr0,16,16);
	for(int i=0;i<fire_tube;i++){
		littlePackHead = 24+128*i;
		if(   *(ptr0+littlePackHead+0)==0xaa && *(ptr0+littlePackHead+1)==0xaa
				&& *(ptr0+littlePackHead+124)==0xbb && *(ptr0+littlePackHead+125)==0xbb)
		{
			fpga = *(ptr0+littlePackHead+5)&0x0f;
			db = (*(ptr0+littlePackHead+5)>>4)&0x0f;
			sc = db*10+fpga;
			channel = *(ptr0+littlePackHead+4);
			SC_Channel2SiPM(sc,channel,&sipm);
			if(sipm>=0&&sipm<=1023){
				sipmCount[sipm]++;
				m_sipm_position.insert(pair<short,int>(sipm,(int)littlePackHead));
			}
			else{
				sipmNumber = 1;//sipm number is wrong
			}
		}
		else
		{
			sipmAddress = 1;//address of little pack is wrong
		}
	}
	for(int i=0;i<1024;i++)
	{
		if(sipmCount[i]>1)
		{
			sipmRepeat = 1;//some sipm repeat in one event
		}
	}
	packCheck = packCheck*1000+sipmNumber*100+sipmAddress*10+sipmRepeat;
}


/********************************************
 * ** get event id in each channel package **
 * ******************************************/
uint64_t WFCTAEventDecode::eventId_in_channel(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;

	uint64_t evtid_in_channel = ((uint64_t)ptr0[packposition+2]<<2)|
		((uint64_t)ptr0[packposition+3]>>6);
	return evtid_in_channel;
}

/****************************************************************
 * ** get zip mode, 00 represent 4 point zipped into one point **
 * **************************************************************/
int16_t WFCTAEventDecode::zipMode(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;

	int16_t zip_mode = ((int16_t)((ptr0[packposition+3]>>4)&0x03));
	return zip_mode;
}

/**********************************************
 * ** get trigger marker, over_single_marker **
 * ********************************************/
bool WFCTAEventDecode::GetOver_Single_Mark(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;

	bool m_Over_Single_Mark = (ptr0[packposition+3]&0x1);
	return m_Over_Single_Mark;
}

/*********************************************
 * ** get trigger marker, over record marker**
 * *******************************************/
bool WFCTAEventDecode::GetOver_Record_Mark(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;

	bool m_Over_Record_Mark = 0;
	int m_over_record_mark = ptr0[m_sipm_position_iter->second+3]&0x0e;
	//printf("m_over_record_mark:%d\n",m_over_record_mark);
	if(m_over_record_mark==0x0e)
	{
		m_Over_Record_Mark=1;
	}
	//bool m_Over_Record_Mark = ptr0[m_sipm_position_iter->second+3]&0xe;
	return m_Over_Record_Mark;
}

/**********************
 * ** high gain base **
 * ********************/
float WFCTAEventDecode::Getwinsum(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;

	float m_winsum = (float)( ((uint64_t)ptr0[packposition+64]<<24)|
			((uint64_t)ptr0[packposition+65]<<16)|
			((uint64_t)ptr0[packposition+66]<<8)|
			((uint64_t)ptr0[packposition+67]) ) ;
	m_winsum /= 16.;
	return m_winsum;
}

/******************************
 * ** get wave form [public] **
 * ****************************/
void WFCTAEventDecode::GetWaveForm(short isipm, int *pulseh, int *pulsel)
{
	this->waveform(isipm);
	for(int i=0;i<28;i++)
	{
		*(pulseh+isipm*28+i) = pulsehigh[i];
		*(pulsel+isipm*28+i) = pulselow[i];
	}

	//printf("%d wave_h: ",isipm);
	//for(int i=0;i<28;i++)
	//{
	//	printf("%d,",pulsehigh[i]);
	//}
	//printf("\n");
	//printf("%d wave_l: ",isipm);
	//for(int i=0;i<28;i++)
	//{
	//	printf("%d,",pulselow[i]);
	//}
	//printf("\n");
}
void WFCTAEventDecode::GeteSaturation(short isipm, int *esath, int *esatl)
{
	this->waveform(isipm);
	for(int i=0;i<28;i++)
	{
		*(esath+isipm*28+i) = saturationH[i];
		*(esatl+isipm*28+i) = saturationL[i];
	}
}




/******************************
 * ** get wave form [privat] **
 * ****************************/
void WFCTAEventDecode::waveform(short isipm)
{
	m_sipm_position_iter = m_sipm_position.find(isipm);
	int packposition = m_sipm_position_iter->second;
	int waveStart1 = packposition+6;
	int waveStart2 = packposition+12;

	for(int i=0; i<14; i++)
	{
		pulsehigh[i] = ((int)(ptr0[waveStart1+i*4]&0x7f)<<8)|((int)ptr0[waveStart1+i*4+1]);
		saturationH[i] = ((int)((ptr0[waveStart1+i*4]>>7)&0x01));
		pulselow[i]  = ((int)(ptr0[waveStart1+i*4+2]&0x7f)<<8)|((int)ptr0[waveStart1+i*4+3]);
		saturationL[i] = ((int)((ptr0[waveStart1+i*4+2]>>7)&0x01));
	}

	for(int i=14; i<28; i++)
	{
		pulsehigh[i] = ((int)(ptr0[waveStart2+i*4]&0x7f)<<8)|((int)ptr0[waveStart2+i*4+1]);
		saturationH[i] = ((int)((ptr0[waveStart2+i*4]>>7)&0x01));
		pulselow[i]  = ((int)(ptr0[waveStart2+i*4+2]&0x7f)<<8)|((int)ptr0[waveStart2+i*4+3]);
		saturationL[i] = ((int)((ptr0[waveStart2+i*4+2]>>7)&0x01));
	}
}



//status decode
uint8_t WFCTAEventDecode::NextStatusLittlePack()
{
	uint8_t status_pack_mark=100;
	//std::cout << ptr-ptr0 <<std::endl;
	while(ptr-ptr0 < pack_len)
	{
		if( *(ptr+0)==0x12 && *(ptr+1)==0x34 && *(ptr+62)==0xab && *(ptr+63)==0xcd ){
			status_pack_mark = *(ptr+2);//FPGA 1-9 PACK
			if(status_pack_mark==0){status_pack_mark = 9;}
			//ptr += 64;
			return status_pack_mark;
		}

		if( *(ptr+0)==0x12 && *(ptr+1)==0x34 && *(ptr+70)==0xab && *(ptr+71)==0xcd){
			status_pack_mark = *(ptr+3);
			//ptr += 72;
			return status_pack_mark;
		}

		if( *(ptr+0)==0x12 && *(ptr+1)==0x34 && *(ptr+72)==0xab && *(ptr+73)==0xcd){
			status_pack_mark = *(ptr+3);
			//ptr += 74;
			return status_pack_mark;
		}
		ptr++;
	}
	return 100;
}

/*****************************************
 * **get single_thresh and record_thresh**
 * ***************************************/
void WFCTAEventDecode::Getthresh(short *single_thresh, short *record_thresh)//Deal21Package
{
	int pack21_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	for(int i=0;i<15;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(single_thresh+sipm) = (*(ptr+4+4*i)<<8) | *(ptr+5+4*i);
	}
	SC_Channel2SiPM(sc,16,&sipm);
	*(single_thresh+sipm) = (*(ptr+66)<<8) | *(ptr+67);

	for(int i=0;i<14;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(record_thresh+sipm) = (*(ptr+6+4*i)<<8) | *(ptr+7+4*i);
	}
	SC_Channel2SiPM(sc,15,&sipm);
	*(record_thresh+sipm) = (*(ptr+64)<<8) | *(ptr+65);
	SC_Channel2SiPM(sc,16,&sipm);
	*(record_thresh+sipm) = (*(ptr+68)<<8) | *(ptr+69);

	ptr += pack21_len;
}

/*******************************
 * **deal status package of 22**
 * *****************************/
void WFCTAEventDecode::Deal22Pack(long *single_count)//Deal22Package get single count [channel 1-8]
{
	int pack22_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	for(int i=0;i<8;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(single_count+sipm) =  ((int64_t)ptr[9+i*5]<<32) |
			((int64_t)ptr[10+i*5]<<24) |
			((int64_t)ptr[11+i*5]<<16) |
			((int64_t)ptr[12+i*5]<<8) |
			((int64_t)ptr[13+i*5]);
	}

	ptr += pack22_len;
}


/*******************************
 * **deal status package of 23**
 * *****************************/
int WFCTAEventDecode::GetDBNumber()
{
	int db = (*(ptr+2)>>4)&0xf;
	int fpga =  *(ptr+2)&0xf;
	int m_dbnumber = db*10 + fpga;
	//printf("m_dbnumber:%d\n",m_dbnumber);
	return m_dbnumber;
}
int WFCTAEventDecode::GetDBVersion()
{
	int m_dbversion = *(ptr+69);
	//printf("m_dbversion:%0x\n",m_dbversion);
	return m_dbversion;
}
void WFCTAEventDecode::Deal23Pack(long *single_count, long *single_time)//Deal23Package get single [channel 9-16] count and single time
{
	int pack23_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;


	long m_single_time = ((int64_t)ptr[44]<<32) |
		((int64_t)ptr[45]<<24) |
		((int64_t)ptr[48]<<16) |
		((int64_t)ptr[49]<<8) |
		((int64_t)ptr[50]);
	for(int i=0;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(single_time+sipm) = m_single_time;
	}
	for(int i=8;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(single_count+sipm) =  ((int64_t)ptr[4+(i-8)*5]<<32) |
			((int64_t)ptr[5+(i-8)*5]<<24) |
			((int64_t)ptr[6+(i-8)*5]<<16) |
			((int64_t)ptr[7+(i-8)*5]<<8) |
			((int64_t)ptr[8+(i-8)*5]);
	}

	ptr += pack23_len;
}


void WFCTAEventDecode::GetHV(float *HV)//Deal81Package
{
	int pack81_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	SC_Channel2SiPM(sc,1,&sipm);
	//SC_Channel2eSiPM(fpga, db, 1, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( ((uint32_t)ptr[13]<<19) | ((uint32_t)ptr[14]<<11) | ((uint32_t)ptr[15]<<3) | ((uint32_t)ptr[16]>>5) );

	SC_Channel2SiPM(sc,2,&sipm);
	//SC_Channel2eSiPM(fpga, db, 2, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[16]&0x1f)<<22) | ((uint32_t)ptr[17]<<14) | ((uint32_t)ptr[18]<<6) | ((uint32_t)ptr[19]>>2) );

	SC_Channel2SiPM(sc,3,&sipm);
	//SC_Channel2eSiPM(fpga, db, 3, &sipm); sipm -= 1;
	*(HV+sipm) = (float)((((uint32_t)ptr[19]&0x02)<<25)|((uint32_t)ptr[20]<<17)|((uint32_t)ptr[21]<<9)|((uint32_t)ptr[22]<<1)|((uint32_t)ptr[23]>>7));

	SC_Channel2SiPM(sc,4,&sipm);
	//SC_Channel2eSiPM(fpga, db, 4, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[23]&0x7f)<<20) | ((uint32_t)ptr[24]<<12) | ((uint32_t)ptr[25]<<4) | ((uint32_t)ptr[26]>>4) );

	SC_Channel2SiPM(sc,5,&sipm);
	//SC_Channel2eSiPM(fpga, db, 5, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[26]&0x0f)<<23) | ((uint32_t)ptr[27]<<15) | ((uint32_t)ptr[28]<<7) | ((uint32_t)ptr[29]>>1) );

	SC_Channel2SiPM(sc,6,&sipm);
	//SC_Channel2eSiPM(fpga, db, 6, &sipm); sipm -= 1;
	*(HV+sipm) = (float)((((uint32_t)ptr[29]&0x01)<<26)|((uint32_t)ptr[30]<<18)|((uint32_t)ptr[31]<<10)|((uint32_t)ptr[32]<<2)|((uint32_t)ptr[33]>>6));

	SC_Channel2SiPM(sc,7,&sipm);
	//SC_Channel2eSiPM(fpga, db, 7, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[33]&0x3f)<<21) | ((uint32_t)ptr[34]<<13) | ((uint32_t)ptr[35]<<5) | ((uint32_t)ptr[36]>>3) );

	SC_Channel2SiPM(sc,8,&sipm);
	//SC_Channel2eSiPM(fpga, db, 8, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[36]&0x07)<<24) | ((uint32_t)ptr[37]<<16) | ((uint32_t)ptr[40]<<8) | ((uint32_t)ptr[41]) );

	SC_Channel2SiPM(sc,9,&sipm);
	//SC_Channel2eSiPM(fpga, db, 9, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( ((uint32_t)ptr[42]<<19) | ((uint32_t)ptr[43]<<11) | ((uint32_t)ptr[44]<<3) | ((uint32_t)ptr[45]>>5) );

	SC_Channel2SiPM(sc,10,&sipm);
	//SC_Channel2eSiPM(fpga, db, 10, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[45]&0x1f)<<22) | ((uint32_t)ptr[46]<<14) | ((uint32_t)ptr[47]<<6) | ((uint32_t)ptr[48]>>2) );

	SC_Channel2SiPM(sc,11,&sipm);
	//SC_Channel2eSiPM(fpga, db, 11, &sipm); sipm -= 1;
	*(HV+sipm) = (float)((((uint32_t)ptr[48]&0x02)<<25)|((uint32_t)ptr[49]<<17)|((uint32_t)ptr[50]<<9)|((uint32_t)ptr[51]<<1)|((uint32_t)ptr[52]>>7));

	SC_Channel2SiPM(sc,12,&sipm);
	//SC_Channel2eSiPM(fpga, db, 12, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[52]&0x7f)<<20) | ((uint32_t)ptr[53]<<12) | ((uint32_t)ptr[54]<<4) | ((uint32_t)ptr[55]>>4) );

	SC_Channel2SiPM(sc,13,&sipm);
	//SC_Channel2eSiPM(fpga, db, 13, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[55]&0x0f)<<23) | ((uint32_t)ptr[56]<<15) | ((uint32_t)ptr[57]<<7) | ((uint32_t)ptr[58]>>1) );
	
	SC_Channel2SiPM(sc,14,&sipm);
	//SC_Channel2eSiPM(fpga, db, 14, &sipm); sipm -= 1;
	*(HV+sipm) = (float)((((uint32_t)ptr[58]&0x01)<<26)|((uint32_t)ptr[59]<<18)|((uint32_t)ptr[60]<<10)|((uint32_t)ptr[61]<<2)|((uint32_t)ptr[62]>>6));

	SC_Channel2SiPM(sc,15,&sipm);
	//SC_Channel2eSiPM(fpga, db, 15, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[62]&0x3f)<<21) | ((uint32_t)ptr[63]<<13) | ((uint32_t)ptr[64]<<5) | ((uint32_t)ptr[65]>>3) );

	SC_Channel2SiPM(sc,16,&sipm);
	//SC_Channel2eSiPM(fpga, db, 16, &sipm); sipm -= 1;
	*(HV+sipm) = (float)( (((uint32_t)ptr[65]&0x07)<<24) | ((uint32_t)ptr[66]<<16) | ((uint32_t)ptr[67]<<8) | ((uint32_t)ptr[68]) );

	for(int i=0;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		//SC_Channel2eSiPM(fpga, db, i+1, &sipm); sipm -= 1;
		*(HV+sipm) /= (512*427.4087);
	}

	ptr += pack81_len;
}

void WFCTAEventDecode::GetPreTemp(float *PreTemp)//Deal82Package
{
	int pack82_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	double A = 0.00433;
	double B = 13.582;
	double C;

	for(int i=0;i<8;i++)
	{   
		SC_Channel2SiPM(sc,i+1,&sipm);
		//SC_Channel2eSiPM(fpga, db, i+1, &sipm); sipm -= 1;
		*(PreTemp+sipm) = (float)( ((int16_t)ptr[13+i*2]<<8) | ((int16_t)ptr[14+i*2]) );
	}
	SC_Channel2SiPM(sc,9,&sipm);
	//SC_Channel2eSiPM(fpga, db, 9, &sipm); sipm -= 1;
	*(PreTemp+sipm) = (float)( ((int16_t)ptr[13+8*2]<<8) | ((int16_t)ptr[14+9*2]) );
	for(int i=9;i<16;i++)
	{   
		SC_Channel2SiPM(sc,i+1,&sipm);
		//SC_Channel2eSiPM(fpga, db, i+1, &sipm); sipm -= 1;
		*(PreTemp+sipm) = (float)( ((int16_t)ptr[13+(i+1)*2]<<8) | ((int16_t)ptr[14+(i+1)*2]) );
	}
	for(int i=0;i<16;i++)
	{   
		SC_Channel2SiPM(sc,i+1,&sipm);
		//SC_Channel2eSiPM(fpga, db, i+1, &sipm); sipm -= 1;
		C = *(PreTemp+sipm)*10000./32768-2230.8;
		*(PreTemp+sipm) = (-1*B+sqrt(B*B-4*A*C))/(2*A);
		*(PreTemp+sipm) += 30;
	}

	ptr += pack82_len;
}

void WFCTAEventDecode::GetBigRes(float *BigResistence)//Deal83Package
{
	int pack83_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	for(int i=0;i<4;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(BigResistence+sipm) = (float)( ((int16_t)ptr[13+i*2]<<8) | (int16_t)ptr[14+i*2] ) *50/256.;
	}
	SC_Channel2SiPM(sc,5,&sipm);
	*(BigResistence+sipm) = (float)( ((int16_t)ptr[13+4*2]<<8) | (int16_t)ptr[14+5*2] ) *50/256.;
	for(int i=5;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(BigResistence+sipm) = (float)( ((int16_t)ptr[13+(i+1)*2]<<8) | (int16_t)ptr[14+(i+1)*2] ) *50/256.;
	}

	ptr += pack83_len;
}

void WFCTAEventDecode::GetSmallRes(float *SmallResistence)//Deal84Package
{
	int pack84_len = 72;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	SC_Channel2SiPM(sc,1,&sipm);
	*(SmallResistence+sipm) = (float)( ((int16_t)ptr[13]<<8) | (int16_t)ptr[14+2] )  *10/1024.;
	for(int i=1;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		*(SmallResistence+sipm) = (float)( ((int16_t)ptr[13+(i+1)*2]<<8) | (int16_t)ptr[14+(i+1)*2] ) *10/1024.;
	}

	ptr += pack84_len;
}


int WFCTAEventDecode::GetClbNumber()//85 pack
{
	int db = (*(ptr+2)>>4)&0xf;
	int fpga =  *(ptr+2)&0xf;
	int m_clbnumber = db*10 + fpga;
	//printf("m_dbnumber:%d\n",m_dbnumber);
	return m_clbnumber;
}
int WFCTAEventDecode::GetClbVersion()//85 pack
{
	int m_clbversion = (int)( ((int32_t)ptr[68]<<8) | (int32_t)ptr[69]);//*(ptr+69);
	//printf("m_dbversion:%0x\n",m_dbversion);
	return m_clbversion;
}
void WFCTAEventDecode::GetClbTemp(float *ClbTemp)//Deal85pack
{
	int pack85_len = 74;

	short sipm;
	short fpga = *(ptr+2)&0x0f;
	short db = (*(ptr+2)>>4)&0x0f;
	short sc = db*10+fpga;

	float m_ClbTemp = (float)( ((int16_t)ptr[15]<<8) | (int16_t)ptr[16] );
	if(m_ClbTemp<40960) {m_ClbTemp /= 256.;}
	else                {m_ClbTemp = -(65536-m_ClbTemp)/256.;}

	for(int i=0;i<16;i++)
	{
		SC_Channel2SiPM(sc,i+1,&sipm);
		//SC_Channel2eSiPM(fpga, db, i+1, &sipm); sipm -= 1;
		*(ClbTemp+sipm) = m_ClbTemp;
	}

	ptr += pack85_len;
}


/*************************
 * **get info in F9 pack**
 * ***********************/
int WFCTAEventDecode::GetF9Mode()
{
	int m_f9_mode = (int)( (int32_t)ptr[4]);
	printf("m_f9_mode:%0x\n",m_f9_mode);
	return m_f9_mode;
}
int WFCTAEventDecode::GetPattern()
{
	int m_pattern = (int)( (int32_t)ptr[64-59]);
	printf("m_pattern:%0x\n",m_pattern);
	return m_pattern;
}
long WFCTAEventDecode::GetPat_Full()
{
	uint64_t m_pat_full = ((uint64_t)ptr[64-58]<<56)|
		((uint64_t)ptr[64-57]<<48)|
		((uint64_t)ptr[64-56]<<40)|
		((uint64_t)ptr[64-55]<<32)|
		((uint64_t)ptr[64-54]<<24)|
		((uint64_t)ptr[64-53]<<16)|
		((uint64_t)ptr[64-52]<<8)|
		((uint64_t)ptr[64-51]);
	return m_pat_full;
}
long WFCTAEventDecode::GetPat_noFull()
{
	uint64_t m_pat_nofull = ((uint64_t)ptr[64-50]<<56)|
		((uint64_t)ptr[64-49]<<48)|
		((uint64_t)ptr[64-48]<<40)|
		((uint64_t)ptr[64-47]<<32)|
		((uint64_t)ptr[64-46]<<24)|
		((uint64_t)ptr[64-45]<<16)|
		((uint64_t)ptr[64-44]<<8)|
		((uint64_t)ptr[64-43]);
	return m_pat_nofull;
}
long WFCTAEventDecode::GetnoPat_Full()
{
	uint64_t m_nopat_full = ((uint64_t)ptr[64-42]<<56)|
		((uint64_t)ptr[64-41]<<48)|
		((uint64_t)ptr[64-40]<<40)|
		((uint64_t)ptr[64-39]<<32)|
		((uint64_t)ptr[64-38]<<24)|
		((uint64_t)ptr[64-37]<<16)|
		((uint64_t)ptr[64-36]<<8)|
		((uint64_t)ptr[64-35]);
	return m_nopat_full;
}
long WFCTAEventDecode::GetnoPat_noFull()
{
	uint64_t m_nopat_nofull = ((uint64_t)ptr[64-34]<<56)|
		((uint64_t)ptr[64-33]<<48)|
		((uint64_t)ptr[64-32]<<40)|
		((uint64_t)ptr[64-31]<<32)|
		((uint64_t)ptr[64-30]<<24)|
		((uint64_t)ptr[64-29]<<16)|
		((uint64_t)ptr[64-28]<<8)|
		((uint64_t)ptr[64-27]);
	return m_nopat_nofull;
}
int WFCTAEventDecode::GetF9Version()
{
	int m_f9version = (int)( (int32_t)ptr[64-3]);
	printf("m_f9version:%0x\n",m_f9version);
	return m_f9version;
}
int WFCTAEventDecode::GetF9PlusVersion()
{
	int m_f9plusversion = (int)( (int32_t)ptr[64-16]);
	printf("m_f9pversion:%0x\n",m_f9plusversion);
	return m_f9plusversion;
}
uint64_t WFCTAEventDecode::GetclbInitialTime()
{
	uint64_t m_clb_initial_Time = ((uint64_t)ptr[64-26]<<30)|
		((uint64_t)ptr[64-25]<<22)|
		((uint64_t)ptr[64-24]<<14)|
		((uint64_t)ptr[64-23]<<6)|
		((uint64_t)ptr[64-22]>>2&0x3f);
	return m_clb_initial_Time;
}
double WFCTAEventDecode::GetclbInitialtime()
{
	double m_clb_initial_time = (double)( (((uint64_t)ptr[64-22]&0x03)<<24)|
			((uint64_t)ptr[64-21]<<16)|
			((uint64_t)ptr[64-20]<<8)|
			((uint64_t)ptr[64-19]<<0) );
	return m_clb_initial_time;
}
int WFCTAEventDecode::GetFiredTube()
{
	int m_fired_tube = (int)( ((int32_t)ptr[64-18]<<8) | (int32_t)ptr[64-17]);
	return m_fired_tube;
}
uint64_t  WFCTAEventDecode::GetStatusReadbackTime()
{
	uint64_t m_status_readback_Time = ((uint64_t)ptr[64-11]<<30)|
		((uint64_t)ptr[64-10]<<22)|
		((uint64_t)ptr[64-9]<<14)|
		((uint64_t)ptr[64-8]<<6)|
		((uint64_t)ptr[64-7]>>2&0x3f);
	return m_status_readback_Time;
}
double  WFCTAEventDecode::GetStatusReadbacktime()
{
	int pack00_len = 64;
	double m_status_readback_time = (double)( (((uint64_t)ptr[64-7]&0x03)<<24)|
			((uint64_t)ptr[64-6]<<16)|
			((uint64_t)ptr[64-5]<<8)|
			((uint64_t)ptr[64-4]<<0) );
	ptr += pack00_len;
	return m_status_readback_time;
}



int WFCTAEventDecode::GetF18Version()//10-80 pack
{
	int m_f18version = (int)( (int32_t)ptr[64-3]);
	//printf("m_f18version:%0x\n",m_f18version);
	return m_f18version;
}

void WFCTAEventDecode::GetMask(uint8_t f_board, int *mask)
{
	int packn0_len = 64;

	int m_mask;
	short db_board,channel,sc,sipm;
	int ich=0;

	for(int i=59;i>27;i--)
	{
		m_mask = (int)( (int32_t)((ptr[64-i]>>6)&0x03));
		channel = ich%16+1;db_board = ich/16+1;ich++;
		sc = db_board*10+f_board;
		SC_Channel2SiPM(sc,channel,&sipm);
		*(mask+sipm) = m_mask;
		//printf("f_board:%d db_board:%d channel:%d sipm:%d m_mask:%d\n",f_board,db_board,channel,sipm,m_mask);
		m_mask = (int)( (int32_t)((ptr[64-i]>>4)&0x03));
		channel = ich%16+1;db_board = ich/16+1;ich++;
		sc = db_board*10+f_board;
		SC_Channel2SiPM(sc,channel,&sipm);
		*(mask+sipm) = m_mask;
		//printf("f_board:%d db_board:%d channel:%d sipm:%d m_mask:%d\n",f_board,db_board,channel,sipm,m_mask);
		m_mask = (int)( (int32_t)((ptr[64-i]>>2)&0x03));
		channel = ich%16+1;db_board = ich/16+1;ich++;
		sc = db_board*10+f_board;
		SC_Channel2SiPM(sc,channel,&sipm);
		*(mask+sipm) = m_mask;
		//printf("f_board:%d db_board:%d channel:%d sipm:%d m_mask:%d\n",f_board,db_board,channel,sipm,m_mask);
		m_mask = (int)( (int32_t)((ptr[64-i])&0x03));
		channel = ich%16+1;db_board = ich/16+1;ich++;
		sc = db_board*10+f_board;
		SC_Channel2SiPM(sc,channel,&sipm);
		*(mask+sipm) = m_mask;
		//printf("f_board:%d db_board:%d channel:%d sipm:%d m_mask:%d\n",f_board,db_board,channel,sipm,m_mask);
	}
	//dumpPacket(begin+packsize-59,34);
	
	ptr += packn0_len;
}













