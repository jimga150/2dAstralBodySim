#include <QGuiApplication>

#include "abswindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);

    ABSWindow simwindow;
    simwindow.setFormat(format);

    simwindow.show();
    simwindow.setAnimating(true);

    return app.exec();
}
