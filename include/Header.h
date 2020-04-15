#ifndef _HEADER_H
#define _HEADER_H

#include <stdint.h>
//#include "zmqext/ZECommon.h"
//#include "memory/Buffer.h"
//#include <zmqext/ZECommon.h>
//#include <pack/SubDetector.h>

//namespace pckread {

enum start_marker_t 
{
    MARKER_CHANNEL = 0xbb1234bb,
    MARKER_ROS = 0xcc1234cc,
    MARKER_TIMED_FRAGMENT = 0xdd1234dd,
    MARKER_EVENT = 0xee1234ee,
};

#define COMMON_HEADER_SIZE ..

class Header 
{
public:
    const static uint32_t s_commonHeaderSize = 20;
    // for read
    Header(uint8_t *buffer);

    // for write
//    Header(memory::Buffer *buffer, start_marker_t marker, int fragmentHeaderSize, int extSize); // write with existing buffer
    Header(start_marker_t marker, int fragmentHeaderSize, int extSize, int maxDataSize);
    Header(start_marker_t marker, int fragmentHeaderSize, int extSize, int maxDataSize, uint32_t l1id);

    ~Header();

    uint32_t startMarker() {return m_startMarker; }
    uint8_t *commonHeader();
    uint8_t *fragmentHeader();
    uint8_t *extendInfo();
    uint8_t *data();

    uint32_t headerSize();
    uint32_t commonHeaderSize();
    uint32_t fragmentHeaderSize();
    uint32_t extendHeaderSize();
    uint32_t dataSize();

    uint32_t totalSize();

    uint32_t l1id();

    //uint8_t* getaddr();
    //memory::Buffer* getbuf();
    // for write
    void setL1id(uint32_t l1id);

protected:
    // for write
public:
    void incDataSize(uint32_t sz);
protected:
    uint32_t maxSize();

private:
    uint32_t m_startMarker;
    uint32_t m_headerSize;
    uint32_t m_extendHeaderSize;
    uint32_t m_totalSize;
    uint32_t m_l1id;

    uint32_t m_fragmentHeaderSize;
    uint8_t* m_ptr;

    // for write
    uint32_t m_maxSize;
    //memory::Buffer *m_buffer;

    bool m_internalBuffer;

    void resetFromBuffer();
};

//}

#endif
