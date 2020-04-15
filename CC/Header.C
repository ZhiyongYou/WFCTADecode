#include "Header.h"
#include <stdint.h>
#include <iostream>
//#include <zmqext/ZECommon.h>
//using namespace pckread;

Header::Header(uint8_t* buffer)
{
    m_buffer = nullptr;

    m_ptr = buffer;
    resetFromBuffer();

    // for read
    m_maxSize = 0;
    m_internalBuffer = false;
}

Header::~Header()
{
    if(m_internalBuffer) {
        std::cout<<"delete buf"<<std::endl;
        delete m_buffer;
    }
}

uint32_t Header::totalSize()
{
    return m_totalSize;
}

uint32_t Header::headerSize()
{
    return m_headerSize;
}

uint32_t Header::extendHeaderSize()
{
    return m_extendHeaderSize;
}

uint32_t Header::l1id()
{
    return m_l1id;
}

// for write
Header::Header(memory::Buffer *buffer, start_marker_t marker, int fragmentHeaderSize, int extSize)
{
    m_buffer = buffer;
    m_ptr = (uint8_t*)buffer->addr();

    *(uint32_t*)m_ptr = marker;
    *(uint32_t*)(m_ptr+4) = fragmentHeaderSize + s_commonHeaderSize + extSize;
    *(uint32_t*)(m_ptr+8) = extSize;
    *(uint32_t*)(m_ptr+12) = fragmentHeaderSize + s_commonHeaderSize + extSize + 0; // data size is 0
    *(uint32_t*)(m_ptr+16) = 0; // empty l1id

    resetFromBuffer();
    m_maxSize = m_buffer->max_size();
    m_totalSize = *(uint32_t*)(m_ptr+12);
    m_internalBuffer = false;
}
    
// for write
Header::Header(start_marker_t marker, int fragmentHeaderSize, int extSize, int maxDataSize)
{
    m_maxSize = maxDataSize + fragmentHeaderSize + s_commonHeaderSize + extSize;
    m_buffer = new memory::Buffer(m_maxSize);
    m_ptr = (uint8_t*)m_buffer->addr();

    *(uint32_t*)m_ptr = marker;
    *(uint32_t*)(m_ptr+4) = fragmentHeaderSize + s_commonHeaderSize + extSize;
    *(uint32_t*)(m_ptr+8) = extSize;
    *(uint32_t*)(m_ptr+12) = fragmentHeaderSize + s_commonHeaderSize + extSize + 0; // data size is 0
    *(uint32_t*)(m_ptr+16) = 0; // empty l1id

    resetFromBuffer();
    m_internalBuffer = true;
}

// for write
Header::Header(start_marker_t marker, int fragmentHeaderSize, int extSize, int maxDataSize, uint32_t l1id)
{
    m_maxSize = maxDataSize + fragmentHeaderSize + s_commonHeaderSize + extSize;
    m_buffer = new memory::Buffer(m_maxSize);
    m_ptr = (uint8_t*)m_buffer->addr();

    *(uint32_t*)m_ptr = marker;
    *(uint32_t*)(m_ptr+4) = fragmentHeaderSize + s_commonHeaderSize + extSize;
    *(uint32_t*)(m_ptr+8) = extSize;
    *(uint32_t*)(m_ptr+12) = fragmentHeaderSize + s_commonHeaderSize + extSize + 0; // data size is 0
    *(uint32_t*)(m_ptr+16) = l1id; 

    resetFromBuffer();
    m_internalBuffer = true;
}

uint8_t *Header::commonHeader()
{
    return m_ptr;
}

uint8_t *Header::fragmentHeader()
{
    return m_ptr+commonHeaderSize();
}

uint8_t *Header::extendInfo()
{
    return m_ptr+commonHeaderSize()+fragmentHeaderSize();
}

uint8_t *Header::data()
{
    return m_ptr+headerSize();
}

uint32_t Header::commonHeaderSize()
{
    return s_commonHeaderSize;
}

uint32_t Header::fragmentHeaderSize()
{
    return m_fragmentHeaderSize;
}

uint32_t Header::dataSize()
{
    return m_totalSize-m_headerSize;
}

// for write
void Header::setL1id(uint32_t l1id)
{
    m_l1id = l1id;
    *(uint32_t*)(m_ptr+16) = m_l1id;
}

// for write
void Header::incDataSize(uint32_t sz)
{
    m_totalSize += sz;
    *(uint32_t*)(m_ptr+12) = m_totalSize;
    if(m_buffer) {
        m_buffer->setSizeUsed(m_totalSize);
    }
}

uint32_t Header::maxSize()
{
    return m_maxSize;
}

void Header::resetFromBuffer()
{
    m_startMarker = *(uint32_t*)m_ptr;
    m_headerSize = *(uint32_t*)(m_ptr+4);
    m_extendHeaderSize= *(uint32_t*)(m_ptr+8);
    m_totalSize= *(uint32_t*)(m_ptr+12);
    m_l1id = *(uint32_t*)(m_ptr+16);
    m_fragmentHeaderSize = m_headerSize - m_extendHeaderSize - s_commonHeaderSize;

    if(m_buffer) {
        m_buffer->setSizeUsed(m_totalSize);
    }
}

//uint8_t* Header::getaddr()
//{
//    return m_ptr;
//}
//
//memory::Buffer* Header::getbuf()
//{
//    if(m_buffer)
//    {
//        return m_buffer;
//    }
//}
