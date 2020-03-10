#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QKeyEvent>
#include <QMutex>
#include "global.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLCanvas(QWidget *parent = nullptr, QImage imgSwir = QImage());
    ~GLCanvas() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setClearColor(const QColor &color);
    inline QImage getImage() { return m_image;}
    void resizeCanvas(int width, int height);
signals:
    void getImage(QImage image);
    void sendMessage(QString);
    void boxSelect(QRect);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void makeObject();

    QColor m_clearColor;
    QImage m_image;
    QMutex m_mutex;
    QOpenGLTexture *m_texture;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vbo;
    SWIRSETTINGS*   m_pSettings;
    QPoint m_lastPos;

private slots:
    void    onTimer();

public slots:
    void    updateImage(QImage);
    void    exportImage();
};

#endif
