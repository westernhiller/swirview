#include "swircapturer.h"

#include <QDebug>
#include <QImage>
#include <QNetworkProxy>

#include "global.h"
#include "swirdialog.h"

SwirCapturer::SwirCapturer(QObject* parent)
    : m_nBytes2Read(0)
    , m_bConnected(false)
    , m_params(nullptr)
    , m_pSettings(nullptr)
{
    if(parent)
        m_pSettings = static_cast<SwirDialog*>(parent)->getSettings();
    QNetworkProxy::setApplicationProxy(QNetworkProxy());
    connect(this, SIGNAL(readyRead()), this, SLOT(readFromSocket()));
    connect(this, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

SwirCapturer::~SwirCapturer()
{
    if(m_params)
        delete m_params;

    disconnectFromHost();
}

void SwirCapturer::onTimer()
{
    if(!m_bConnected)
        return;

    // no response for 30 seconds, then lost connection
    if(m_nHeartbeating++ > 300)
    {
//        m_timer->stop();
//        restart();
    }

    // wait for no one to finish
    if(0 == m_nBytes2Read)
    {
        if(m_cmdlist.isEmpty())
            readFrame();
        else
        {
            m_mutex.lock();
            QPair<ENUM_SWIRCMD, QByteArray> cmd = m_cmdlist.front();
            m_cmdlist.pop_front();
            m_lastCmd = cmd.first;
            switch(m_lastCmd)
            {
            case SWIRCMD_GETFRAME:
//                qDebug() << "get frame command";
                m_nBytes2Read = FRAMEBUFSIZE;
                break;
            case SWIRCMD_GETPARAMS:
//                qDebug() << "get params command";
                m_nBytes2Read = 16;
                break;
            default:
                m_nBytes2Read = 0;
                break;
            }
            write(cmd.second);
            waitForBytesWritten();
            m_mutex.unlock();
        }
    }
}

void SwirCapturer::start()
{
    if(m_pSettings)
    {
        connectToHost(m_pSettings->ip, m_pSettings->port, QTcpSocket::ReadWrite);
        if(waitForConnected())
        {
//            qDebug() << "swir socket connected\n";
            emit cameraConncted(true);
            m_bConnected = true;
            m_timer->start(5);
            m_nBytes2Read = 0;
            m_cmdlist.clear();
            m_frameByteArray.clear();
            getParams();
        }
    }
}

void SwirCapturer::stop()
{
//    qDebug() << "socket reconnecting...\n";
//    emit sendMessage(tr("swir socket reconnecting..."));
    m_timer->stop();
    disconnectFromHost();
    m_bConnected = false;
    emit cameraConncted(false);
}

void SwirCapturer::onDisconnect()
{
    m_timer->stop();
    disconnectFromHost();
    m_bConnected = false;
    emit cameraConncted(false);
}

void SwirCapturer::restart()
{
    stop();
    msleep(500);
    start();
}

void SwirCapturer::getParams()
{
    QByteArray ba;
    ba.resize(5);
    ba[0] = 'p';
    ba[1] = 0x00;
    ba[2] = 0x00;
    ba[3] = 0x00;
    ba[4] = 0x00;

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_GETPARAMS;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
}

void SwirCapturer::adjustOnsite()
{
    QByteArray ba;
    ba.resize(5);
    ba[0] = 'n';
    ba[1] = 0x00;
    ba[2] = 0x00;
    ba[3] = 0x00;
    ba[4] = 0x00;

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_ADJUST;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
}

void SwirCapturer::readFrame()
{
    QByteArray ba;
    ba.resize(5);
    ba[0] = 'r';
    ba[1] = 0x00;
    ba[2] = 0x00;
    ba[3] = 0x00;
    ba[4] = 0x00;

    m_mutex.lock();
    m_lastCmd = SWIRCMD_GETFRAME;
    m_nBytes2Read = FRAMEBUFSIZE;
    write(ba);
    waitForBytesWritten();
    m_mutex.unlock();
}


void SwirCapturer::readFromSocket()
{
    m_nHeartbeating = 0;
    m_frameByteArray.append(readAll());
    if (m_frameByteArray.size() < m_nBytes2Read)
        return;

    switch(m_lastCmd)
    {
    case SWIRCMD_GETFRAME:
        emit getFrame(m_frameByteArray);
        m_frameByteArray.clear();
        m_nBytes2Read = 0;
        break;
    case SWIRCMD_GETPARAMS:
        parseParams();
        break;
//    case SWIRCMD_ADJUST:
//    case SWIRCMD_SETCYCLE:
//    case SWIRCMD_SETINTEG:
//    case SWIRCMD_SETMODE:
//    case SWIRCMD_NONE:
    default:
        break;
    }
}

void SwirCapturer::parseParams()
{
    if(!m_params)
        m_params = new STRUCT_PARAMS;

    memcpy(m_params, m_frameByteArray.data(), sizeof(STRUCT_PARAMS));
    m_mutex.lock();
    m_nBytes2Read = 0;
    m_mutex.unlock();

    emit updateMode(m_params->mode.data32);
    emit updateIntegral(m_params->integration.data32 * 0.00544);
    emit updateCycle(m_params->framecycle.data32 * 0.05474);
}


void SwirCapturer::fpareCall(quint8 code)
{
    QByteArray ba;
    ba.resize(11);
    ba[0] = 'f';
    ba[1] = 'p';
    ba[2] = 'a';
    ba[3] = 'r';
    ba[4] = 'e';
    ba[5] = 'c';
    ba[6] = 'a';
    ba[7] = 'l';
    ba[8] = 'l';
    ba[9] = ' ';
    ba[10] = '0' + code;

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_SETMODE;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
}

void SwirCapturer::setMode()
{
    QByteArray ba;
    ba.resize(5);
    ba[0] = 'm';
    ba[1] = static_cast<char>(m_params->mode.data8[0]);
    ba[2] = static_cast<char>(m_params->mode.data8[1]);
    ba[3] = static_cast<char>(m_params->mode.data8[2]);
    ba[4] = static_cast<char>(m_params->mode.data8[3]);

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_SETMODE;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
}


void SwirCapturer::enableHighgain(bool bEnable)
{
    if(bEnable)
        m_params->mode.data8[0] |= 0x04;
    else {
        m_params->mode.data8[0] &= 0xFB;
    }
    setMode();
/*
    if(bEnable)
        fpareCall(3);
    else {
        fpareCall(2);
    }
*/
}

void SwirCapturer::enableNonuniformityCorrection(bool bEnable)
{
    if(bEnable)
        m_params->mode.data8[0] |= 0x10;
    else {
        m_params->mode.data8[0] &= 0xEF;
    }
    setMode();
}

void SwirCapturer::enableIntegralAdjustion(bool bEnable)
{
    if(bEnable)
        m_params->mode.data8[1] |= 0x02;
    else {
        m_params->mode.data8[1] &= 0xFD;
    }
    setMode();
}

void SwirCapturer::setIntegral(double integral)
{
    UNION_REG regvalue;
    regvalue.data32 = static_cast<uint32_t>(integral / 0.00544);

    QByteArray ba;
    ba.resize(5);
    ba[0] = 'i';
//    memcpy(ba.data() + 1, &regvalue, sizeof(UNION_REG));

    ba[1] = static_cast<char>(regvalue.data8[0]);
    ba[2] = static_cast<char>(regvalue.data8[1]);
    ba[3] = static_cast<char>(regvalue.data8[2]);
    ba[4] = static_cast<char>(regvalue.data8[3]);

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_SETINTEG;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
    getParams();
}

void SwirCapturer::setCycle(double cycle)
{
    UNION_REG regvalue;
    regvalue.data32 = static_cast<uint32_t>(cycle / 0.05474);;

    QByteArray ba;
    ba.resize(5);
    ba[0] = 'c';

    ba[1] = static_cast<char>(regvalue.data8[0]);
    ba[2] = static_cast<char>(regvalue.data8[1]);
    ba[3] = static_cast<char>(regvalue.data8[2]);
    ba[4] = static_cast<char>(regvalue.data8[3]);

    m_mutex.lock();
    QPair<ENUM_SWIRCMD, QByteArray> cmd;
    cmd.first = SWIRCMD_SETCYCLE;
    cmd.second = ba;
    m_cmdlist.push_back(cmd);
    m_mutex.unlock();
    getParams();
}


