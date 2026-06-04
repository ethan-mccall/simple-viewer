/*
 * SimpleViewer
 * Copyright (C) 2026 Ethan McCall
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/icons/icon.ico"));

    qputenv("QT_LOGGING_RULES", "qt.gui.imageio=false;qt.imageio=false");

    QString fileToOpen;
    if (argc > 1) {
        fileToOpen = argv[1];
    }

    MainWindow w(fileToOpen);
    w.show();

    return a.exec();
}
