#include "global.h"
#include <QTime>
#include <QDebug>
#include <opencv2/xphoto.hpp>

using namespace std;
using namespace cv;

void msleep(unsigned int msec)
{
    QTime dueTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dueTime);
}

QImage mat2qimage(const Mat &mat)
{
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);          // Set the color table (used to translate colour indexes to qRgb values)          image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }      // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

Mat qimage2mat(const QImage &qimage)
{
    Mat mat, mat2;
    int from_to[] = {0, 0, 1, 1, 2, 2, 3, 3};
    switch(qimage.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC4, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        mat2 = Mat(mat.rows, mat.cols, CV_8UC4);
        mixChannels(&mat, 1, &mat2, 1, from_to, 4);
        return mat2;
        break;
    case QImage::Format_RGB888:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC3, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        mat2 = Mat(mat.rows, mat.cols, CV_8UC3);
        mixChannels(&mat, 1, &mat2, 1, from_to, 3);
        return mat2;
        break;
    case QImage::Format_Indexed8:
        mat = Mat(qimage.height(), qimage.width(), CV_8UC1, (uchar *)qimage.constBits(), qimage.bytesPerLine());
        break;
    default:
        break;
    }
    return mat;
}


