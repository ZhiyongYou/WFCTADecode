#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <map>
#include <TFile.h>
#include <TTree.h>
#include "pack/Event.h"
#include "core/EventDecode.h"
#include <fstream>
#include <zlib.h>

#include "KM2AEventDecode.h"
#include "KM2ASlowDecode.h"
#include "KM2AMDWaveDecode.h"
#include "WCDAEventDecode.h"
#include "FullRecDecode.h"

#define BUF_LEN 500000000
#define EV_FILE_HEAD 256

long g_index = 0;
long g_time = 1584861630000208716;

typedef EventDecode* (*create_event_decode_fun)();

EventDecode* createMDWaveDecode() {
    return new KM2AMDWaveDecode("md_wave");
}

EventDecode* createEDSlowDecode() {
    return new KM2ASlowDecode("ed");
}

EventDecode* createMDSlowDecode() {
    return new KM2ASlowDecode("md");
}

EventDecode* createEDEventDecode() {
    return new KM2AEventDecode("ed");
}

EventDecode* createMDEventDecode() {
    return new KM2AEventDecode("md");
}

EventDecode* createWCDAEventDecode() {
    return new WCDAEventDecode;
}

EventDecode* createFullRecDecode() {
    return new FullRecEventDecode;
}

void decode(char* datafile, char* rootfile, bool isCompress)
{
    int nED=0, nMD=0, nAll=0;
    int decoded = 0;
    int maxDecoded = -1;
    TFile froot(rootfile,"recreate");

    std::map<uint32_t, EventDecode*> theMap;//THE map:)
    std::map<uint32_t, create_event_decode_fun> facMap;

    facMap[0x00000002] = createEDSlowDecode;
    facMap[0x00010002] = createMDSlowDecode;
    facMap[0x00010003] = createMDWaveDecode;
    facMap[0x00000001] = createEDEventDecode;
    facMap[0x00010001] = createMDEventDecode;
    facMap[0x00030001] = createWCDAEventDecode;
    facMap[0xFFFF0000] = createFullRecDecode;
    
    std::ifstream fdata;
    fdata.open(datafile,std::ios::in|std::ios::binary);
    if(!fdata.good())
    {
        std::cout<<"cannot open file"<<std::endl;
    }

    uint8_t* bufferRaw = new uint8_t[BUF_LEN];
    uint8_t* buffer = new uint8_t[BUF_LEN];
    uint8_t* tmpBuffer = new uint8_t[260];
    fdata.read((char*)tmpBuffer,EV_FILE_HEAD);
    if(!fdata)
    {
    }

    uint64_t eventNumCount=0;
    while(1)
    {
        size_t sizeRead = 0;
        uint8_t* ptr1 = bufferRaw;
        fdata.read((char*)ptr1, Header::s_commonHeaderSize);
        if(!fdata) {
            break;
        }
        sizeRead += Header::s_commonHeaderSize;
        Header h(ptr1);
        std::cout << "MARKER: " <<std::hex <<h.startMarker()<< std::dec << std::endl;
        if(h.totalSize() > BUF_LEN) {
            std::cout << fdata.tellg() << std::endl;
            fdata.seekg(0,std::ios::end);
            std::cout << fdata.tellg() << std::endl;
            dumpPacket(ptr1, 100, 32);
            std::cout << h.totalSize() << std::endl;
        }
        assert(h.totalSize() <= BUF_LEN);

        fdata.read((char*)ptr1+sizeRead, h.totalSize()-sizeRead);
        if(!fdata)
        {
            break;
        }

        //continue;

        uint8_t *ptr0 = buffer;
        uint8_t *ptr = ptr0;
        unsigned long len;

        if(isCompress) {
            len = BUF_LEN-h.headerSize();

            if(uncompress(buffer, &len, bufferRaw+h.headerSize(), h.dataSize()) != Z_OK)
            {
                printf("uncompress failed!\n");
                return;
            }
            std::cout << "uncompressed OK" << std::endl;
            ptr0 = buffer;
            ptr = ptr0;
        }
        else {
            ptr0 = bufferRaw;
            ptr = ptr0;
            len = h.totalSize();
        }

        //dumpPacket(ptr, header.dataSize(), 32);
        while(ptr - ptr0 < len) {

            Event ev(ptr);
            uint32_t ev_streamTag = ev.streamTag();
            std::cout << std::hex << ev_streamTag << std::dec << std::endl;

            uint32_t ev_toSize = ev.totalSize();
            eventNumCount++;

            if(!facMap.count(ev_streamTag)) {
                ptr += ev.totalSize();
                std::cout << std::hex << "unknown stream: " << ev_streamTag << std::dec << std::endl;
                continue;
            }

            std::map<uint32_t, EventDecode*>::iterator itor;
            itor=theMap.find(ev_streamTag);   

            //if(0x00000001 == ev_streamTag) {
            //    nED++;
            //    nAll++;
            //    std::cout << nAll << " " << nED << " " << nMD << std::endl;
            //}
            //else if(0x00010001 == ev_streamTag) {
            //    nMD++;
            //    nAll++;
            //    std::cout << nAll << " " << nED << " " << nMD << std::endl;
            //}
            //else
            //{
            //    std::cout<<"strange streamTag found as "<<ev_streamTag<<std::endl;
            //}
            //switch(ev_streamTag)
            if(itor==theMap.end())//if exists
            {
                //create new class
                EventDecode *evd = (*facMap[ev_streamTag])();
                evd->init();
                theMap[ev_streamTag]=evd;
                //call init func
                //add to map
            }
            //call decode func
            std::cout << std::hex << "stream tag: " << ev_streamTag << std::dec << std::endl;
            theMap[ev_streamTag]->decode(ptr, ev_toSize);
            theMap[ev_streamTag]->fill();
            decoded ++;
            if(decoded >= maxDecoded && maxDecoded != -1) {
                break;
            }

            ptr += ev.totalSize();
        }
    }
    fdata.close();
    //t.Write();//write every tree
    std::cout<<"Data file has been scanned,  no data format error found, total "<<eventNumCount<<" events."<<std::endl;
    std::map<uint32_t,EventDecode*>::iterator itr;
    for(itr=theMap.begin();itr!=theMap.end();itr++)
    {
        if(itr->second) {
            itr->second->close();
            delete itr->second;
        }
    }
    froot.Close();
}

int main(int argc, char **argv)
{
    decode(argv[1],argv[2],false);
    return 0;
}
