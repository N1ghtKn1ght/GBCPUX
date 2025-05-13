#pragma once

// Qt
#include <QObject>
#include <QQmlEngine>
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMutex>
#include <QThread>
#include <QTimer>

// project
#include "hardware/memory.h"
#include "hardware/cpu.h" 
#include "hardware/ppu.h"
#include "hardware/timer.h"

namespace GBCPUX {

class FrameRender : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions 
{
public:
    FrameRender(const uint8_t* buffer);
    ~FrameRender();

    void render() override;
private:
    QOpenGLTexture* m_texture;
    const uint8_t* m_buffer;
};

class GameBoyEmulator : public QQuickFramebufferObject 
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GameBoyEmulator)

public:
    GameBoyEmulator() {}

    Renderer* createRenderer() const override {
        return new FrameRender(m_cpu.buffer());
    }

public slots:
    void load(const QUrl& path);

private:
    Hardware::CPU m_cpu;
};


}