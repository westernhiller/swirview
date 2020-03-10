#include "analyzer.h"
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include "swirdialog.h"
#include "linepicker.h"

Analyzer::Analyzer(QWidget* parent)
    : QWidget(parent)
    , m_pSettings(nullptr)
{
    if(parent)
        m_pSettings = static_cast<SwirDialog*>(parent)->getSettings();

    m_colorlist.push_back(Qt::blue);
    m_colorlist.push_back(Qt::red);
    m_colorlist.push_back(Qt::yellow);
    m_colorlist.push_back(Qt::green);
    m_colorlist.push_back(Qt::white);

    m_linelist.push_back(SWIRHEIGHT/2);
    for(int i = 0; i < 4; i++)
        m_linelist.push_back(-1);

    QHBoxLayout* pLayout = new QHBoxLayout();
    QVBoxLayout* pControl = new QVBoxLayout();
    QPushButton* pbtnSnapshot = new QPushButton(QString::fromUtf8("快照"));
    pControl->addWidget(pbtnSnapshot);
    connect(pbtnSnapshot, SIGNAL(clicked()), this, SLOT(dumpImage()));
#ifdef WIN32
    pbtnSnapshot->setEnabled(false);
#endif
    QVBoxLayout* pTable = new QVBoxLayout();
    for(int i = 0; i < 5; i++)
    {
        LinePicker* pLinePicker = new LinePicker(i, m_linelist[i] > 0, m_linelist[i], m_colorlist[i]);
        pTable->addWidget(pLinePicker);
        connect(pLinePicker, SIGNAL(addLine(int,int)), this, SLOT(addLine(int,int)));
        connect(pLinePicker, SIGNAL(removeLine(int)), this, SLOT(removeLine(int)));
    }

    pControl->addLayout(pTable);
    pLayout->addLayout(pControl);
    createChart(QString::fromUtf8("图像分析"));

    for(int i = 0; i < 5; i++)
    {
        QLineSeries* pSeries = new QLineSeries();
        pSeries->setPen(QPen(m_colorlist[i]));
        m_pChart->addSeries(pSeries);
        pSeries->attachAxis(m_axisX);
        pSeries->attachAxis(m_axisY);
        m_seriesList.push_back(pSeries);
    }

    m_pChartView = new QChartView(m_pChart);
    pLayout->addWidget(m_pChartView);

    setLayout(pLayout);
}

Analyzer::~Analyzer()
{

}


void Analyzer::addLine(int index, int line)
{
    for(int i = 0; i < 5; i++)
    {
        if(m_linelist[i] == line)
        {
            m_linepickerList[index]->deselect();
            return;
        }
    }

    m_linelist[index] = line;
}

void Analyzer::removeLine(int index)
{
    m_linelist[index] = -1;
}

void Analyzer::createChart(QString title)
{
    m_pChart = new QChart();
    m_pChart->legend()->hide();
    m_axisX = new QValueAxis;
    m_axisX->setTickCount(9);
    m_axisX->setMinorTickCount(1);
    m_axisY = new QValueAxis;
    m_axisY->setTickCount(17);
    m_axisY->setMinorTickCount(1);
    m_pChart->addAxis(m_axisX, Qt::AlignBottom);
    m_pChart->addAxis(m_axisY, Qt::AlignLeft);
    m_pChart->setTitle(title);
    m_pChart->setTheme(static_cast<QChart::ChartTheme>(2));

    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText(tr("X coordinate"));
    m_axisX->setRange(0, SWIRWIDTH);
    m_axisY->setRange(0, 256);
    m_axisY->setTitleText(tr("gray scale value"));
}

void Analyzer::dumpImage()
{
    QImage snapshot = imageFromChart();
    QDateTime timeNow = QDateTime::currentDateTime();
    QString time = timeNow.toString("yyyy-MM-dd-HH-mm-ss-zzz");
    QString path = m_pSettings ? m_pSettings->path : QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString filename = path + tr("/") + time + ".png";

    snapshot.save(filename);
    emit imageSaved(filename);
}

QImage Analyzer::imageFromChart()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    // same coordinate as geometry at line 196
    QRect rt = m_pChartView->geometry();
    QPixmap p = screen->grabWindow(winId(), rt.x(), rt.y(), rt.width(), rt.height());
    return p.toImage();
}

void Analyzer::updateImage(QImage image)
{
    m_image = image;
    cv::Mat matImg = qimage2mat(image);

    for(int i = 0; i < 5; i++)
    {
        QVector<QPointF > testData;
        if(m_linelist[i] > 0)
        {
            uchar* ptr = matImg.ptr<uchar>(m_linelist[i]);
            for(int j = 0; j < matImg.cols; j++)
            {
                testData.push_back(QPointF(j, ptr[j]));
            }
        }
        m_seriesList[i]->replace(testData);
    }
}
