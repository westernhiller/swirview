#ifndef SWIRDISPLAYER_H
#define SWIRDISPLAYER_H

#include <QThread>
#include <QImage>
#include "global.h"
#include "imagebuffer.h"

class SwirDisplayer : public QThread
{
    Q_OBJECT

public:
    explicit SwirDisplayer(QObject* parent = nullptr);
    ~SwirDisplayer() override;

signals:
    void display(QImage);

protected:
    void run() override;

private:
    ImageBuffer* m_pImageBuffer;
    ImageBuffer* m_pVideoBuffer;
    SWIRSETTINGS* m_pSettings;
    bool m_bRecording;

public slots:
    void onRecord(bool);
};

#endif // SWIRDISPLAYER_H
