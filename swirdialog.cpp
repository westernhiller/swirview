#include "swirdialog.h"
#include <QLayout>
#include "swirprocessor.h"
#include "photothread.h"

SwirDialog::SwirDialog(QWidget *parent)
    : QDialog(parent)
    , m_nCapturers(3)
    , m_nFrames(0)
    , m_bRecording(false)
    , m_bCapturing(false)
    , m_bAnalyzing(false)
    , m_rtZoom(QRect(0, 0, SWIRWIDTH, SWIRHEIGHT))
{
    QSize size = qApp->screens()[0]->size();
    setFixedSize(size);

    setWindowFlags(Qt::FramelessWindowHint);
    loadConfig();

    createGUI(size);
    ImageBuffer* pImageBuffer = new ImageBuffer("source", 15, SWIRWIDTH, SWIRHEIGHT, CV_8UC1);
    if(!pImageBuffer)
    {
        qDebug() << "Failed creating image buffer!";
        return;
    }
    m_ImageBufferList.push_back(pImageBuffer);
    pImageBuffer = new ImageBuffer("video", 10, SWIRWIDTH, SWIRHEIGHT, CV_8UC1);
    if(!pImageBuffer)
    {
        qDebug() << "Failed creating video buffer!";
        return;
    }
    m_ImageBufferList.push_back(pImageBuffer);

    for(int i = 0; i < m_nCapturers; i++)
    {
        SwirCapturer* pCapturer = new SwirCapturer(this);
        connect(pCapturer, SIGNAL(getFrame(QByteArray)), this, SLOT(onFrame(QByteArray)));
        if(i == 0)
        {
            connect(pCapturer, SIGNAL(cameraConncted(bool)), m_pControl, SLOT(connected(bool)));
            connect(pCapturer, SIGNAL(updateMode(uint32_t)), m_pControl, SLOT(setMode(uint32_t)));
            connect(pCapturer, SIGNAL(updateIntegral(double)), m_pControl, SLOT(setIntegral(double)));
            connect(pCapturer, SIGNAL(updateCycle(double)), m_pControl, SLOT(setFrameCycle(double)));
            connect(m_pControl, SIGNAL(updateIntegral(double)), pCapturer, SLOT(setIntegral(double)));
            connect(m_pControl, SIGNAL(updateCycle(double)), pCapturer, SLOT(setCycle(double)));
            connect(m_pControl, SIGNAL(enableHighgain(bool)), pCapturer, SLOT(enableHighgain(bool)));
            connect(m_pControl, SIGNAL(enableNonuniform(bool)), pCapturer, SLOT(enableNonuniformityCorrection(bool)));
            connect(m_pControl, SIGNAL(enableIntegral(bool)), pCapturer, SLOT(enableIntegralAdjustion(bool)));
            connect(m_pControl, SIGNAL(adjustOnsite()), pCapturer, SLOT(adjustOnsite()));
        }
        m_CapturerList.push_back(pCapturer);
    }
    m_pDisplayer = new SwirDisplayer(this);
    m_pVideoSaver = new VideoThread(this);

    connect(this, SIGNAL(updateFPS(float)), m_pControl, SLOT(updateFPS(float)));
    connect(m_pControl, SIGNAL(connectCamera(bool)), this, SLOT(onConnectCamera(bool)));
    connect(m_pControl, SIGNAL(savePhoto()), this, SLOT(onSavePhoto()));
    connect(m_pControl, SIGNAL(startRecording()), this, SLOT(onStartRecording()));
    connect(m_pControl, SIGNAL(stopRecording()), this, SLOT(onStopRecording()));
    connect(m_pControl, SIGNAL(exit()), this, SLOT(onExit()));
    connect(this, SIGNAL(updatePatch(QImage)), m_pPatch, SLOT(updateImage(QImage)));
    connect(m_pCamera, SIGNAL(boxSelect(QRect)), this, SLOT(boxSelect(QRect)));
    connect(this, SIGNAL(updateImage(QImage)), m_pCamera, SLOT(updateImage(QImage)));
    connect(this, SIGNAL(updateAnalyzeImage(QImage)), m_pAnalyzer, SLOT(updateImage(QImage)));

    connect(m_pDisplayer, SIGNAL(display(QImage)), this, SLOT(onDisplay(QImage)));
    connect(this, SIGNAL(record(bool)), m_pDisplayer, SLOT(onRecord(bool)));
    connect(this, SIGNAL(record(bool)), m_pVideoSaver, SLOT(record(bool)));
    connect(m_pVideoSaver, SIGNAL(videoSaved(QString)), this, SLOT(onVideoSaved(QString)));
    connect(m_pAnalyzer, SIGNAL(imageSaved(QString)), this, SLOT(onPhotoSaved(QString)));

    m_pDisplayer->start();
    QTimer* t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(onTimer()));
    t->start(1000);
}

