#ifndef SWIRDIALOG_H
#define SWIRDIALOG_H

#include <QDialog>
#include <QMutex>
#include <QKeyEvent>
#include "global.h"
#include "analyzer.h"
#include "controlpanel.h"
#include "videothread.h"
#include "swircapturer.h"
#include "swirdisplayer.h"
#include "glcanvas.h"

class SwirDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SwirDialog(QWidget *parent = nullptr);
    ~SwirDialog();

    inline SWIRSETTINGS* getSettings() { return &m_settings;}
    ImageBuffer* getImageBuffer(QString);

signals:
    void updateImage(QImage);
    void updateAnalyzeImage(QImage);
    void updatePatch(QImage);
    void saveVideoFrame(QImage);
    void updateFPS(float);
    void record(bool);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    ControlPanel*   m_pControl;
    Analyzer*       m_pAnalyzer;
    int             m_nCapturers;
    QList<SwirCapturer*> m_CapturerList;
//    SwirCapturer*   m_pCapturer;
    GLCanvas*       m_pCamera;
    GLCanvas*       m_pPatch;
    VideoThread*    m_pVideoSaver;
    SwirDisplayer*  m_pDisplayer;
    QList<ImageBuffer*> m_ImageBufferList;
    SWIRSETTINGS    m_settings;
    int             m_nFrames;
    QMutex          m_mutex;
    bool            m_bRecording;
    bool            m_bCapturing;
    bool            m_bAnalyzing;
    QRect           m_rtZoom;

    void            loadConfig();
    void            saveConfig();
    void            createGUI(QSize);

public slots:
    void            onFrame(QByteArray);
    void            onDisplay(QImage);
    void            onTimer();
    void            killProcessor();
    void            boxSelect(QRect);
    void            onSavePhoto();
    void            onStartRecording();
    void            onStopRecording();
    void            onExit();
    void            onPhotoSaved(QString);
    void            onVideoSaved(QString);
    void            onConnectCamera(bool);
};

#endif // SWIRDIALOG_H
