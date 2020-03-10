#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include "global.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel() override;

signals:
    void updateGain(int nGain);
    void savePhoto();
    void startRecording();
    void stopRecording();
    void updateMode(uint32_t nMode);
    void updateIntegral(double);
    void updateCycle(double);
    void enableHighgain(bool bEnable);
    void enableNonuniform(bool bEnable);
    void enableIntegral(bool bEnable);
    void connectCamera(bool);
    void adjustOnsite();
    void exit();

private:
    Ui::ControlPanel *ui;
    SWIRSETTINGS*   m_pSettings;
    bool    m_bCameraConnected;

    void enableItems(bool);

public slots:
    void gainChanged(int nGain);
    void recordVideo();
    void saveImage();
    void setIntegral(double integral);
    void setFrameCycle(double cycle);
    void setMode(uint32_t mode);
    void onSetIntegral();
    void onSetCycle();
    void histogramCheckBoxChanged(int);
    void highgainCheckBoxChanged(int);
    void integralCheckBoxChanged(int);
    void nonuniformCheckBoxChanged(int);
    void mirrorCheckBoxChanged(int);
    void smoothCheckBoxChanged(int);
    void keepratioCheckBoxChanged(int);
    void updateFPS(float);
    void onBrowse();
    void onConnect();
    void onAdjustOnsite();
    void connected(bool);
    void onExit();
};

#endif // CONTROLPANEL_H