SwirDialog::~SwirDialog()
{
    if(m_pVideoSaver)
        delete m_pVideoSaver;
    if(m_pDisplayer)
        delete m_pDisplayer;
    for(int i = 0; i < m_nCapturers; i++)
        delete m_CapturerList[i];
    m_CapturerList.clear();

    for(int i = 0; i < m_ImageBufferList.size(); i++)
        delete m_ImageBufferList[i];
    m_ImageBufferList.clear();
}

ImageBuffer* SwirDialog::getImageBuffer(QString name)
{
    for(int i = 0; i < m_ImageBufferList.size(); i++)
    {
        BUFFERINFO* pBufInfo = m_ImageBufferList[i]->getBufferInfo();
        if(pBufInfo && (pBufInfo->name.compare(name) == 0))
            return m_ImageBufferList[i];
    }
    return nullptr;
}

void SwirDialog::createGUI(QSize screenSize)
{
    QImage imgSwir = QImage(tr(":/icons/swir.png"));
    QColor clearColor;
    clearColor.setHsv(255, 255, 63);

    QHBoxLayout* pLayout = new QHBoxLayout();
//    pLayout->setSpacing(0);
    pLayout->setMargin(0);
    QVBoxLayout* pVBLayout = new QVBoxLayout();
    m_pControl = new ControlPanel(this);
    m_pControl->setFixedSize(770, 512);
//    m_pControl->setFixedSize(screenSize.width()/2, screenSize.height()/2);
    pVBLayout->addWidget(m_pControl);
    m_pAnalyzer = new Analyzer(this);
    m_pAnalyzer->setFixedWidth(770);
    pVBLayout->addWidget(m_pAnalyzer);
    pLayout->addLayout(pVBLayout);

    QVBoxLayout* pCanvasLayout = new QVBoxLayout();
    m_pCamera = new GLCanvas(this, imgSwir);
    m_pCamera->setClearColor(clearColor);
    int height = (screenSize.width() - 820) / 4 * 3;
    m_pCamera->setFixedSize(screenSize.width() - 820, height);
    pCanvasLayout->addWidget(m_pCamera);

    m_pPatch = new GLCanvas(this, imgSwir);
    m_pPatch->setClearColor(clearColor);
    int h = screenSize.height() - height;
    int w = h / 3 * 4;
    m_pPatch->setFixedSize(w, h);
    pCanvasLayout->addWidget(m_pPatch, 1, Qt::AlignCenter);

    pLayout->addLayout(pCanvasLayout);

    setLayout(pLayout);
}

void SwirDialog::onDisplay(QImage image)
{
    emit updateImage(image);
    emit updatePatch(image.copy(m_rtZoom));
    if(m_bAnalyzing)
    {
        updateAnalyzeImage(image.copy());
        m_bAnalyzing = false;
    }
}

void SwirDialog::onConnectCamera(bool bConnect)
{
    for(int i = 0; i < m_nCapturers; i++)
    {
        if(bConnect)
            m_CapturerList[i]->start();
        else
            m_CapturerList[i]->stop();
    }
}

void SwirDialog::onStopRecording()
{
    emit record(false);
}

void SwirDialog::onStartRecording()
{
    m_pVideoSaver->start();
    emit record(true);
}

void SwirDialog::boxSelect(QRect rct)
{
    m_rtZoom = rct;
}

void SwirDialog::onExit()
{
    saveConfig();
    close();
}

