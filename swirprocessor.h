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
    explicit SwirProcessor(QByteArray &frame, QObject* parent = nullptr);
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

    void smoothImage(cv::Mat imgFrame);
    cv::Mat frame2Mat(uint16_t *buffer);
};

#endif // SWIRPROCESSOR_H
