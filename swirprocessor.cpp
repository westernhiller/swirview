#include "swirprocessor.h"
#include <QDebug>
#include <QDateTime>

#include "global.h"
#include "swirdialog.h"

using namespace std;
using namespace cv;

SwirProcessor::SwirProcessor(QByteArray &frame, QObject* parent)
    : QThread(parent)
    , m_frame(frame)
{
    SwirDialog* pDlg = static_cast<SwirDialog*>(parent);
    m_pSettings = pDlg->getSettings();
    m_pImageBuffer = pDlg->getImageBuffer("source");
}

SwirProcessor::~SwirProcessor()
{
    requestInterruption();
    quit();
    wait();
}

void SwirProcessor::run()
{
    if(!m_pImageBuffer || m_frame.isNull())
    {
        return;
    }
    memcpy(&m_packageBuffer, m_frame.data(), FRAMEBUFSIZE);
    Mat imgFrame = frame2Mat(m_packageBuffer.image);
    if(m_pSettings->bHistogram)
        equalizeHist(imgFrame, imgFrame);

    if(m_pSettings->bSmooth)
        smoothImage(imgFrame);

    FRAMEINFO frame;
    frame.matImg = imgFrame;
    frame.timeStamp = QDateTime::currentDateTime();
    m_pImageBuffer->writeImage(frame);

    emit killMe();    //发送信号
}

Mat SwirProcessor::frame2Mat(uint16_t *buffer)
{
    static uint8_t unit[10] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};
    double alpha = 1.0 / unit[m_pSettings->nGain];

    Mat matFrame(SWIRHEIGHT, SWIRWIDTH, CV_8UC1);
    //    uint8_t goffset = offset[m_nGain];
    double acc = 0.0;
    for(int k = 0; k < FRAME_PXCNT; k++)
        acc += buffer[k];
    int avg= static_cast<int>(acc/FRAME_PXCNT);
    int k = 0;
    for(int i = 0; i < SWIRHEIGHT; i++)
    {
        uchar* ptr = static_cast<uchar*>(matFrame.ptr<uchar>(i));
        for(int j = 0; j < SWIRWIDTH; j++, k++)
            ptr[j] = saturate_cast<uchar>((buffer[k] - avg)* alpha + 128);
    }

    return matFrame;
}

void SwirProcessor::smoothImage(Mat imgFrame)
{
    static Mat imgPrev[3];
    static int nImages = 0;
    static int idx = 0;

    if(nImages < 3)
    {
        imgPrev[nImages++] = imgFrame/4;
    }
    else
    {
        Mat imgQuarter = imgFrame / 4;
        imgFrame = imgPrev[0] + imgPrev[1] + imgPrev[2] + imgQuarter;
        imgQuarter.copyTo(imgPrev[idx++]);
        if(idx == 3)
            idx = 0;
    }
}
