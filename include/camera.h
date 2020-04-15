#ifndef __CAMERA__
#define __CAMERA__

#include <stdio.h>
#include <stdint.h>
#include <math.h>

//In subcluster, 23 means F_3DB_2//
/*int subcluster[8][8] =
    {
        {11,12,13,14,15,16,17,18},
        {21,22,23,24,25,26,27,28},
        {31,32,33,34,35,36,37,38},
        {41,42,43,44,45,46,47,48},
        {51,52,53,54,55,56,57,58},
        {61,62,63,64,65,66,67,68},
        {71,72,73,74,75,76,77,78},
        {81,82,83,84,85,86,87,88}
    };*/
//In SC_ADRESS, 23 means row_2 column_3//
extern int SC_ADRESS[8][8];

/********************************************************************************
 * void SC_Channel2SiPM(short F_DB, short mChannel, short *mSiPM)               *
 * change FPGA_DB & Channel to sipm.                                            *
 * F_DB==25 means F5_DB2; Channel==1 means channel_1; range of sipm is [0,1023] *
 ********************************************************************************/
extern void SC_Channel2SiPM(short F_DB, short mChannel, short *mSiPM);

extern short sipm2SC[1024];

extern short sipm2Channel[1024];

/*********************************************************************************
 * void SiPM2SC_Channel(short mSiPM, short *mSC, short *mChannel)                *
 * change sipm to FPGA_DB & Channel                                              *
 * mSC==25 means F5_DB2; mChannel==1 means channel_1; range of mSiPM is [0,1023] *
 *********************************************************************************/
extern void SiPM2SC_Channel(short mSiPM, short *mSC, short *mChannel);

/*******************************************************************************
 * void SC_Channel2eSiPM(short fpga, short db, short channel, short *sipm)     *
 * change FPGA & DB & Channel to esipm                              *
 * range of esipm is [1,1024]                                                   *
 *******************************************************************************/
extern void SC_Channel2eSiPM(short fpga, short db, short channel, short *sipm);

/*******************************************************************************
 * void eSiPM2SC_Channel(short mSiPM, short *mSC, short *mChannel)             *
 * change esipm to FPGA_DB & Channel                                           *
 * range of esipm is [1,1024]                                                  *
 *******************************************************************************/
extern void eSiPM2SC_Channel(short mSiPM, short *mSC, short *mChannel);



#endif //__CAMERA__











