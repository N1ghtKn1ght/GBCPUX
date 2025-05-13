// qt 
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QIcon>

#include "tools/emulator.h"

int main(int argc, char* argv[])
{

    qputenv("QT_WIN_DEBUG_CONSOLE", "attach");
    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArray("Material"));
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", QByteArray("Dark"));

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/logo.ico"));

    QQmlApplicationEngine engine;
    engine.addImportPath(":/");

    QCoreApplication::setApplicationName("GBCPUX");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("GBCPUX", "Main");

    return app.exec();
}