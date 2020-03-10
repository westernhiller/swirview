#ifndef SWIRPROCESSOR_H
#define SWIRPROCESSOR_H

#include <QThread>
#include <QImage>
#include "global.h"
#include "imagebuffer.h"

typedef struct _framebuffer {
        uint16_t image[FRAME_PXCNT];
        uint32_t param[FRAME_PMNUM];
} PACKAGEBUFFER;
typedef PACKAGEBUFFER* LPPACKAGEBUFFER;

class SwirProcessor : public QThread
{
    Q_OBJECT

public:
    explicit SwirProcessor(QByteArray &frame, ImageBuffer* pImageBuffer, QObject* parent = nullptr);
    ~SwirProcessor() override;

signals:
    void killMe();

protected:
    void run() override;

private:
    PACKAGEBUFFER m_packageBuffer;
    QByteArray  m_frame;
    ImageBuffer* m_pImageBuffer;
    SWIRSETTINGS* m_pSettings;
    bool m_bSmooth;
    bool m_bHistogram;
    int m_nGain;
    int m_nFPS;

    void smoothImage(cv::Mat imgFrame);
    void addFPS(cv::Mat imgFrame);
    cv::Mat frame2Mat(uint16_t *buffer);

public slots:
    void updateFPS(int);
    void updateGain(int);
    void enableHistogram(bool);
    void enableSmooth(bool);
};

#endif // RTSPCAPTURER_H
