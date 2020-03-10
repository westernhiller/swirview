#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QIntValidator>
#include <QRegExpValidator>

#include "global.h"
#include "swirdialog.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel),
    m_bCameraConnected(false)
{
    if(parent)
        m_pSettings = static_cast<SwirDialog*>(parent)->getSettings();
    ui->setupUi(this);

    ui->labelPath->setText(m_pSettings->path);
    ui->sliderGain->setValue(5);
    ui->sliderGain->setRange(1, 10);
    ui->sliderGain->setTickInterval(1);
    ui->editCycle->setValidator(new QIntValidator(28, 5700, this));

    if(m_pSettings)
    {
        ui->labelPath->setText(m_pSettings->path);
        ui->ipaddr->setText(m_pSettings->ip);
        ui->portnum->setText(QString::number(m_pSettings->port));
        ui->cbMirror->setCheckState(m_pSettings->bMirror? Qt::Checked : Qt::Unchecked);
        ui->cbKeepRatio->setCheckState(m_pSettings->bKeepRatio ? Qt::Checked : Qt::Unchecked);
        ui->sliderGain->setValue(m_pSettings->nGain);
        ui->cbSmooth->setCheckState(m_pSettings->bSmooth? Qt::Checked : Qt::Unchecked);
        ui->cbHistogram->setCheckState(m_pSettings->bHistogram? Qt::Checked : Qt::Unchecked);
        ui->cbHighgain->setCheckState(m_pSettings->bHighgain? Qt::Checked : Qt::Unchecked);
        ui->labelGain->setText(QString::number(m_pSettings->nGain));
    }

    enableItems(false);

    connect(ui->sliderGain, SIGNAL(valueChanged(int)), this, SLOT(gainChanged(int)));
    connect(ui->btnPhoto, SIGNAL(clicked()), this, SLOT(saveImage()));
    connect(ui->btnVideo, SIGNAL(clicked()), this, SLOT(recordVideo()));
    connect(ui->btnSetIntegral, SIGNAL(clicked()), this, SLOT(onSetIntegral()));
    connect(ui->btnSetCycle, SIGNAL(clicked()), this, SLOT(onSetCycle()));
    connect(ui->cbSmooth, SIGNAL(stateChanged(int)), this, SLOT(smoothCheckBoxChanged(int)));
    connect(ui->cbHighgain, SIGNAL(stateChanged(int)), this, SLOT(highgainCheckBoxChanged(int)));
    connect(ui->cbIntegral, SIGNAL(stateChanged(int)), this, SLOT(integralCheckBoxChanged(int)));
    connect(ui->cbHistogram, SIGNAL(stateChanged(int)), this, SLOT(histogramCheckBoxChanged(int)));
    connect(ui->cbNonuniform, SIGNAL(stateChanged(int)), this, SLOT(nonuniformCheckBoxChanged(int)));
    connect(ui->cbMirror, SIGNAL(stateChanged(int)), this, SLOT(mirrorCheckBoxChanged(int)));
    connect(ui->cbKeepRatio, SIGNAL(stateChanged(int)), this, SLOT(keepratioCheckBoxChanged(int)));
    connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));
    connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(onConnect()));
    connect(ui->btnExit, SIGNAL(clicked()), this, SLOT(onExit()));
    connect(ui->btnAdjustOnsite, SIGNAL(clicked()), this, SLOT(onAdjustOnsite()));
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::enableItems(bool bEnable)
{
    ui->ipaddr->setEnabled(!bEnable);
    ui->portnum->setEnabled(!bEnable);

    ui->sliderGain->setEnabled(bEnable);
    ui->editCycle->setEnabled(bEnable);
    ui->cbSmooth->setEnabled(bEnable);
    ui->cbIntegral->setEnabled(bEnable);
    ui->cbNonuniform->setEnabled(bEnable);
    ui->cbHistogram->setEnabled(bEnable);
    ui->cbHighgain->setEnabled(bEnable);
    ui->editCycle->setEnabled(bEnable);
    ui->editIntegral->setEnabled(bEnable);
    ui->btnSetCycle->setEnabled(bEnable);
    ui->btnSetIntegral->setEnabled(bEnable);
    ui->btnAdjustOnsite->setEnabled(bEnable);
    ui->btnPhoto->setEnabled(bEnable);
    ui->btnVideo->setEnabled(bEnable);
}

