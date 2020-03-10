#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QImage>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "global.h"
#include "glcanvas.h"
#include "linepicker.h"

class Analyzer : public QWidget
{
    Q_OBJECT
public:
    explicit Analyzer(QWidget* parent = nullptr);
    ~Analyzer();

signals:
    void imageSaved(QString);

private:
    QImage          m_image;
    QLineEdit*      m_pImageName;
    QList<QColor>   m_colorlist;
    QList<int>      m_linelist;
    QList<QLineSeries*> m_seriesList;
    QList<LinePicker*> m_linepickerList;
    QChart  *       m_pChart;
    QChartView *    m_pChartView;
    QVector<QPointF > m_testData;
    QValueAxis *    m_axisX;
    QValueAxis *    m_axisY;
    SWIRSETTINGS*   m_pSettings;

    void createChart(QString title);
    QImage imageFromChart();

private slots:
    void updateImage(QImage);
    void addLine(int, int);
    void removeLine(int);
    void dumpImage();
};

#endif // ANALYZEDIALOG_H
