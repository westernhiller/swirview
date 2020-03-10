#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

#include <QList>
#include <QDateTime>
#include <QMutex>

#include "global.h"

typedef struct
{
    QString name;
    int width;
    int height;
    int type;
    int num;
} BUFFERINFO;

typedef struct
{
    cv::Mat matImg;
    QDateTime timeStamp;
} FRAMEINFO;

class ImageBuffer
{
public:
    ImageBuffer(QString name, int nMem, int nWidth, int nHeight, int nType);
    ~ImageBuffer();

    bool        writeImage(const FRAMEINFO&);
    FRAMEINFO*  readImage(bool bKeep = false);
    void        cleanUp();

    inline bool isEmpty() { return m_memlist.empty();}
    inline BUFFERINFO* getBufferInfo() { return &m_bufferInfo;}

private:
    BUFFERINFO  m_bufferInfo;
    FRAMEINFO*  m_pMemBuf;
    QList<int>	m_memlist;
    QList<int>	m_availist;
    QMutex      m_Lock;

    void        release();
    void        init();
    bool        isImageValid(const cv::Mat& image);
};

#endif // IMAGEBUFFER_H