void ControlPanel::onBrowse()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setDirectory(m_pSettings->path);
    fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    if(fileDialog->exec() == QDialog::Accepted)
    {
        m_pSettings->path = fileDialog->selectedFiles()[0];
        ui->labelPath->setText(m_pSettings->path);
    }
}

void ControlPanel::onAdjustOnsite()
{
    emit adjustOnsite();
}

void ControlPanel::onConnect()
{    
    emit connectCamera(!m_bCameraConnected);
}

void ControlPanel::connected(bool bConnected)
{
    if(bConnected)
    {
        ui->btnConnect->setText(QString::fromUtf8("断开相机"));
    }
    else {
        ui->btnConnect->setText(QString::fromUtf8("连接相机"));
    }
    enableItems(bConnected);
    m_bCameraConnected = bConnected;
}

void ControlPanel::recordVideo()
{
    static bool bRecording = false;
    bRecording = !bRecording;
    if(bRecording)
    {
        ui->btnVideo->setText(QString::fromUtf8("停止录像"));
        emit startRecording();
    }
    else
    {
        ui->btnVideo->setText(QString::fromUtf8("开始录像"));
        emit stopRecording();
    }
}

void ControlPanel::updateFPS(float fps)
{
    ui->fps->setText(QString::number(fps));
}

void ControlPanel::onExit()
{
    emit exit();
}

void ControlPanel::saveImage()
{
    emit savePhoto();
}

void ControlPanel::gainChanged(int nGain)
{
    ui->labelGain->setText(QString::number(nGain));
    m_pSettings->nGain = nGain - 1;
}

void ControlPanel::setMode(uint32_t mode)
{
    UNION_REG value;
    value.data32 = mode;
    if((value.data8[0] & 0x04) == 0x04)
        ui->cbHighgain->setChecked(true);
    else {
        ui->cbHighgain->setChecked(false);
    }

    if((value.data8[0] & 0x10) == 0x10)
        ui->cbNonuniform->setChecked(true);
    else {
        ui->cbNonuniform->setChecked(false);
    }

    if((value.data8[1] & 0x02) == 0x02)
        ui->cbIntegral->setChecked(true);
    else {
        ui->cbIntegral->setChecked(false);
    }
}

void ControlPanel::setIntegral(double integral)
{
    ui->editIntegral->setText(QString::number(integral, 10, 3));
}

void ControlPanel::setFrameCycle(double cycle)
{
    ui->editCycle->setText(QString::number(cycle, 10, 3));
    int maxv = int(cycle - 28);
    ui->editIntegral->setValidator(new QIntValidator(1, maxv, this));
}

void ControlPanel::onSetIntegral()
{
    double value = ui->editIntegral->text().toDouble();

    emit updateIntegral(value);
}

void ControlPanel::onSetCycle()
{
    double value = ui->editCycle->text().toDouble();
    emit updateCycle(value);
}

void ControlPanel::integralCheckBoxChanged(int state)
{
    m_pSettings->bIntegral = state == Qt::Checked;
    emit enableIntegral(m_pSettings->bIntegral);
}

void ControlPanel::histogramCheckBoxChanged(int state)
{
    m_pSettings->bHistogram = state == Qt::Checked;
}

void ControlPanel::highgainCheckBoxChanged(int state)
{
    m_pSettings->bHighgain = state == Qt::Checked;
    emit enableHighgain(m_pSettings->bHighgain);
}

void ControlPanel::nonuniformCheckBoxChanged(int state)
{
    m_pSettings->bCorrection = state == Qt::Checked;
    emit enableNonuniform(m_pSettings->bCorrection);
}

void ControlPanel::smoothCheckBoxChanged(int state)
{
    m_pSettings->bSmooth = state == Qt::Checked;
}

void ControlPanel::mirrorCheckBoxChanged(int state)
{
    m_pSettings->bMirror = state == Qt::Checked;
}

void ControlPanel::keepratioCheckBoxChanged(int state)
{
    m_pSettings->bKeepRatio = state == Qt::Checked;
}

