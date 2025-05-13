
// Qt
#include <QQmlApplicationEngine>
#include <QFile>
#include <QMutexLocker>

// project
#include "emulator.h"
#include "hardware/tools/commontoolkit.h"

namespace GBCPUX {
FrameRender::FrameRender(const uint8_t* buffer)
    : m_texture(nullptr), m_buffer(buffer)
{
    initializeOpenGLFunctions();

    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_texture->setSize(Hardware::SCREEN_WIDTH, Hardware::SCREEN_HEIGHT);  
    m_texture->setFormat(QOpenGLTexture::RGBA8_UNorm); 
    m_texture->setMinificationFilter(QOpenGLTexture::Linear); 
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear); 
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
 
}

FrameRender::~FrameRender()
{
    if (m_texture) {
        delete m_texture;
    }
}

void FrameRender::render()
{
    const QColor palette[] = {
        QColor(0x00, 0x80, 0x00),
        QColor(0x00, 0x00, 0x80),
        QColor(0x00, 0x80, 0x00),
        QColor(0xFF, 0xFF, 0x00)
    };

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    std::array<uint8_t, 0x5A00 * 4> buffer;
    for (size_t i = 0; i < 0x5A00; ++i) {
        QColor color = palette[m_buffer[i]];
        buffer[i * 4 + 0] = color.red();
        buffer[i * 4 + 1] = color.green();
        buffer[i * 4 + 2] = color.blue();
        buffer[i * 4 + 3] = 255;
    }

    m_texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, buffer.data());

    glEnable(GL_TEXTURE_2D);
    m_texture->bind();

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(1, 0); glVertex2f(1, -1);
    glTexCoord2f(1, 1); glVertex2f(1, 1);
    glTexCoord2f(0, 1); glVertex2f(-1, 1);
    glEnd();

    m_texture->release();

    update();
}

void GameBoyEmulator::load(const QUrl& path)
{
    m_cpu.stop();
    m_cpu.start(path.toLocalFile().toStdString());
}

}