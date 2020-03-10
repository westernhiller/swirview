#include "photothread.h"
#include <QDateTime>
#include <QDebug>
#include "swirdialog.h"

PhotoThread::PhotoThread(QObject* parent)
    : QThread(parent)
    , m_pImageBuffer(nullptr)
    , m_pSettings(nullptr)
{
    if(parent)
    {
        SwirDialog* pDlg = static_cast<SwirDialog*>(parent);
        m_pSettings = pDlg->getSettings();
        m_pImageBuffer = pDlg->getImageBuffer("source");
    }
}

PhotoThread::~PhotoThread()
{
    requestInterruption();
    quit();
    wait();
}

void PhotoThread::run()
{
    if(!m_pImageBuffer)
        return;

    QString path = m_pSettings ? m_pSettings->path : QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    while (!isInterruptionRequested())
    {
        FRAMEINFO* pFrame = m_pImageBuffer->readImage(true);
        if(pFrame && pFrame->matImg.data)
        {
            QDateTime timeNow = QDateTime::currentDateTime();
            QString time = timeNow.toString("yyyy-MM-dd-HH-mm-ss-zzz");
            QString filename = path + tr("/") + time + ".png";
            imwrite(filename.toStdString(), pFrame->matImg);
            delete pFrame;
            emit photoSaved(QString(filename));
            break;
        }
        usleep(10000);
    }
}