void SwirDialog::keyPressEvent(QKeyEvent* event)
{
    int keyValue = event->key();
    if(keyValue & Qt::Key_Escape)
    {
        saveConfig();
        close();
    }
}

void SwirDialog::onTimer()
{
    static int pos = 0;
    static int framebuffer[] = {0,0,0,0,0,0,0,0,0,0};

    m_mutex.lock();
    framebuffer[pos++] = m_nFrames;
    m_nFrames = 0;
    m_mutex.unlock();

    if(pos == 10)
        pos = 0;

    int nSum = 0;
    for(int i = 0; i < 10; i++)
        nSum += framebuffer[i];
    float fFrame = (float)nSum / 10.0f;

    emit updateFPS(fFrame);

    m_bAnalyzing = true;
}

void SwirDialog::onSavePhoto()
{
    PhotoThread* pPhotoSaver = new PhotoThread(this);
    connect(pPhotoSaver, SIGNAL(photoSaved(QString)), this, SLOT(onPhotoSaved(QString)));
}

void SwirDialog::onPhotoSaved(QString filename)
{
    qDebug() << "Photo " << filename << " saved!";
    PhotoThread* pPhotoSaver = static_cast<PhotoThread*>(sender());
    delete pPhotoSaver;
}

void SwirDialog::onVideoSaved(QString filename)
{
    qDebug() << "Video " << filename << " saved!";
}

void SwirDialog::onFrame(QByteArray frame)
{
    m_mutex.lock();
    m_nFrames++;
    m_mutex.unlock();
    SwirProcessor* pProcessor = new SwirProcessor(frame, this);
    connect(pProcessor, SIGNAL(killMe()), this, SLOT(killProcessor()));
    pProcessor->start();
}

void SwirDialog::killProcessor()
{
    SwirProcessor* pProcessor = static_cast<SwirProcessor *>(sender());
    delete pProcessor;
}

void SwirDialog::loadConfig()
{
    QString pathDefault = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
#ifdef WIN32
    QSettings settings("HKEY_CURRENT_USER\\Software\\SwirView", QSettings::NativeFormat);
#else
    QSettings settings(pathDefault + "/.swirview.ini", QSettings::NativeFormat);
#endif
    settings.beginGroup("swir");
    m_settings.ip = settings.value("ipaddress", "192.168.1.10").toString();
    m_settings.port = settings.value("port", "2001").toInt();
    m_settings.nGain = settings.value("gain", "4").toInt();
    m_settings.bHighgain = settings.value("highgain", "false").toBool();
    m_settings.bSmooth = settings.value("smooth", "false").toBool();
    m_settings.bHistogram = settings.value("histogram", "false").toBool();
    m_settings.bIntegral = settings.value("integral", "false").toBool();
    m_settings.bCorrection = settings.value("correction", "false").toBool();
    settings.endGroup();

    settings.beginGroup("common");
    m_settings.path = settings.value("path", pathDefault).toString();
    m_settings.bKeepRatio = settings.value("keepratio", "false").toBool();
    m_settings.bMirror = settings.value("mirror", "false").toBool();

    settings.endGroup();
}

void SwirDialog::saveConfig()
{
#ifdef WIN32
    QSettings settings("HKEY_CURRENT_USER\\Software\\SwirView", QSettings::NativeFormat);
#else
    QString pathDefault = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QSettings settings(pathDefault + "/.swirview.ini", QSettings::NativeFormat);
#endif
    settings.beginGroup("swir");
    settings.setValue("ipaddress", m_settings.ip);
    settings.setValue("port", m_settings.port);
    settings.setValue("gain", m_settings.nGain);
    settings.setValue("highgain", m_settings.bHighgain);
    settings.setValue("smooth", m_settings.bSmooth);
    settings.setValue("histogram", m_settings.bHistogram);
    settings.setValue("integral", m_settings.bIntegral);
    settings.setValue("correction", m_settings.bCorrection);
    settings.endGroup();

    settings.beginGroup("Common");
    settings.setValue("path", m_settings.path);
    settings.setValue("keepratio", m_settings.bKeepRatio);
    settings.setValue("mirror", m_settings.bMirror);
    settings.endGroup();
}


