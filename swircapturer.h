#ifndef SWIRCAPTURER_H
#define SWIRCAPTURER_H

#include <QTcpSocket>
#include <QByteArray>
#include <QImage>
#include <QTimer>
#include <QMutex>
#include <QPair>
#include "global.h"


typedef enum
{
    SWIRCMD_NONE,
    SWIRCMD_GETFRAME,
    SWIRCMD_GETPARAMS,
    SWIRCMD_SETMODE,
    SWIRCMD_SETCYCLE,
    SWIRCMD_SETINTEG,
    SWIRCMD_ADJUST
} ENUM_SWIRCMD;

typedef union
{
    uint32_t data32;
    uint8_t data8[4];
} UNION_REG;

typedef struct
{
    UNION_REG mode;
    UNION_REG framecycle;
    UNION_REG integration;
    UNION_REG reservied;
} STRUCT_PARAMS;


class SwirCapturer : public QTcpSocket
{
    Q_OBJECT

public:
    explicit SwirCapturer(QObject* parent = nullptr);
    ~SwirCapturer();
    void start();    
    void stop();

signals:
    void getFrame(QByteArray);
    void sendMessage(QString message);
    void updateMode(uint32_t mode);
    void updateIntegral(double integral);
    void updateCycle(double cycle);
    void cameraConncted(bool);
private:
    int m_nBytes2Read;
    bool m_bConnected;
    int m_nHeartbeating;
    QTimer* m_timer;
    QMutex m_mutex;
    QVector<QPair<ENUM_SWIRCMD, QByteArray> > m_cmdlist;

    STRUCT_PARAMS* m_params;
    ENUM_SWIRCMD m_lastCmd;
    QByteArray m_frameByteArray;
    SWIRSETTINGS* m_pSettings;

    void readFrame();
    void getParams();
    void setMode();
    void nextCommand();

    void parseParams();
    void fpareCall(quint8 code);

private slots:
    void onTimer();
    void readFromSocket();

public slots:
    void restart();
    void enableHighgain(bool bEnable);
    void enableNonuniformityCorrection(bool bEnable);
    void enableIntegralAdjustion(bool bEnable);
    void adjustOnsite();
    void setIntegral(double integral);
    void setCycle(double cycle);
    void onDisconnect();
};

typedef SwirCapturer*   LPSWIRCAPTURER;

#endif // SWIRCAPTURER_H
