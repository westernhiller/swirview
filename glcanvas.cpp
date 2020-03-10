#include "glcanvas.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include "swirdialog.h"

GLCanvas::GLCanvas(QWidget *parent, QImage img)
    : QOpenGLWidget(parent)
    , m_clearColor(Qt::black)
    , m_image(img)
    , m_texture(nullptr)
    , m_program(nullptr)
    , m_pSettings(nullptr)
{
    if(parent)
        m_pSettings = static_cast<SwirDialog*>(parent)->getSettings();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(30);
}

GLCanvas::~GLCanvas()
{
    makeCurrent();
    if(m_texture)
        delete m_texture;
    m_vbo.destroy();
    delete m_program;
    doneCurrent();
}

void GLCanvas::onTimer()
{
    update();
}

QSize GLCanvas::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLCanvas::sizeHint() const
{
    return QSize(200, 200);
}

void GLCanvas::setClearColor(const QColor &color)
{
    m_clearColor = color;
}

void GLCanvas::initializeGL()
{
    initializeOpenGLFunctions();

    makeObject();
    if(!m_image.isNull())
        m_texture = new QOpenGLTexture(m_image);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);
    m_program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->link();

    m_program->bind();
    m_program->setUniformValue("texture", 0);
}

void GLCanvas::paintGL()
{
    glClearColor(m_clearColor.redF(), m_clearColor.greenF(),
                 m_clearColor.blueF(), m_clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 2.0f, 10.0f);
    m.translate(0.0f, 0.0f, -5.0f);

    m_program->setUniformValue("matrix", m);
    m_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    m_mutex.lock();
    int swiridx = 0;
    if(m_pSettings)
    {
        if(m_pSettings->bMirror)
        {
            if(m_pSettings->bKeepRatio)
                swiridx = 3;
            else {
                swiridx = 2;
            }
        }
        else if(m_pSettings->bKeepRatio)
        {
            swiridx = 1;
        }
        else {
            swiridx = 0;
        }
    }

    if(m_texture)
    {
        m_texture->bind();
        glDrawArrays(GL_TRIANGLE_FAN, swiridx * 4, 4);
    }
    m_mutex.unlock();
}
void GLCanvas::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
//    QString msg = tr("(") + QString::number(width) + tr(",") + QString::number(height) + tr(")");
//    emit sendMessage(msg);
}

void GLCanvas::makeObject()
{
    static const float coords[4][4][5] = {
        {{ +1, -1, -1, 1, 0 }, { -1, -1, -1, 0, 0 }, { -1, +1, -1, 0, 1 }, { +1, +1, -1, 1, 1 }},
        {{ +0.815, -1, -1, 1, 0 }, { -0.815f, -1, -1, 0, 0 }, { -0.815f, +1, -1, 0, 1 }, { +0.815, +1, -1, 1, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -1, -1, -1, 1, 0 }, { -1, +1, -1, 1, 1 }, { +1, +1, -1, 0, 1 }},
        {{ +0.815, -1, -1, 0, 0 }, { -0.815f, -1, -1, 1, 0 }, { -0.815f, +1, -1, 1, 1 }, { +0.815, +1, -1, 0, 1 }}
    };
    QVector<GLfloat> vertData;
    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; ++j)
        {
            // vertex position
            vertData.append(0.5f * coords[i][j][0]);
            vertData.append(0.5f * coords[i][j][1]);
            vertData.append(0.5f * coords[i][j][2]);

            vertData.append(coords[i][j][3]);
            vertData.append(coords[i][j][4]);
        }
    }

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

void GLCanvas::resizeCanvas(int width, int height)
{
    resizeGL(width, height);
}

void GLCanvas::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void GLCanvas::mouseReleaseEvent(QMouseEvent * event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if((abs(dx) > 7) && (abs(dy) > 7))
    {
        QRect rt = geometry();
        int x = m_lastPos.x() * m_image.width() / rt.width();
        int y = m_lastPos.y() * m_image.height() / rt.height();
        dx = dx * m_image.width() / rt.width();
        dy = dy * m_image.height() / rt.height();

        emit boxSelect(QRect(x, y, dx, dy));
    }
}

void GLCanvas::updateImage(QImage image)
{
    m_mutex.lock();
    m_image = image;
    if(m_texture)
        delete m_texture;
    m_texture = new QOpenGLTexture(image);
    m_mutex.unlock();
}

void GLCanvas::exportImage()
{
    m_mutex.lock();
    emit getImage(m_image);
    m_mutex.unlock();
}
