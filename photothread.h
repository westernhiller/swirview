#ifndef PHOTOTHREAD_H
#define PHOTOTHREAD_H

#include <QThread>
#include <QImage>
#include <QMutex>

#include "global.h"
#include "imagebuffer.h"

class PhotoThread : public QThread
{
    Q_OBJECT

public:
    explicit PhotoThread(QObject* parent = nullptr);
    ~PhotoThread() override;

signals:
    void photoSaved(QString photoFile);

protected:
    void run();

private:
    ImageBuffer*    m_pImageBuffer;
    SWIRSETTINGS*   m_pSettings;
};

#endif // PHOTOTHREAD_H
