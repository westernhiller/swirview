#include "videothread.h"
#include <QDateTime>
#include "swirdialog.h"

using namespace std;
using namespace cv;

VideoThread::VideoThread(QObject* parent)
    : QThread(parent)
    , m_pSettings(nullptr)
    , m_pVideoSaver(nullptr)
    , m_framecnt(0)
{
    if(parent)
    {
        SwirDialog* pDlg = static_cast<SwirDialog*>(parent);
        m_pSettings = pDlg->getSettings();
        m_pImageBuffer = pDlg->getImageBuffer("video");
    }
}

VideoThread::~VideoThread()
{
    requestInterruption();
    stopRecording();
    quit();
    wait();
}

void VideoThread::run()
{
    if(!m_pImageBuffer)
        return;

//    startRecording();
    while(!isInterruptionRequested())
    {
        FRAMEINFO* pFrame = m_pImageBuffer->readImage();
        if(pFrame && pFrame->matImg.data)
        {
            m_mutex.lock();
            if(m_pVideoSaver)
            {
//                m_pVideoSaver->write(pFrame->matImg);

                Mat image;
                cvtColor(pFrame->matImg, image, CV_GRAY2RGB);
                m_pVideoSaver->write(image);
            }
            m_mutex.unlock();

            if(m_framecnt++ == 15000)
            {
                stopRecording();
                startRecording();
            }
        }
        delete pFrame;
        usleep(10000);
    }
}

void VideoThread::record(bool bRecording)
{
    if(bRecording)
        startRecording();
    else
        stopRecording();
}

void VideoThread::startRecording()
{
    QString path = m_pSettings ? m_pSettings->path : QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    m_videofile = path + tr("/") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + tr(".avi");
    m_mutex.lock();
    m_pVideoSaver = new VideoWriter(m_videofile.toStdString(),
                                    VideoWriter::fourcc('M', 'J', 'P', 'G'),
//                                    VideoWriter::fourcc('M', 'P', '4','2'),
                                    25,
                                    Size(SWIRWIDTH, SWIRHEIGHT),
                                    true);

    m_framecnt = 0;
    m_mutex.unlock();
}

void VideoThread::stopRecording()
{
    bool bfinished = false;
    m_mutex.lock();
    if(m_pVideoSaver)
    {
        delete m_pVideoSaver;
        m_pVideoSaver = nullptr;
        bfinished = true;
    }
    m_mutex.unlock();

    if(bfinished)
        emit videoSaved(m_videofile);
}
