#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>

#include "global.h"
#include "imagebuffer.h"

class VideoThread : public QThread
{
    Q_OBJECT

public:
    explicit VideoThread(QObject* parent = nullptr);
    ~VideoThread() override;

signals:
    void videoSaved(QString);

protected:
    void run() override;

private:
    ImageBuffer*    m_pImageBuffer;
    SWIRSETTINGS*   m_pSettings;
    cv::VideoWriter* m_pVideoSaver;
    int             m_framecnt;
    QString         m_videofile;
    QMutex          m_mutex;

    void            startRecording();
    void            stopRecording();

public slots:
    void record(bool);
};

#endif // VIDEOTHREAD_H
