#include "imagebuffer.h"
#include <string.h>
#include <stdio.h>
#include <QDebug>

using namespace cv;

ImageBuffer::ImageBuffer(QString name, int nMem, int nWidth, int nHeight, int nType)
    : m_pMemBuf(nullptr)
{
    m_bufferInfo.name = name;
    m_bufferInfo.num = nMem;
    m_bufferInfo.type = nType;
    m_bufferInfo.width = nWidth;
    m_bufferInfo.height = nHeight;

    init();
}

ImageBuffer::~ImageBuffer()
{
    release();
    cleanUp();
}

void ImageBuffer::init()
{
    m_Lock.lock();

    m_pMemBuf = new FRAMEINFO[m_bufferInfo.num];
    cleanUp();
    m_Lock.unlock();
}

void ImageBuffer::cleanUp()
{
    m_availist.clear();
    m_memlist.clear();
    for (int i = 0; i < m_bufferInfo.num; i++)
        m_availist.push_back(i);
}

void ImageBuffer::release()
{
    m_Lock.lock();
    for(int i = 0; i < m_bufferInfo.num; i++)
        m_pMemBuf[i].matImg.release();
    delete[] m_pMemBuf;
    m_pMemBuf = nullptr;
    m_Lock.unlock();
}

FRAMEINFO* ImageBuffer::readImage(bool bKeep)
{
    FRAMEINFO* pFrame = nullptr;

    m_Lock.lock();
    if (!m_memlist.empty())
    {
        // find next available buffer
        int id = m_memlist.front();
        pFrame = new FRAMEINFO;
        m_pMemBuf[id].matImg.copyTo(pFrame->matImg);
        pFrame->timeStamp = m_pMemBuf[id].timeStamp;
        if(!bKeep)
        {
            m_memlist.pop_front();
            m_availist.push_back(id);
        }
    }
    m_Lock.unlock();

    return pFrame;
}

bool ImageBuffer::isImageValid(const Mat& image)
{
    if (!image.data || (image.type() != m_bufferInfo.type))
        return false;

    if((image.cols != m_bufferInfo.width) || (image.rows != m_bufferInfo.height))
        return false;

    return true;
}

bool ImageBuffer::writeImage(const FRAMEINFO &frame)
{
    if (!isImageValid(frame.matImg))
        return false;

    m_Lock.lock();

    qint64 timeMSecs = frame.timeStamp.toMSecsSinceEpoch();

    int idx = m_memlist.size() - 1;
    while(idx >= 0)
    {
        if(m_pMemBuf[m_memlist[idx--]].timeStamp.toMSecsSinceEpoch() < timeMSecs)
            break;
    }
    int id = 0;
    if(0 > idx)     // current frame is older than all frames
    {
        if(m_availist.empty())     //  discard current frame
        {
            m_Lock.unlock();
            return true;
        }

        id = m_availist.front();
        m_availist.pop_front();
        m_memlist.push_front(id);
    }
    else  // insert here
    {
        if(!m_availist.empty())     // add to front
        {
            id = m_availist.front();
            m_availist.pop_front();
            m_memlist.insert(m_memlist.begin() + idx, id);
        }
        else
        {
            id = m_memlist.front();
            m_memlist.insert(m_memlist.begin() + idx, id);
            m_memlist.pop_front();
        }
    }

    frame.matImg.copyTo(m_pMemBuf[id].matImg);
    m_pMemBuf[id].timeStamp = frame.timeStamp;

    m_Lock.unlock();
    return true;
}
