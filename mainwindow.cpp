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
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QDir>
#include <QMenuBar>
#include <QWheelEvent>
#include <QFileInfo>
#include <QTimer>
#include <QResizeEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QImageReader>
#include <QMovie>
#include <QWindow>
#include <QWindowStateChangeEvent>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QClipboard>
#include <QDebug>
#include <QBuffer>
#include <QIODevice>
#include <QInputDialog>
#include <QLineEdit>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(const QString &fileToOpen, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Simple Viewer");
    setWindowIcon(QIcon(":/icons/icon.ico"));
    resize(1000, 700);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect geo = screen->geometry();
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }

    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);

    view->setRenderHint(QPainter::SmoothPixmapTransform, true);
    view->setRenderHint(QPainter::Antialiasing, true);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setTransformationAnchor(QGraphicsView::NoAnchor);
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    view->setDragMode(QGraphicsView::NoDrag);

    setCentralWidget(view);
    setAcceptDrops(true);

    prevImageButton = new QPushButton("‹", view);
    nextImageButton = new QPushButton("›", view);

    const int btnSz = 28;
    prevImageButton->setFixedSize(btnSz, btnSz);
    nextImageButton->setFixedSize(btnSz, btnSz);
    prevImageButton->setFlat(true);
    nextImageButton->setFlat(true);
    prevImageButton->setCursor(Qt::PointingHandCursor);
    nextImageButton->setCursor(Qt::PointingHandCursor);

    prevImageButton->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #ccc;"
        "  border: none;"
        "  font-size: 18px;"
        "  padding: 0px 2px 3px 0px;"
        "  margin: 0px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(90, 90, 90, 130);"
        "  color: #ddd;"
        "  border: 1px solid rgba(140, 140, 140, 70);"
        "  border-radius: 14px;"
        "}"
        "QPushButton:disabled {"
        "  background-color: transparent;"
        "  color: #666;"
        "  border: none;"
        "}"
    );
    nextImageButton->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #ccc;"
        "  border: none;"
        "  font-size: 18px;"
        "  padding: 0px 0px 3px 2px;"
        "  margin: 0px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(90, 90, 90, 130);"
        "  color: #ddd;"
        "  border: 1px solid rgba(140, 140, 140, 70);"
        "  border-radius: 14px;"
        "}"
        "QPushButton:disabled {"
        "  background-color: transparent;"
        "  color: #666;"
        "  border: none;"
        "}"
    );

    prevImageButton->hide();
    nextImageButton->hide();

    connect(prevImageButton, &QPushButton::clicked, this, &MainWindow::goToPreviousImage);
    connect(nextImageButton, &QPushButton::clicked, this, &MainWindow::goToNextImage);

    fullscreenButton = new QPushButton("⛶", view);
    fullscreenButton->setFixedSize(btnSz, btnSz);
    fullscreenButton->setFlat(true);
    fullscreenButton->setCursor(Qt::PointingHandCursor);
    fullscreenButton->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #ccc;"
        "  border: none;"
        "  font-size: 13px;"
        "  padding: 0px 0px 1px 1px;"
        "  margin: 0px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(90, 90, 90, 130);"
        "  color: #ddd;"
        "  border: 1px solid rgba(140, 140, 140, 70);"
        "  border-radius: 14px;"
        "}"
        "QPushButton:disabled {"
        "  background-color: transparent;"
        "  color: #666;"
        "  border: none;"
        "}"
    );

    fullscreenButton->show();

    connect(fullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);

    view->viewport()->installEventFilter(this);

    setBackgroundColor(Background::Grey);

    openAction = new QAction("Open Image", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setIcon(QIcon::fromTheme("document-open"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    addAction(openAction);

    fitAction = new QAction("Fit to Window", this);
    fitAction->setShortcut(Qt::Key_F);
    fitAction->setIcon(QIcon::fromTheme("zoom-fit-best"));
    connect(fitAction, &QAction::triggered, this, &MainWindow::fitImageToWindow);
    addAction(fitAction);

    zoomInAction = new QAction("Zoom In", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setIcon(QIcon::fromTheme("zoom-in"));
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    addAction(zoomInAction);

    zoomOutAction = new QAction("Zoom Out", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setIcon(QIcon::fromTheme("zoom-out"));
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    addAction(zoomOutAction);

    autoResizeAction = new QAction("Auto Resize", this);
    autoResizeAction->setCheckable(true);
    autoResizeAction->setChecked(true);
    autoResizeAction->setIcon(QIcon::fromTheme("zoom-original"));
    addAction(autoResizeAction);

    connect(autoResizeAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            Qt::WindowStates st = windowState();
            if (st & (Qt::WindowMaximized | Qt::WindowFullScreen)) {
                fitImageToWindow();
            }
        }
    });

    resetAction = new QAction("Reset to Original", this);
    resetAction->setShortcut(Qt::CTRL | Qt::Key_R);
    resetAction->setIcon(QIcon::fromTheme("edit-undo"));
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetToOriginal);
    addAction(resetAction);

    saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon::fromTheme("document-save"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    addAction(saveAction);

    saveAsAction = new QAction("Save As", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveImageAs);
    addAction(saveAsAction);

    copyAction = new QAction("Copy", this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyImage);
    addAction(copyAction);

    copyToAction = new QAction("Copy To", this);
    copyToAction->setIcon(QIcon::fromTheme("edit-copy"));
    connect(copyToAction, &QAction::triggered, this, &MainWindow::copyImageTo);
    addAction(copyToAction);

    moveToAction = new QAction("Move To", this);
    moveToAction->setIcon(QIcon::fromTheme("folder-move"));
    connect(moveToAction, &QAction::triggered, this, &MainWindow::moveImageTo);
    addAction(moveToAction);

    deleteAction = new QAction("Delete", this);
    deleteAction->setShortcut(Qt::Key_Delete);
    deleteAction->setIcon(QIcon::fromTheme("edit-delete"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteImage);
    addAction(deleteAction);

    renameAction = new QAction("Rename", this);
    renameAction->setShortcut(Qt::Key_F2);
    renameAction->setIcon(QIcon::fromTheme("edit-rename"));
    connect(renameAction, &QAction::triggered, this, &MainWindow::renameImage);
    addAction(renameAction);

    aboutAction = new QAction("About App", this);
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    addAction(aboutAction);

    propertiesAction = new QAction("Properties", this);
    propertiesAction->setIcon(QIcon::fromTheme("document-properties"));
    connect(propertiesAction, &QAction::triggered, this, &MainWindow::showProperties);
    addAction(propertiesAction);

    animationToggleAction = new QAction("Pause/Play Animation", this);
    animationToggleAction->setShortcuts({QKeySequence(Qt::Key_P), QKeySequence(Qt::Key_Space)});
    animationToggleAction->setIcon(QIcon::fromTheme("media-playback-start"));
    connect(animationToggleAction, &QAction::triggered, this, &MainWindow::toggleAnimation);
    addAction(animationToggleAction);

    rotateLeftAction = new QAction("Rotate Left", this);
    rotateLeftAction->setShortcut(Qt::CTRL | Qt::Key_BracketLeft);
    rotateLeftAction->setIcon(QIcon::fromTheme("object-rotate-left"));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);
    addAction(rotateLeftAction);

    rotateRightAction = new QAction("Rotate Right", this);
    rotateRightAction->setShortcut(Qt::CTRL | Qt::Key_BracketRight);
    rotateRightAction->setIcon(QIcon::fromTheme("object-rotate-right"));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);
    addAction(rotateRightAction);

    mirrorAction = new QAction("Mirror", this);
    mirrorAction->setShortcut(Qt::CTRL | Qt::Key_H);
    mirrorAction->setIcon(QIcon::fromTheme("object-flip-horizontal"));
    connect(mirrorAction, &QAction::triggered, this, &MainWindow::mirrorImage);
    addAction(mirrorAction);

    flipAction = new QAction("Flip", this);
    flipAction->setIcon(QIcon::fromTheme("object-flip-vertical"));
    connect(flipAction, &QAction::triggered, this, &MainWindow::flipImage);
    addAction(flipAction);

    resizeAction = new QAction("Resize...", this);
    resizeAction->setIcon(QIcon::fromTheme("transform-scale"));
    connect(resizeAction, &QAction::triggered, this, &MainWindow::resizeImage);
    addAction(resizeAction);

    prevAction = new QAction("Previous Image", this);
    prevAction->setShortcut(Qt::Key_Left);
    prevAction->setIcon(QIcon::fromTheme("go-previous"));
    connect(prevAction, &QAction::triggered, this, &MainWindow::goToPreviousImage);
    addAction(prevAction);

    nextAction = new QAction("Next Image", this);
    nextAction->setShortcut(Qt::Key_Right);
    nextAction->setIcon(QIcon::fromTheme("go-next"));
    connect(nextAction, &QAction::triggered, this, &MainWindow::goToNextImage);
    addAction(nextAction);

    fullscreenAction = new QAction("Full Screen", this);
    fullscreenAction->setShortcut(Qt::Key_F11);
    fullscreenAction->setCheckable(true);
    fullscreenAction->setIcon(QIcon::fromTheme("view-fullscreen"));
    connect(fullscreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    addAction(fullscreenAction);

    menuBar()->hide();

    updateNavigationState();

    if (!fileToOpen.isEmpty()) {
        QTimer::singleShot(0, this, [this, fileToOpen]() {
            loadImage(fileToOpen);
        });
    } else {
        QString welcomeHtml = loadWelcomeHtml();
        welcomeItem = new QGraphicsTextItem();
        scene->addItem(welcomeItem);
        welcomeItem->setHtml(welcomeHtml);
        welcomeItem->setDefaultTextColor(Qt::white);
        welcomeItem->setTextWidth(600);

        QTextDocument *doc = welcomeItem->document();
        QTextOption option = doc->defaultTextOption();
        option.setAlignment(Qt::AlignCenter);
        doc->setDefaultTextOption(option);

        welcomeItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        if (view && view->viewport() && view->viewport()->width() > 10) {
            centerWelcomeItem();
        }
    }

    if (welcomeItem) {
        QTimer::singleShot(0, this, [this]() {
            if (welcomeItem) {
                centerWelcomeItem();
            }
        });
    }
}

void MainWindow::loadImage(const QString &filePath)
{
    if (filePath.isEmpty()) return;

    prepareForNewImage();

    QImageReader reader(filePath);
    reader.setAllocationLimit(0);

    bool useMovie = reader.supportsAnimation();
    if (useMovie) {
        currentMovie = new QMovie(filePath, QByteArray(), this);
        if (!currentMovie->isValid()) {
            delete currentMovie;
            currentMovie = nullptr;
            useMovie = false;
        } else {
            int fc = currentMovie->frameCount();
            if (fc > 0 && fc <= 1) {
                delete currentMovie;
                currentMovie = nullptr;
                useMovie = false;
            }
        }
    }

    if (useMovie && currentMovie) {
        isAnimated = true;
        currentMovie->jumpToFrame(0);
        QPixmap pixmap = currentMovie->currentPixmap();
        if (pixmap.isNull()) {
            cleanupMovie();
            QMessageBox::warning(this, "Error", "Animated image loaded but first frame was empty:\n" + filePath);
            return;
        }

        currentFilePath = filePath;
        originalWasAnimated = true;
        originalSourcePath = filePath;
        originalPixmap = QPixmap();
        isCurrentlyCropped = false;

        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
        item->setTransformationMode(Qt::SmoothTransformation);

        const qreal w = pixmap.width();
        const qreal h = pixmap.height();
        item->setPos(-w / 2.0, -h / 2.0);

        scene->addItem(item);

        updateSceneRectAroundItem(item);

        pendingInitialFit = true;

        if (view->viewport()->width() > 50 && view->viewport()->height() > 50) {
            fitImageToWindow();
            pendingInitialFit = false;
        }

        setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(filePath).fileName()));

        updateFolderImages(filePath);

        connect(currentMovie, &QMovie::frameChanged, this, &MainWindow::onMovieFrameChanged);
        currentMovie->start();
        return;
    }

    QImage image;
    if (reader.format() == "ico") {
        int bestArea = 0;
        for (int i = 0; i < reader.imageCount(); ++i) {
            if (reader.jumpToImage(i)) {
                QImage candidate = reader.read();
                if (!candidate.isNull()) {
                    int area = candidate.width() * candidate.height();
                    if (area > bestArea) {
                        image = candidate;
                        bestArea = area;
                    }
                }
            }
        }
    } else {
        image = reader.read();
    }
    if (image.isNull()) {
        QString err = reader.errorString();
        QMessageBox::warning(this, "Error",
            "Could not load:\n" + filePath + (err.isEmpty() ? QString() : "\n\n" + err));
        return;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Loaded image data but could not create displayable pixmap for:\n" + filePath);
        return;
    }

    currentFilePath = filePath;
    originalWasAnimated = false;
    originalSourcePath.clear();
    originalPixmap = pixmap;
    isCurrentlyCropped = false;

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    item->setTransformationMode(Qt::SmoothTransformation);

    const qreal w = pixmap.width();
    const qreal h = pixmap.height();
    item->setPos(-w / 2.0, -h / 2.0);

    scene->addItem(item);

    updateSceneRectAroundItem(item);

    pendingInitialFit = true;

    if (view->viewport()->width() > 50 && view->viewport()->height() > 50) {
        fitImageToWindow();
        pendingInitialFit = false;
    }

    setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(filePath).fileName()));

    updateFolderImages(filePath);
}

void MainWindow::fitImageToWindow()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;
    view->fitInView(item, Qt::KeepAspectRatio);
    QPointF itemCenter = item->pos() + QPointF(item->pixmap().width() / 2.0, item->pixmap().height() / 2.0);
    view->centerOn(itemCenter);

    updateSceneRectAroundItem(item);

    if (cropPanel && cropPanel->isVisible()) {
        updateCropPanelPosition();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        bool startedEdit = false;
        if (cropRectItem && (event->button() == Qt::LeftButton)) {
            QPoint local = viewportLocalPos(event);
            QPointF sp = view->mapToScene(local);
            Handle hitHandle = NoHandle;
            QGraphicsItem* hitItem = nullptr;
            auto hits = scene->items(sp, Qt::IntersectsItemShape, Qt::DescendingOrder);
            for (auto* it : hits) {
                if (handleForItem.contains(it)) {
                    hitHandle = handleForItem.value(it);
                    hitItem = it;
                    break;
                } else if (it == cropRectItem) {
                    hitItem = it;
                    break;
                }
            }
            if (hitHandle != NoHandle) {
                activeHandle = hitHandle;
                originalRect = cropRectItem->rect();
                dragStartPos = sp;
                setCursorForHandle(activeHandle);
                startedEdit = true;
            } else if (hitItem == cropRectItem) {
                isMovingRect = true;
                originalRect = cropRectItem->rect();
                dragStartPos = sp;
                view->viewport()->setCursor(Qt::SizeAllCursor);
                startedEdit = true;
            }
        }
        if (!startedEdit && pixmapItem()) {
            dragging = true;
            lastMousePos = viewportLocalPos(event);
            view->viewport()->setCursor(Qt::ClosedHandCursor);
        }
    } else if (event->button() == Qt::RightButton) {
        QGraphicsPixmapItem *item = pixmapItem();
        if (item && !isAnimated) {
            clearCropSelection();
            if (rubberBand) {
                rubberBand->hide();
                delete rubberBand;
                rubberBand = nullptr;
            }
            cropping = true;
            cropRubberStart = viewportLocalPos(event);

            rubberBand = new QRubberBand(QRubberBand::Rectangle, view->viewport());
            rubberBand->setGeometry(QRect(cropRubberStart, QSize(0, 0)));
            rubberBand->show();

            view->viewport()->setCursor(Qt::CrossCursor);
        } else if (item && isAnimated) {
            view->viewport()->setCursor(Qt::ArrowCursor);
        } else {
            view->viewport()->setCursor(Qt::ArrowCursor);
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPointF oldScenePos = view->mapToScene(lastMousePos.toPoint());
        QPoint localNew = viewportLocalPos(event);
        QPointF newScenePos = view->mapToScene(localNew);

        QPointF delta = newScenePos - oldScenePos;

        view->translate(delta.x(), delta.y());

        lastMousePos = localNew;

        updateSceneRectAroundItem(pixmapItem());
    } else if (activeHandle != NoHandle && cropRectItem) {
        QPoint local = viewportLocalPos(event);
        QPointF currScene = view->mapToScene(local);
        QRectF newR = computeResizedRect(originalRect, activeHandle, currScene);
        cropRectItem->setRect(newR);
        updateResizeHandles();
    } else if (isMovingRect && cropRectItem) {
        QPoint local = viewportLocalPos(event);
        QPointF currScene = view->mapToScene(local);
        QPointF delta = currScene - dragStartPos;
        QRectF newR = originalRect.translated(delta);
        cropRectItem->setRect(newR);
        updateResizeHandles();
    } else if (cropping && rubberBand) {
        QPoint currWidget = viewportLocalPos(event);
        QRect wr;
        if (cropAspectRatio > 0.0) {
            wr = computeConstrainedRubberRect(cropRubberStart, currWidget, cropAspectRatio);
        } else {
            wr = QRect(cropRubberStart, currWidget);
        }
        rubberBand->setGeometry(wr.normalized());
    }

    if (!dragging && activeHandle == NoHandle && !isMovingRect && !cropping) {
        QPoint local = viewportLocalPos(event);
        QPointF sp = view->mapToScene(local);
        Handle h = NoHandle;
        auto hits = scene->items(sp, Qt::IntersectsItemShape, Qt::DescendingOrder);
        for (auto* it : hits) {
            if (handleForItem.contains(it)) {
                h = handleForItem.value(it);
                break;
            }
        }
        if (h != NoHandle) {
            setCursorForHandle(h);
        } else if (cropRectItem) {
            QPointF localInItem = cropRectItem->mapFromScene(sp);
            if (cropRectItem->rect().contains(localInItem)) {
                view->viewport()->setCursor(Qt::SizeAllCursor);
            } else {
                view->viewport()->setCursor(defaultCursorShape);
            }
        } else {
            view->viewport()->setCursor(defaultCursorShape);
        }
    }

    if (cropPanel && cropPanel->isVisible()) {
        updateCropPanelPosition();
    }

    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        if (activeHandle != NoHandle || isMovingRect) {
            activeHandle = NoHandle;
            isMovingRect = false;
            defaultCursorShape = Qt::ArrowCursor;
            view->viewport()->setCursor(defaultCursorShape);
        } else if (dragging) {
            dragging = false;
            defaultCursorShape = Qt::OpenHandCursor;
            view->viewport()->setCursor(defaultCursorShape);
        }
    } else if (event->button() == Qt::RightButton) {
        if (cropping) {
            cropping = false;
            QRect bandGeom;
            if (rubberBand) {
                bandGeom = rubberBand->geometry();
                rubberBand->hide();
                delete rubberBand;
                rubberBand = nullptr;
            }
            if (bandGeom.width() >= 5 && bandGeom.height() >= 5) {
                QPointF sceneTopLeft = view->mapToScene(bandGeom.topLeft());
                QPointF sceneBottomRight = view->mapToScene(bandGeom.bottomRight());
                QRectF sceneRect(sceneTopLeft, sceneBottomRight);

                cropRectItem = new QGraphicsRectItem();
                cropRectItem->setRect(sceneRect.normalized());
                QPen solidPen(QColor(100, 150, 255), 1.0);
                solidPen.setStyle(Qt::SolidLine);
                solidPen.setCosmetic(true);
                cropRectItem->setPen(solidPen);
                cropRectItem->setBrush(QBrush(QColor(100, 150, 255, 40)));
                cropRectItem->setZValue(10);
                scene->addItem(cropRectItem);

                createResizeHandles();
                showCropPanel();

                view->viewport()->setCursor(Qt::ArrowCursor);
            } else {
                view->viewport()->setCursor(Qt::ArrowCursor);
                showContextMenu(event->globalPosition().toPoint());
            }
        } else if (pixmapItem() && isAnimated) {
            view->viewport()->setCursor(Qt::ArrowCursor);
            showContextMenu(event->globalPosition().toPoint());
        } else if (!pixmapItem()) {
            view->viewport()->setCursor(Qt::ArrowCursor);
            showContextMenu(event->globalPosition().toPoint());
        }
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    QSize newViewSize = view && view->viewport() ? view->viewport()->size() : QSize();

    if (!pixmapItem() && welcomeItem) {
        centerWelcomeItem();
    }

    if (pendingInitialFit && view->viewport()->width() > 50) {
        pendingInitialFit = false;
        fitImageToWindow();
    } else if (pendingAutoFit && autoResizeAction && autoResizeAction->isChecked()) {
        pendingAutoFit = false;
        bool wasExiting = m_exitingFullMode;
        m_exitingFullMode = false;
        if (view && view->viewport() && (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen))) {
            m_lastFullViewSize = view->viewport()->size();
        } else if (wasExiting) {
            m_lastFullViewSize = QSize();
        }
        fitImageToWindow();
    } else if (autoResizeAction && autoResizeAction->isChecked() &&
               ((windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen)) || m_exitingFullMode)) {

        pendingAutoFit = false;
        bool wasExiting = m_exitingFullMode;
        m_exitingFullMode = false;
        if (view && view->viewport() && (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen))) {
            m_lastFullViewSize = view->viewport()->size();
        } else if (wasExiting) {
            m_lastFullViewSize = QSize();
        }
        fitImageToWindow();
    } else if (autoResizeAction && autoResizeAction->isChecked() && !m_lastAutoResizeViewSize.isEmpty()) {
        int dw = qAbs(newViewSize.width() - m_lastAutoResizeViewSize.width());
        int dh = qAbs(newViewSize.height() - m_lastAutoResizeViewSize.height());

        if (dw > 48 || dh > 48 ||
            (m_lastAutoResizeViewSize.width() > 0 && dw > m_lastAutoResizeViewSize.width() / 6) ||
            (m_lastAutoResizeViewSize.height() > 0 && dh > m_lastAutoResizeViewSize.height() / 6)) {
            fitImageToWindow();
        } else {
            updateSceneRectAroundItem(pixmapItem());
        }
    } else {
        updateSceneRectAroundItem(pixmapItem());
    }

    if (cropPanel && cropPanel->isVisible()) {
        updateCropPanelPosition();
    }

    positionFloatingButtons();

    m_lastAutoResizeViewSize = newViewSize;

    if (!pixmapItem() && welcomeItem) {
        centerWelcomeItem();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            wheelEvent(wheel);
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent *mouseEv = static_cast<QMouseEvent *>(event);

            if (event->type() == QEvent::MouseButtonPress)
                mousePressEvent(mouseEv);
            else if (event->type() == QEvent::MouseMove)
                mouseMoveEvent(mouseEv);
            else if (event->type() == QEvent::MouseButtonRelease)
                mouseReleaseEvent(mouseEv);
            else if (event->type() == QEvent::MouseButtonDblClick)
                mouseDoubleClickEvent(mouseEv);

            return true;
        }

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEv = static_cast<QKeyEvent *>(event);
            keyPressEvent(keyEv);
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        fitImageToWindow();
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        bool canceled = false;

        if (rubberBand) {
            rubberBand->hide();
            delete rubberBand;
            rubberBand = nullptr;
            hideCropPanel();
            canceled = true;
        }

        if (cropRectItem) {
            clearCropSelection();
            canceled = true;
        }

        if (canceled) {
            cropping = false;
            activeHandle = NoHandle;
            isMovingRect = false;
            view->viewport()->setCursor(Qt::ArrowCursor);
            event->accept();
            return;
        }

        if (windowState() & Qt::WindowFullScreen) {
            showNormal();
            if (fullscreenAction) {
                fullscreenAction->setChecked(false);
            }
            positionFloatingButtons();
            event->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        auto *stateChange = static_cast<QWindowStateChangeEvent *>(event);
        Qt::WindowStates oldState = stateChange->oldState();
        Qt::WindowStates newState = windowState();

        Qt::WindowStates relevant = Qt::WindowMaximized | Qt::WindowFullScreen;
        if (((oldState & relevant) != (newState & relevant)) &&
            autoResizeAction && autoResizeAction->isChecked()) {
            pendingAutoFit = true;

            bool wasFull = (oldState & relevant);
            bool isFull = (newState & relevant);
            if (wasFull && !isFull) {
                m_exitingFullMode = true;
            }

            QTimer::singleShot(0, this, [this]() {
                if (pendingAutoFit && autoResizeAction && autoResizeAction->isChecked()) {
                    QSize curV = (view && view->viewport()) ? view->viewport()->size() : QSize();
                    bool stillOnFullSize = !m_lastFullViewSize.isEmpty() && (curV == m_lastFullViewSize);
                    if (m_exitingFullMode && stillOnFullSize) {
                        QTimer::singleShot(16, this, [this]() {
                            if (pendingAutoFit && autoResizeAction && autoResizeAction->isChecked()) {
                                pendingAutoFit = false;
                                bool wasExiting = m_exitingFullMode;
                                m_exitingFullMode = false;
                                if (wasExiting) {
                                    m_lastFullViewSize = QSize();
                                }
                                fitImageToWindow();
                            }
                        });
                        return;
                    }
                    pendingAutoFit = false;
                    bool wasExiting = m_exitingFullMode;
                    m_exitingFullMode = false;
                    if (wasExiting) {
                        m_lastFullViewSize = QSize();
                    }
                    fitImageToWindow();
                }
            });
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::openImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Image", QDir::homePath(),
                                                    "Images (*.jpg *.jpeg *.png *.bmp *.gif *.webp *.tiff *.tif *.tga *.svg *.ico *.avif *.icns *.heic *.heif)");
    loadImage(filePath);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        loadImage(event->mimeData()->urls().first().toLocalFile());
    }
}

QGraphicsPixmapItem* MainWindow::pixmapItem() const
{
    for (auto *i : scene->items()) {
        if (auto *p = qgraphicsitem_cast<QGraphicsPixmapItem*>(i))
            return p;
    }
    return nullptr;
}

void MainWindow::updateSceneRectAroundItem(QGraphicsPixmapItem *item)
{
    Q_UNUSED(item);

    if (scene->items().isEmpty()) return;

    QRectF vis = view->mapToScene(view->viewport()->rect()).boundingRect();
    if (!vis.isValid() || vis.width() < 1.0 || vis.height() < 1.0) {
        if (item) {
            const qreal w = item->pixmap().width();
            const qreal h = item->pixmap().height();
            QPointF center = item->pos() + QPointF(w / 2.0, h / 2.0);
            const qreal margin = qMax(w, h) * 3.0;
            scene->setSceneRect(center.x() - margin, center.y() - margin, margin * 2, margin * 2);
        }
        return;
    }

    QRectF itemsB;
    for (auto *it : scene->items()) {
        itemsB = itemsB.united(it->sceneBoundingRect());
    }

    QRectF target = vis;
    if (itemsB.isValid()) {
        target = target.united(itemsB);
    }

    QPointF c = target.center();
    qreal vw = vis.width();
    qreal vh = vis.height();
    qreal m = qMax(vw, vh) * 3.5;
    if (itemsB.isValid()) {
        m = qMax(m, qMax(itemsB.width(), itemsB.height()) * 0.75);
    }

    scene->setSceneRect(c.x() - m, c.y() - m, m * 2.0, m * 2.0);
}

void MainWindow::centerWelcomeItem()
{
    if (!welcomeItem || !view || !view->viewport()) return;

    QRectF vis = view->mapToScene(view->viewport()->rect()).boundingRect();
    qreal desiredW = qMin(qreal(600), vis.width() * 0.6);
    desiredW = qMax(desiredW, qreal(200));
    welcomeItem->setTextWidth(desiredW);

    QRectF br = welcomeItem->boundingRect();
    QPointF c = vis.center();
    welcomeItem->setPos(c.x() - br.width() / 2.0, c.y() - br.height() / 2.0);
}

QString MainWindow::loadWelcomeHtml() const
{
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/welcome.txt",
        QDir::currentPath() + "/welcome.txt",
        "welcome.txt"
    };

    for (const QString& path : searchPaths) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString text = in.readAll();
            if (!text.isEmpty()) {
                return text;
            }
        }
    }

    return "<h2>Simple Viewer</h2>"
           "<p>Edit <b>welcome.txt</b> (next to the executable or in the project root) to customize this text.<br/>"
           "Use basic HTML for rich formatting and centering.</p>";
}

QPoint MainWindow::viewportLocalPos(QMouseEvent *event) const
{
    return view->viewport()->mapFromGlobal(event->globalPosition().toPoint());
}

void MainWindow::createResizeHandles()
{
    if (!cropRectItem) return;
    removeResizeHandles();

    struct Def { Handle h; qreal fx; qreal fy; };
    QList<Def> defs = {
        {TopLeftHandle, 0.0, 0.0},
        {TopHandle, 0.5, 0.0},
        {TopRightHandle, 1.0, 0.0},
        {LeftHandle, 0.0, 0.5},
        {RightHandle, 1.0, 0.5},
        {BottomLeftHandle, 0.0, 1.0},
        {BottomHandle, 0.5, 1.0},
        {BottomRightHandle, 1.0, 1.0},
    };

    for (const Def& d : defs) {
        auto* hi = new QGraphicsEllipseItem();
        hi->setRect(-6, -6, 12, 12);
        QPen pen(QColor(100, 150, 255), 2);
        pen.setCosmetic(true);
        hi->setPen(pen);
        hi->setBrush(QColor(255, 255, 255));
        hi->setZValue(11);
        scene->addItem(hi);
        cropHandles[d.h] = hi;
        handleForItem[hi] = d.h;
    }

    updateResizeHandles();
}

void MainWindow::updateResizeHandles()
{
    if (!cropRectItem || cropHandles.isEmpty()) return;

    QRectF r = cropRectItem->rect();

    QTransform t = view->transform();
    qreal scale = qMax(qAbs(t.m11()), qAbs(t.m22()));
    if (scale < 0.05) scale = 1.0;
    const qreal screenSize = 12.0;
    qreal size = screenSize / scale;
    const qreal hs = size / 2.0;

    for (auto it = cropHandles.begin(); it != cropHandles.end(); ++it) {
        Handle h = it.key();
        QGraphicsEllipseItem* hi = it.value();
        if (!hi) continue;
        QPointF p;
        switch (h) {
            case TopLeftHandle: p = r.topLeft(); break;
            case TopHandle: p = QPointF(r.center().x(), r.top()); break;
            case TopRightHandle: p = r.topRight(); break;
            case LeftHandle: p = QPointF(r.left(), r.center().y()); break;
            case RightHandle: p = QPointF(r.right(), r.center().y()); break;
            case BottomLeftHandle: p = r.bottomLeft(); break;
            case BottomHandle: p = QPointF(r.center().x(), r.bottom()); break;
            case BottomRightHandle: p = r.bottomRight(); break;
            default: continue;
        }
        hi->setRect(-hs, -hs, size, size);
        hi->setPos(p);
    }
}

void MainWindow::removeResizeHandles()
{
    for (QGraphicsItem* hi : cropHandles) {
        if (hi) {
            scene->removeItem(hi);
            delete hi;
        }
    }
    cropHandles.clear();
    handleForItem.clear();
    activeHandle = NoHandle;
    isMovingRect = false;
}

void MainWindow::setCursorForHandle(Handle h)
{
    Qt::CursorShape shape = Qt::ArrowCursor;
    switch (h) {
        case TopLeftHandle:
        case BottomRightHandle: shape = Qt::SizeFDiagCursor; break;
        case TopRightHandle:
        case BottomLeftHandle: shape = Qt::SizeBDiagCursor; break;
        case TopHandle:
        case BottomHandle: shape = Qt::SizeVerCursor; break;
        case LeftHandle:
        case RightHandle: shape = Qt::SizeHorCursor; break;
        default: break;
    }
    view->viewport()->setCursor(shape);
}

QRectF MainWindow::computeResizedRect(const QRectF& orig, Handle h, const QPointF& mouseScene) const
{
    if (h == NoHandle) return orig;

    qreal l = orig.left();
    qreal t = orig.top();
    qreal r = orig.right();
    qreal b = orig.bottom();

    const qreal ar = cropAspectRatio;

    if (ar > 0.0) {
        switch (h) {
            case TopLeftHandle: {
                qreal fx = orig.right();
                qreal fy = orig.bottom();
                qreal dx = fx - mouseScene.x();
                qreal dy = fy - mouseScene.y();
                if (dx > 0 && dy > 0) {
                    if (dx / dy > ar) dx = dy * ar;
                    else              dy = dx / ar;
                    l = fx - dx;
                    t = fy - dy;
                    r = fx;
                    b = fy;
                }
                break;
            }
            case TopRightHandle: {
                qreal fx = orig.left();
                qreal fy = orig.bottom();
                qreal dx = mouseScene.x() - fx;
                qreal dy = fy - mouseScene.y();
                if (dx > 0 && dy > 0) {
                    if (dx / dy > ar) dx = dy * ar;
                    else              dy = dx / ar;
                    l = fx;
                    t = fy - dy;
                    r = fx + dx;
                    b = fy;
                }
                break;
            }
            case BottomLeftHandle: {
                qreal fx = orig.right();
                qreal fy = orig.top();
                qreal dx = fx - mouseScene.x();
                qreal dy = mouseScene.y() - fy;
                if (dx > 0 && dy > 0) {
                    if (dx / dy > ar) dx = dy * ar;
                    else              dy = dx / ar;
                    l = fx - dx;
                    t = fy;
                    r = fx;
                    b = fy + dy;
                }
                break;
            }
            case BottomRightHandle: {
                qreal fx = orig.left();
                qreal fy = orig.top();
                qreal dx = mouseScene.x() - fx;
                qreal dy = mouseScene.y() - fy;
                if (dx > 0 && dy > 0) {
                    if (dx / dy > ar) dx = dy * ar;
                    else              dy = dx / ar;
                    l = fx;
                    t = fy;
                    r = fx + dx;
                    b = fy + dy;
                }
                break;
            }

            case TopHandle: {
                qreal fixedY = orig.bottom();
                qreal newH = fixedY - mouseScene.y();
                if (newH > 0) {
                    qreal newW = newH * ar;
                    qreal cx = orig.center().x();
                    l = cx - newW / 2.0;
                    t = mouseScene.y();
                    r = cx + newW / 2.0;
                    b = fixedY;
                }
                break;
            }
            case BottomHandle: {
                qreal fixedY = orig.top();
                qreal newH = mouseScene.y() - fixedY;
                if (newH > 0) {
                    qreal newW = newH * ar;
                    qreal cx = orig.center().x();
                    l = cx - newW / 2.0;
                    t = fixedY;
                    r = cx + newW / 2.0;
                    b = mouseScene.y();
                }
                break;
            }
            case LeftHandle: {
                qreal fixedX = orig.right();
                qreal newW = fixedX - mouseScene.x();
                if (newW > 0) {
                    qreal newH = newW / ar;
                    qreal cy = orig.center().y();
                    l = mouseScene.x();
                    t = cy - newH / 2.0;
                    r = fixedX;
                    b = cy + newH / 2.0;
                }
                break;
            }
            case RightHandle: {
                qreal fixedX = orig.left();
                qreal newW = mouseScene.x() - fixedX;
                if (newW > 0) {
                    qreal newH = newW / ar;
                    qreal cy = orig.center().y();
                    l = fixedX;
                    t = cy - newH / 2.0;
                    r = mouseScene.x();
                    b = cy + newH / 2.0;
                }
                break;
            }
            default: break;
        }
    } else {
        switch (h) {
            case TopLeftHandle: l = mouseScene.x(); t = mouseScene.y(); break;
            case TopHandle: t = mouseScene.y(); break;
            case TopRightHandle: r = mouseScene.x(); t = mouseScene.y(); break;
            case LeftHandle: l = mouseScene.x(); break;
            case RightHandle: r = mouseScene.x(); break;
            case BottomLeftHandle: l = mouseScene.x(); b = mouseScene.y(); break;
            case BottomHandle: b = mouseScene.y(); break;
            case BottomRightHandle: r = mouseScene.x(); b = mouseScene.y(); break;
            default: break;
        }
    }

    const qreal minSz = 5.0;
    if (h == TopLeftHandle || h == LeftHandle || h == BottomLeftHandle) {
        if (r - l < minSz) l = r - minSz;
    } else if (h == TopRightHandle || h == RightHandle || h == BottomRightHandle) {
        if (r - l < minSz) r = l + minSz;
    }
    if (h == TopLeftHandle || h == TopHandle || h == TopRightHandle) {
        if (b - t < minSz) t = b - minSz;
    } else if (h == BottomLeftHandle || h == BottomHandle || h == BottomRightHandle) {
        if (b - t < minSz) b = t + minSz;
    }

    return QRectF(l, t, r - l, b - t);
}

QRectF MainWindow::computeFittedCropRect(QGraphicsPixmapItem* item, qreal aspect) const
{
    if (!item) return {};

    QRectF bounds = item->sceneBoundingRect();
    if (!bounds.isValid() || bounds.width() < 10 || bounds.height() < 10)
        return bounds;

    if (aspect <= 0.0) {
        qreal mx = bounds.width() * 0.12;
        qreal my = bounds.height() * 0.12;
        return bounds.adjusted(mx, my, -mx, -my);
    }

    qreal bw = bounds.width();
    qreal bh = bounds.height();

    qreal w, h;
    if (bw / bh > aspect) {
        h = bh;
        w = h * aspect;
    } else {
        w = bw;
        h = w / aspect;
    }

    qreal shrink = 0.90;
    w *= shrink;
    h *= shrink;

    qreal cx = bounds.center().x();
    qreal cy = bounds.center().y();

    return QRectF(cx - w / 2.0, cy - h / 2.0, w, h);
}

QRect MainWindow::computeConstrainedRubberRect(const QPoint& start, const QPoint& current, qreal aspect) const
{
    if (aspect <= 0.0)
        return QRect(start, current);

    int dx = current.x() - start.x();
    int dy = current.y() - start.y();

    int sdx = (dx >= 0) ? 1 : -1;
    int sdy = (dy >= 0) ? 1 : -1;

    dx = qAbs(dx);
    dy = qAbs(dy);

    int newW = dx;
    int newH = dy;

    if (dx > 0 || dy > 0) {
        if (dx > dy * aspect) {
            newH = dy;
            newW = qRound(dy * aspect);
        } else {
            newW = dx;
            newH = qRound(dx / aspect);
        }
    }

    return QRect(start, QPoint(start.x() + newW * sdx, start.y() + newH * sdy));
}

void MainWindow::startCropWithRatio(qreal aspect)
{
    if (isAnimated) return;

    cropAspectRatio = aspect;

    QGraphicsPixmapItem* item = pixmapItem();
    if (!item) return;

    clearCropSelection();
    if (rubberBand) {
        rubberBand->hide();
        delete rubberBand;
        rubberBand = nullptr;
    }
    cropping = false;

    QRectF sceneRect = computeFittedCropRect(item, aspect);
    if (!sceneRect.isValid() || sceneRect.width() < 5 || sceneRect.height() < 5)
        return;

    cropRectItem = new QGraphicsRectItem();
    cropRectItem->setRect(sceneRect.normalized());

    QPen solidPen(QColor(100, 150, 255), 1.0);
    solidPen.setStyle(Qt::SolidLine);
    solidPen.setCosmetic(true);
    cropRectItem->setPen(solidPen);
    cropRectItem->setBrush(QBrush(QColor(100, 150, 255, 40)));
    cropRectItem->setZValue(10);
    scene->addItem(cropRectItem);

    createResizeHandles();
    showCropPanel();
}

void MainWindow::setBackgroundColor(Background bg)
{
    currentBackground = bg;

    QColor col;
    if (bg == Background::Black) {
        col = Qt::black;
    } else if (bg == Background::White) {
        col = Qt::white;
    } else {
        col = QColor(30, 30, 30);
    }

    view->setBackgroundBrush(QBrush(col));
    setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(col.name()));

    update();
    if (view) {
        view->update();
        if (view->viewport()) {
            view->viewport()->update();
        }
    }
}

void MainWindow::clearCropSelection()
{
    activeHandle = NoHandle;
    isMovingRect = false;
    if (cropRectItem) {
        scene->removeItem(cropRectItem);
        delete cropRectItem;
        cropRectItem = nullptr;
    }
    removeResizeHandles();
    hideCropPanel();
}

void MainWindow::createCropPanel()
{
    if (cropPanel) return;

    cropPanel = new QWidget(view);
    cropPanel->setContentsMargins(0, 0, 0, 0);

    auto *layout = new QHBoxLayout(cropPanel);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(3);

    cancelButton = new QPushButton("✕", cropPanel);
    cancelButton->setObjectName("cropCancelBtn");
    cancelButton->setFixedSize(20, 18);
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setToolTip("Cancel");

    cropButton = new QPushButton("Crop", cropPanel);
    cropButton->setObjectName("cropApplyBtn");
    cropButton->setFixedSize(42, 18);
    cropButton->setCursor(Qt::PointingHandCursor);
    cropButton->setToolTip("Crop image to selection");

    layout->addWidget(cancelButton);
    layout->addWidget(cropButton);

    cropPanel->setStyleSheet(
        "QWidget { background-color: #2a2a2a; border: 1px solid #555; border-radius: 3px; }"
        "QPushButton { color: #ddd; background: transparent; border: none; font-size: 11px; padding: 0px; }"
        "QPushButton:hover { background-color: #3a3a3a; }"
        "QPushButton#cropCancelBtn:hover { color: #ff8888; background-color: #4a2a2a; }"
        "QPushButton#cropApplyBtn:hover { color: #88ccff; background-color: #2a3a5a; }"
    );

    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::onCropPanelCancel);
    connect(cropButton, &QPushButton::clicked, this, &MainWindow::applyCrop);

    cropPanel->hide();
}

void MainWindow::showCropPanel()
{
    if (!cropRectItem) return;
    if (!cropPanel) {
        createCropPanel();
    }
    cropPanel->show();
    updateCropPanelPosition();
}

void MainWindow::hideCropPanel()
{
    if (cropPanel) {
        cropPanel->hide();
    }
}

void MainWindow::updateCropPanelPosition()
{
    if (!cropRectItem || !cropPanel || !cropPanel->isVisible()) return;

    QRectF r = cropRectItem->rect();
    QPointF bottomCenterScene(r.center().x(), r.bottom());

    QPoint bottomCenterView = view->mapFromScene(bottomCenterScene);

    cropPanel->adjustSize();
    QSize ps = cropPanel->sizeHint();
    if (ps.width() <= 0) ps = QSize(70, 22);

    const int inset = 6;
    int x = bottomCenterView.x() - ps.width() / 2;
    int y = bottomCenterView.y() - ps.height() - inset;

    QPoint rectLeftView = view->mapFromScene(QPointF(r.left(), r.bottom()));
    QPoint rectRightView = view->mapFromScene(QPointF(r.right(), r.bottom()));
    int minX = rectLeftView.x() + 2;
    int maxX = rectRightView.x() - ps.width() - 2;
    if (minX > maxX) {
        x = rectLeftView.x() + (rectRightView.x() - rectLeftView.x() - ps.width()) / 2;
    } else {
        x = qBound(minX, x, maxX);
    }

    QPoint rectTopView = view->mapFromScene(QPointF(r.left(), r.top()));
    y = qMax(y, rectTopView.y() + 2);

    x = qBound(0, x, view->width() - ps.width());
    y = qBound(0, y, view->height() - ps.height());

    cropPanel->move(x, y);
    cropPanel->raise();
}

void MainWindow::onCropPanelCancel()
{
    clearCropSelection();
    cropping = false;
    view->viewport()->setCursor(Qt::ArrowCursor);
}

void MainWindow::showContextMenu(const QPoint &globalPos)
{
    QMenu menu(this);
    updateNavigationState();

    bool hasImage = pixmapItem() != nullptr;

    menu.addAction(openAction);

    if (isAnimated && currentMovie) {
        bool running = (currentMovie->state() == QMovie::Running);
        animationToggleAction->setText(running ? "Pause Animation" : "Play Animation");
        animationToggleAction->setIcon(QIcon::fromTheme(running ? "media-playback-pause" : "media-playback-start"));
        menu.addAction(animationToggleAction);
    }

    if (isCurrentlyCropped) {
        menu.addAction(resetAction);
    }

    menu.addSeparator();

    if (hasImage && folderImages.size() > 1 && currentImageIndex >= 0) {
        menu.addAction(prevAction);
        menu.addAction(nextAction);
    }

    menu.addSeparator();

    menu.addAction(fitAction);
    fullscreenAction->setChecked((windowState() & Qt::WindowFullScreen) != 0);
    menu.addAction(fullscreenAction);
    menu.addAction(autoResizeAction);
    menu.addAction(zoomInAction);
    menu.addAction(zoomOutAction);

    menu.addSeparator();

    if (hasImage && !isAnimated) {
        QMenu* cropMenu = menu.addMenu("Crop");
        cropMenu->setIcon(QIcon::fromTheme("edit-cut"));
        struct AspectPreset {
            QString label;
            qreal ratio;
        };
        const QList<AspectPreset> presets = {
            {"Free",          0.0},
            {"1:1 Square",    1.0},
            {"3:2",           3.0/2.0},
            {"2:3",           2.0/3.0},
            {"4:3",           4.0/3.0},
            {"3:4",           3.0/4.0},
            {"16:9",          16.0/9.0},
            {"9:16",          9.0/16.0},
        };

        for (const AspectPreset& p : presets) {
            QAction* act = cropMenu->addAction(p.label);
            act->setCheckable(true);
            act->setChecked(qFuzzyCompare(cropAspectRatio, p.ratio) ||
                            (cropAspectRatio == 0.0 && p.ratio == 0.0));
            connect(act, &QAction::triggered, this, [this, ratio = p.ratio]() {
                startCropWithRatio(ratio);
            });
        }
    }

    if (hasImage && !isAnimated) {
        QMenu* transformMenu = menu.addMenu("Transform");
        transformMenu->setIcon(QIcon::fromTheme("object-rotate-left"));
        transformMenu->addAction(rotateLeftAction);
        transformMenu->addAction(rotateRightAction);
        transformMenu->addSeparator();
        transformMenu->addAction(mirrorAction);
        transformMenu->addAction(flipAction);
        transformMenu->addSeparator();
        transformMenu->addAction(resizeAction);
    }

    QMenu* bgMenu = menu.addMenu("Background Color");
    bgMenu->setIcon(QIcon::fromTheme("preferences-desktop-theme"));
    struct BgPreset {
        QString label;
        Background bg;
    };
    const QList<BgPreset> bgPresets = {
        {"Grey",   Background::Grey},
        {"Black",  Background::Black},
        {"White",  Background::White},
    };

    for (const BgPreset& p : bgPresets) {
        QAction* act = bgMenu->addAction(p.label);
        act->setCheckable(true);
        act->setChecked(currentBackground == p.bg);
        connect(act, &QAction::triggered, this, [this, b = p.bg]() {
            setBackgroundColor(b);
        });
    }

    menu.addSeparator();

    menu.addAction(saveAction);
    menu.addAction(saveAsAction);
    menu.addAction(copyAction);
    menu.addAction(copyToAction);
    menu.addAction(moveToAction);
    menu.addAction(renameAction);
    menu.addAction(deleteAction);

    menu.addSeparator();
    menu.addAction(aboutAction);
    if (hasImage) {
        menu.addAction(propertiesAction);
    }

    menu.exec(globalPos);
}

void MainWindow::applyCrop()
{
    if (!cropRectItem) return;
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) {
        scene->removeItem(cropRectItem);
        delete cropRectItem;
        cropRectItem = nullptr;
        removeResizeHandles();
        hideCropPanel();
        return;
    }

    if (currentMovie) {
        currentMovie->stop();
    }
    cleanupMovie();

    QPixmap orig = item->pixmap();
    QRectF selScene = cropRectItem->rect().normalized();
    QRectF selLocal = item->mapRectFromScene(selScene);
    QRect cropRect = selLocal.toRect().intersected(QRect(0, 0, orig.width(), orig.height()));
    if (cropRect.width() < 1 || cropRect.height() < 1) {
        scene->removeItem(cropRectItem);
        delete cropRectItem;
        cropRectItem = nullptr;
        removeResizeHandles();
        hideCropPanel();
        return;
    }

    QPixmap cropped = orig.copy(cropRect);
    if (cropped.isNull()) return;

    scene->removeItem(item);
    delete item;

    scene->removeItem(cropRectItem);
    delete cropRectItem;
    cropRectItem = nullptr;
    removeResizeHandles();
    hideCropPanel();

    QGraphicsPixmapItem *newItem = new QGraphicsPixmapItem(cropped);
    newItem->setTransformationMode(Qt::SmoothTransformation);
    newItem->setPos(selScene.topLeft());
    scene->addItem(newItem);

    updateSceneRectAroundItem(newItem);

    isCurrentlyCropped = true;
}

void MainWindow::resetToOriginal()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    cleanupMovie();

    if (originalWasAnimated && !originalSourcePath.isEmpty()) {
        QTransform savedTransform = view->transform();
        QPointF savedCenterScene = view->mapToScene(view->viewport()->rect().center());

        clearCropSelection();
        if (rubberBand) {
            rubberBand->hide();
            delete rubberBand;
            rubberBand = nullptr;
        }
        cropping = false;
        activeHandle = NoHandle;
        isMovingRect = false;

        scene->removeItem(item);
        delete item;

        currentMovie = new QMovie(originalSourcePath, QByteArray(), this);
        if (currentMovie->isValid()) {
            isAnimated = true;
            currentMovie->jumpToFrame(0);
            QPixmap pm = currentMovie->currentPixmap();

            QGraphicsPixmapItem *newItem = new QGraphicsPixmapItem(pm);
            newItem->setTransformationMode(Qt::SmoothTransformation);
            const qreal w = pm.width();
            const qreal h = pm.height();
            newItem->setPos(-w / 2.0, -h / 2.0);
            scene->addItem(newItem);

            updateSceneRectAroundItem(newItem);

            view->setTransform(savedTransform);
            view->centerOn(savedCenterScene);

            connect(currentMovie, &QMovie::frameChanged, this, &MainWindow::onMovieFrameChanged);
            currentMovie->start();

            currentFilePath = originalSourcePath;
            setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(currentFilePath).fileName()));
            updateFolderImages(currentFilePath);
        } else {
            delete currentMovie;
            currentMovie = nullptr;
            isAnimated = false;
        }

        isCurrentlyCropped = false;
        return;
    }

    if (originalPixmap.isNull()) return;

    if (!isCurrentlyCropped && item->pixmap().size() == originalPixmap.size()) {
        fitImageToWindow();
        return;
    }

    QTransform savedTransform = view->transform();
    QPointF savedCenterScene = view->mapToScene(view->viewport()->rect().center());

    clearCropSelection();
    if (rubberBand) {
        rubberBand->hide();
        delete rubberBand;
        rubberBand = nullptr;
    }
    cropping = false;
    activeHandle = NoHandle;
    isMovingRect = false;

    scene->removeItem(item);
    delete item;

    QGraphicsPixmapItem *newItem = new QGraphicsPixmapItem(originalPixmap);
    newItem->setTransformationMode(Qt::SmoothTransformation);
    const qreal w = originalPixmap.width();
    const qreal h = originalPixmap.height();
    newItem->setPos(-w / 2.0, -h / 2.0);
    scene->addItem(newItem);

    updateSceneRectAroundItem(newItem);

    view->setTransform(savedTransform);
    view->centerOn(savedCenterScene);

    isCurrentlyCropped = false;

    updateNavigationState();
}

void MainWindow::saveImage()
{
    if (currentFilePath.isEmpty()) {
        saveImageAs();
        return;
    }

    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QPixmap toSave = item->pixmap();
    if (!toSave.save(currentFilePath)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not save the image to:\n" + currentFilePath);
        return;
    }

    cleanupMovie();
    originalWasAnimated = false;
    originalSourcePath.clear();
    originalPixmap = toSave;
    isCurrentlyCropped = false;

    updateFolderImages(currentFilePath);
}

void MainWindow::saveImageAs()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QString suggested = currentFilePath;
    if (suggested.isEmpty()) {
        suggested = QDir::homePath() + "/untitled.png";
    } else {
        QFileInfo fi(suggested);
        QString suffix = isCurrentlyCropped ? "_cropped." : "_copy.";
        suggested = fi.absolutePath() + "/" + fi.baseName() + suffix + fi.suffix();
    }

    QString filter = "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tiff);;WebP (*.webp);;All Files (*)";
    QString filePath = QFileDialog::getSaveFileName(this, "Save Image As", suggested, filter);
    if (filePath.isEmpty()) return;

    QPixmap toSave = item->pixmap();
    if (!toSave.save(filePath)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not save the image to:\n" + filePath);
        return;
    }

    cleanupMovie();
    currentFilePath = filePath;
    originalWasAnimated = false;
    originalSourcePath.clear();
    originalPixmap = toSave;
    isCurrentlyCropped = false;

    setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(filePath).fileName()));

    updateFolderImages(currentFilePath);
}

void MainWindow::copyImage()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) {
        qDebug() << "copyImage: no pixmapItem found in scene";
        return;
    }
    QPixmap pm = item->pixmap();
    if (pm.isNull()) {
        qDebug() << "copyImage: pixmap from item is null";
        return;
    }

    qDebug() << "copyImage: copying pixmap of size" << pm.size() << "to clipboard";

    QClipboard *cb = QApplication::clipboard();
    if (!cb) {
        qDebug() << "copyImage: no clipboard available";
        return;
    }

    cb->setPixmap(pm);
    cb->setImage(pm.toImage());

    QByteArray pngData;
    QBuffer buffer(&pngData);
    if (buffer.open(QIODevice::WriteOnly) && pm.save(&buffer, "PNG")) {
        buffer.close();

        auto *mimeData = new QMimeData();
        mimeData->setData("image/png", pngData);
        mimeData->setData("image/x-png", pngData);
        mimeData->setImageData(pm.toImage());

        cb->setMimeData(mimeData);
    }

    const QString oldTitle = windowTitle();
    setWindowTitle(QString("Copied %1×%2 to clipboard").arg(pm.width()).arg(pm.height()));
    QTimer::singleShot(1200, this, [this, oldTitle]() {
        if (windowTitle().startsWith("Copied ")) {
            setWindowTitle(oldTitle);
        }
    });

    qDebug() << "copyImage: setPixmap/setImage/setMimeData calls completed";
}

void MainWindow::copyImageTo()
{
    if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) return;

    QFileInfo fi(currentFilePath);
    QString startDir = fi.absolutePath();
    QString targetDir = QFileDialog::getExistingDirectory(this, "Copy to folder", startDir);
    if (targetDir.isEmpty()) return;

    QString dest = QDir(targetDir).filePath(fi.fileName());

    if (QFile::exists(dest)) {
        auto ans = QMessageBox::question(this, "Overwrite file?",
            QString("'%1' already exists in the target folder. Overwrite?").arg(fi.fileName()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
        if (ans == QMessageBox::Cancel) return;
        if (ans == QMessageBox::No) return;
        QFile::remove(dest);
    }

    if (!QFile::copy(currentFilePath, dest)) {
        QMessageBox::warning(this, "Copy failed",
            "Could not copy the file to:\n" + dest);
    }
}

void MainWindow::moveImageTo()
{
    if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) return;

    if (isCurrentlyCropped) {
        auto ans = QMessageBox::warning(this, "Unsaved changes",
            "The current image has unsaved crop or transforms.\n"
            "Moving will move the original file on disk; your in-memory changes will remain only until you close or reset.\n\n"
            "Continue anyway?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ans != QMessageBox::Yes) return;
    }

    QFileInfo fi(currentFilePath);
    QString startDir = fi.absolutePath();
    QString targetDir = QFileDialog::getExistingDirectory(this, "Move to folder", startDir);
    if (targetDir.isEmpty()) return;

    QString dest = QDir(targetDir).filePath(fi.fileName());

    if (QFile::exists(dest)) {
        auto ans = QMessageBox::question(this, "Overwrite file?",
            QString("'%1' already exists in the target folder. Overwrite?").arg(fi.fileName()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
        if (ans == QMessageBox::Cancel) return;
        if (ans == QMessageBox::No) return;
        QFile::remove(dest);
    }

    bool success = QFile::rename(currentFilePath, dest);
    if (!success) {
        if (QFile::copy(currentFilePath, dest)) {
            if (QFile::remove(currentFilePath)) {
                success = true;
            } else {
                QMessageBox::warning(this, "Move incomplete",
                    "Copied to destination but could not remove the original.\n"
                    "You may now have duplicates.");
                success = true;
            }
        }
    }

    if (success) {
        QString oldPath = currentFilePath;
        currentFilePath = dest;
        setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(dest).fileName()));

        if (originalSourcePath == oldPath) {
            originalSourcePath = dest;
        }

        updateFolderImages(dest);
    } else {
        QMessageBox::warning(this, "Move failed",
            "Could not move the file to:\n" + dest);
    }
}

void MainWindow::deleteImage()
{
    if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) return;

    QFileInfo fi(currentFilePath);
    QString msg = QString("Permanently delete \"%1\" from disk?").arg(fi.fileName());
    if (isCurrentlyCropped) {
        msg += "\n\n(Note: this file has unsaved crop/transform modifications in the viewer.)";
    }
    auto reply = QMessageBox::question(this, "Confirm Delete", msg,
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString deletedPath = currentFilePath;
    int deletedIdx = currentImageIndex;

    if (!QFile::remove(deletedPath)) {
        QMessageBox::warning(this, "Delete failed",
            "Could not delete the file:\n" + deletedPath);
        return;
    }

    prepareForNewImage();
    currentFilePath.clear();
    setWindowTitle("Simple Viewer");

    if (deletedIdx >= 0 && !folderImages.isEmpty()) {
        if (deletedIdx < folderImages.size() && folderImages[deletedIdx] == deletedPath) {
            folderImages.removeAt(deletedIdx);
        } else {
            folderImages.removeAll(deletedPath);
        }

        if (!folderImages.isEmpty()) {
            int newIdx = deletedIdx;
            if (newIdx >= folderImages.size()) newIdx = folderImages.size() - 1;
            if (newIdx < 0) newIdx = 0;
            loadImage(folderImages[newIdx]);
            return;
        }
    }

    updateNavigationState();
}

void MainWindow::renameImage()
{
    if (currentFilePath.isEmpty() || !QFile::exists(currentFilePath)) return;

    if (isCurrentlyCropped) {
        auto ans = QMessageBox::warning(this, "Unsaved changes",
            "The current image has unsaved crop or transforms.\n"
            "Renaming will rename the original file on disk; your in-memory changes will remain only until you close or reset.\n\n"
            "Continue anyway?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ans != QMessageBox::Yes) return;
    }

    QFileInfo fi(currentFilePath);
    QString currentName = fi.fileName();

    QString newName = QInputDialog::getText(this, "Rename", "New name:", QLineEdit::Normal, currentName);
    if (newName.isEmpty() || newName.trimmed() == currentName) return;

    newName = newName.trimmed();

    if (newName.contains('/') || newName.contains('\\')) {
        QMessageBox::warning(this, "Invalid name",
            "New name cannot contain folder separators.\nUse \"Move to...\" to move the file to another folder.");
        return;
    }

    QString newPath = fi.dir().filePath(newName);
    if (newPath == currentFilePath) return;

    if (QFile::exists(newPath)) {
        auto ans = QMessageBox::question(this, "Overwrite file?",
            QString("A file named \"%1\" already exists.\nOverwrite it?").arg(newName),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
        if (ans == QMessageBox::Cancel) return;
        if (ans == QMessageBox::No) return;
        if (!QFile::remove(newPath)) {
            QMessageBox::warning(this, "Rename failed",
                "Could not remove the existing file to make room.");
            return;
        }
    }

    if (!QFile::rename(currentFilePath, newPath)) {
        QMessageBox::warning(this, "Rename failed",
            "Could not rename the file to:\n" + newPath);
        return;
    }

    QString oldPath = currentFilePath;
    currentFilePath = newPath;
    setWindowTitle(QString("Simple Viewer - %1").arg(QFileInfo(newPath).fileName()));

    if (originalSourcePath == oldPath) {
        originalSourcePath = newPath;
    }

    updateFolderImages(newPath);
}

void MainWindow::showAbout()
{
    QDialog dlg(this);
    dlg.setWindowTitle("About Simple Viewer");
    dlg.resize(600, 450);

    auto *mainLayout = new QVBoxLayout(&dlg);

    auto *stack = new QStackedWidget(&dlg);

    auto *aboutPage = new QWidget();
    auto *aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(20, 20, 20, 20);
    aboutLayout->setSpacing(12);

    auto *title = new QLabel("Simple Viewer");
    title->setStyleSheet("font-size: 22px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);

    auto *version = new QLabel("v 1.0");
    version->setStyleSheet("font-size: 16px;");
    version->setAlignment(Qt::AlignCenter);

    auto *copyright = new QLabel("Copyright (C) 2026 Ethan McCall");
    copyright->setAlignment(Qt::AlignCenter);

    aboutLayout->addStretch();
    aboutLayout->addWidget(title);
    aboutLayout->addWidget(version);
    aboutLayout->addWidget(copyright);
    aboutLayout->addStretch();

    stack->addWidget(aboutPage);

    auto *licensePage = new QWidget();
    auto *licenseLayout = new QVBoxLayout(licensePage);
    licenseLayout->setContentsMargins(20, 20, 20, 20);

    auto *licTitle = new QLabel("GNU General Public License v3");
    licTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    licTitle->setAlignment(Qt::AlignCenter);

    auto *licenseBrowser = new QTextBrowser();
    licenseBrowser->setReadOnly(true);
    licenseBrowser->setFrameShape(QFrame::NoFrame);
    licenseBrowser->setStyleSheet(
        "QTextBrowser {"
        "  border-radius: 10px;"
        "  border: 1px solid #555;"
        "  background-color: #2c3e50;"
        "  padding: 8px;"
        "}"
    );

    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/LICENSE",
        QDir::currentPath() + "/LICENSE",
        "LICENSE"
    };
    QString licContent;
    bool loaded = false;
    for (const QString& path : searchPaths) {
        QFile licFile(path);
        if (licFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            licContent = QTextStream(&licFile).readAll();
            loaded = true;
            break;
        }
    }
    if (!loaded || licContent.isEmpty()) {
        licContent = "License file not found.\n\n"
                     "Please see the LICENSE file in the application directory.\n"
                     "The full text of the GNU General Public License v3 is available at:\n"
                     "https://www.gnu.org/licenses/gpl-3.0.txt";
    }
    licenseBrowser->setPlainText(licContent);

    licenseLayout->addWidget(licTitle);
    licenseLayout->addWidget(licenseBrowser, 1);

    stack->addWidget(licensePage);

    auto *changelogPage = new QWidget();
    auto *changelogLayout = new QVBoxLayout(changelogPage);
    changelogLayout->setContentsMargins(20, 20, 20, 20);

    auto *changelogBrowser = new QTextBrowser();
    changelogBrowser->setReadOnly(true);
    changelogBrowser->setFrameShape(QFrame::NoFrame);
    changelogBrowser->setStyleSheet(
        "QTextBrowser {"
        "  border-radius: 10px;"
        "  border: 1px solid #555;"
        "  background-color: #2c3e50;"
        "  padding: 8px;"
        "}"
    );

    QStringList changelogSearchPaths = {
        QCoreApplication::applicationDirPath() + "/changelog.html",
        QDir::currentPath() + "/changelog.html",
        "changelog.html"
    };
    QString changelogContent;
    bool changelogLoaded = false;
    for (const QString& path : changelogSearchPaths) {
        QFile chFile(path);
        if (chFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            changelogContent = QTextStream(&chFile).readAll();
            changelogLoaded = true;
            break;
        }
    }
    if (!changelogLoaded || changelogContent.isEmpty()) {
        changelogContent = "<p>changelog.html not found.</p>"
                           "<p>Please see the changelog.html file in the application directory.</p>";
    }
    changelogBrowser->setHtml(changelogContent);

    changelogLayout->addWidget(changelogBrowser, 1);

    stack->addWidget(changelogPage);

    mainLayout->addWidget(stack);

    auto *buttonLayout = new QHBoxLayout();
    auto *whatsNewBtn = new QPushButton("What's New");
    auto *githubBtn = new QPushButton("View on GitHub");
    auto *bugBtn = new QPushButton("Report a Bug");
    auto *coffeeBtn = new QPushButton("Buy Me a Coffee");
    auto *viewLicenseBtn = new QPushButton("View License");
    auto *backBtn = new QPushButton("Back");
    auto *closeBtn = new QPushButton("Close");

    backBtn->hide();

    buttonLayout->addWidget(whatsNewBtn);
    buttonLayout->addWidget(githubBtn);
    buttonLayout->addWidget(bugBtn);
    buttonLayout->addWidget(coffeeBtn);
    buttonLayout->addWidget(viewLicenseBtn);
    buttonLayout->addWidget(backBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    connect(whatsNewBtn, &QPushButton::clicked, this, [stack, whatsNewBtn, githubBtn, bugBtn, coffeeBtn, viewLicenseBtn, backBtn]() {
        stack->setCurrentIndex(2);
        whatsNewBtn->hide();
        githubBtn->hide();
        bugBtn->hide();
        coffeeBtn->hide();
        viewLicenseBtn->hide();
        backBtn->show();
    });

    connect(viewLicenseBtn, &QPushButton::clicked, this, [stack, whatsNewBtn, githubBtn, bugBtn, coffeeBtn, viewLicenseBtn, backBtn]() {
        stack->setCurrentIndex(1);
        whatsNewBtn->hide();
        githubBtn->hide();
        bugBtn->hide();
        coffeeBtn->hide();
        viewLicenseBtn->hide();
        backBtn->show();
    });

    connect(backBtn, &QPushButton::clicked, this, [stack, whatsNewBtn, githubBtn, bugBtn, coffeeBtn, viewLicenseBtn, backBtn]() {
        stack->setCurrentIndex(0);
        backBtn->hide();
        whatsNewBtn->show();
        githubBtn->show();
        bugBtn->show();
        coffeeBtn->show();
        viewLicenseBtn->show();
    });

    connect(coffeeBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://buymeacoffee.com/ethan_mccall"));
    });

    connect(githubBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/ethan-mccall/simple-viewer"));
    });

    connect(bugBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/ethan-mccall/simple-viewer/issues"));
    });

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

void MainWindow::showProperties()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QPixmap pm = item->pixmap();
    QStringList lines;

    if (!currentFilePath.isEmpty() && QFile::exists(currentFilePath)) {
        QFileInfo fi(currentFilePath);
        lines << "File: " + fi.fileName();
        lines << "Path: " + fi.absoluteFilePath();
        lines << "File size: " + QString::number(fi.size() / 1024.0, 'f', 1) + " KB";
        if (fi.lastModified().isValid()) {
            lines << "Modified: " + fi.lastModified().toString(Qt::ISODate);
        }
        lines << "";
    }

    lines << "Image size: " + QString::number(pm.width()) + " × " + QString::number(pm.height()) + " pixels";
    lines << "Depth: " + QString::number(pm.depth()) + " bits";

    if (isAnimated) {
        lines << "Animated: Yes (current frame shown)";
    }

    if (isCurrentlyCropped) {
        lines << "";
        lines << "(Note: displayed image is cropped or transformed; source file on disk may differ.)";
    }

    QMessageBox::information(this, "Image Properties", lines.join("\n"));
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) {
        event->accept();
        return;
    }

    QPoint localPos = view->viewport()->mapFromGlobal(event->globalPosition().toPoint());
    const QPointF scenePos = view->mapToScene(localPos);

    const qreal factor = (event->angleDelta().y() > 0) ? 1.15 : 0.85;

    performZoom(factor, scenePos);

    event->accept();
}

void MainWindow::zoomIn()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QPointF centerScene = view->mapToScene(view->viewport()->rect().center());
    performZoom(1.15, centerScene);
}

void MainWindow::zoomOut()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QPointF centerScene = view->mapToScene(view->viewport()->rect().center());
    performZoom(0.85, centerScene);
}

void MainWindow::performZoom(qreal factor, const QPointF &sceneAnchor)
{
    if (!pixmapItem()) return;

    QTransform transform = view->transform();
    transform.translate(sceneAnchor.x(), sceneAnchor.y());
    transform.scale(factor, factor);
    transform.translate(-sceneAnchor.x(), -sceneAnchor.y());

    view->setTransform(transform);

    updateSceneRectAroundItem(pixmapItem());

    if (cropRectItem && !cropHandles.isEmpty()) {
        updateResizeHandles();
    }

    if (cropPanel && cropPanel->isVisible()) {
        updateCropPanelPosition();
    }
}

void MainWindow::cleanupMovie()
{
    if (currentMovie) {
        currentMovie->stop();
        disconnect(currentMovie, &QMovie::frameChanged, this, &MainWindow::onMovieFrameChanged);
        delete currentMovie;
        currentMovie = nullptr;
    }
    isAnimated = false;
}

void MainWindow::prepareForNewImage()
{
    cleanupMovie();
    scene->clear();
    cropRectItem = nullptr;
    cropHandles.clear();
    handleForItem.clear();
    activeHandle = NoHandle;
    isMovingRect = false;
    cropping = false;
    hideCropPanel();
    if (rubberBand) {
        rubberBand->hide();
        delete rubberBand;
        rubberBand = nullptr;
    }
    pendingInitialFit = false;
    pendingAutoFit = false;
    m_exitingFullMode = false;
    m_lastFullViewSize = QSize();
    m_lastAutoResizeViewSize = QSize();
    welcomeItem = nullptr;

    updateNavigationState();
}

void MainWindow::onMovieFrameChanged(int)
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || !currentMovie) return;
    QPixmap pm = currentMovie->currentPixmap();
    if (pm.isNull()) return;
    item->setPixmap(pm);
}

void MainWindow::toggleAnimation()
{
    if (!currentMovie) return;
    if (currentMovie->state() == QMovie::Running) {
        currentMovie->setPaused(true);
    } else {
        currentMovie->setPaused(false);
    }

    bool isRunning = (currentMovie->state() == QMovie::Running);
    animationToggleAction->setText(isRunning ? "Pause Animation" : "Play Animation");
    animationToggleAction->setIcon(QIcon::fromTheme(isRunning ? "media-playback-pause" : "media-playback-start"));
}

void MainWindow::updateFolderImages(const QString &filePath)
{
    folderImages.clear();
    currentImageIndex = -1;

    if (filePath.isEmpty()) {
        updateNavigationState();
        return;
    }

    QFileInfo fi(filePath);
    if (!fi.exists()) {
        updateNavigationState();
        return;
    }

    QDir dir(fi.absolutePath());

    const QStringList exts = {"jpg", "jpeg", "png", "bmp", "gif", "webp", "tiff", "tif", "tga", "svg", "ico", "avif", "icns", "heic", "heif"};
    QStringList nameFilters;
    for (const QString &e : exts) {
        nameFilters << "*." + e << "*." + e.toUpper();
    }

    QStringList entries = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
    for (const QString &name : entries) {
        folderImages << dir.absoluteFilePath(name);
    }

    currentImageIndex = folderImages.indexOf(fi.absoluteFilePath());
    updateNavigationState();
}

void MainWindow::updateNavigationState()
{
    const bool canNavigate = (folderImages.size() > 1 && currentImageIndex >= 0);

    if (prevImageButton) {
        prevImageButton->setEnabled(canNavigate);
        prevImageButton->setVisible(canNavigate);
    }
    if (nextImageButton) {
        nextImageButton->setEnabled(canNavigate);
        nextImageButton->setVisible(canNavigate);
    }
    if (prevAction) prevAction->setEnabled(canNavigate);
    if (nextAction) nextAction->setEnabled(canNavigate);

    QGraphicsPixmapItem *p = pixmapItem();
    bool hasImg = p != nullptr;
    bool hasF = !currentFilePath.isEmpty() && QFile::exists(currentFilePath);

    if (fitAction) fitAction->setEnabled(hasImg);
    if (zoomInAction) zoomInAction->setEnabled(hasImg);
    if (zoomOutAction) zoomOutAction->setEnabled(hasImg);
    if (saveAction) saveAction->setEnabled(hasImg);
    if (saveAsAction) saveAsAction->setEnabled(hasImg);
    if (copyAction) copyAction->setEnabled(hasImg);
    if (copyToAction) copyToAction->setEnabled(hasF);
    if (moveToAction) moveToAction->setEnabled(hasF);
    if (deleteAction) deleteAction->setEnabled(hasF);
    if (renameAction) renameAction->setEnabled(hasF);
    if (propertiesAction) propertiesAction->setEnabled(hasImg);
    if (aboutAction) aboutAction->setEnabled(true);
    if (animationToggleAction) animationToggleAction->setEnabled(isAnimated && currentMovie != nullptr);

    positionFloatingButtons();
}

void MainWindow::positionFloatingButtons()
{
    if (!view) return;

    const int sz = 28;
    const int margin = 5;
    const int gap = 1;
    const int gapBeforeFs = 4;

    int yTop = margin;

    if (fullscreenButton) {
        int xFs = view->width() - margin - sz;
        fullscreenButton->setGeometry(xFs, yTop, sz, sz);
        fullscreenButton->raise();
    }

    if (prevImageButton && nextImageButton) {
        int xFs = view->width() - margin - sz;
        int xNext = xFs - gapBeforeFs - sz;
        nextImageButton->setGeometry(xNext, yTop, sz, sz);
        prevImageButton->setGeometry(xNext - gap - sz, yTop, sz, sz);
        nextImageButton->raise();
        prevImageButton->raise();
    }
}

void MainWindow::toggleFullScreen()
{
    if (windowState() & Qt::WindowFullScreen) {
        showNormal();
    } else {
        showFullScreen();
    }
    if (fullscreenAction) {
        fullscreenAction->setChecked((windowState() & Qt::WindowFullScreen) != 0);
    }
    positionFloatingButtons();
}

void MainWindow::goToNextImage()
{
    if (currentImageIndex < 0 || folderImages.size() <= 1) return;
    int next = currentImageIndex + 1;
    if (next >= folderImages.size()) next = 0;
    loadImage(folderImages[next]);
}

void MainWindow::goToPreviousImage()
{
    if (currentImageIndex < 0 || folderImages.size() <= 1) return;
    int prev = currentImageIndex - 1;
    if (prev < 0) prev = folderImages.size() - 1;
    loadImage(folderImages[prev]);
}

void MainWindow::replacePixmapItem(const QPixmap &newPix)
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item) return;

    QTransform savedTransform = view->transform();
    QPointF savedCenterScene = view->mapToScene(view->viewport()->rect().center());

    clearCropSelection();
    if (rubberBand) {
        rubberBand->hide();
        delete rubberBand;
        rubberBand = nullptr;
    }
    cropping = false;
    activeHandle = NoHandle;
    isMovingRect = false;

    scene->removeItem(item);
    delete item;

    QGraphicsPixmapItem *newItem = new QGraphicsPixmapItem(newPix);
    newItem->setTransformationMode(Qt::SmoothTransformation);
    const qreal w = newPix.width();
    const qreal h = newPix.height();
    newItem->setPos(-w / 2.0, -h / 2.0);
    scene->addItem(newItem);

    updateSceneRectAroundItem(newItem);

    view->setTransform(savedTransform);
    view->centerOn(savedCenterScene);

    isCurrentlyCropped = true;
}

void MainWindow::rotateLeft()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || isAnimated) return;
    QPixmap pm = item->pixmap();
    if (pm.isNull()) return;

    QTransform t;
    t.rotate(-90);
    QPixmap result = pm.transformed(t, Qt::SmoothTransformation);
    replacePixmapItem(result);
}

void MainWindow::rotateRight()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || isAnimated) return;
    QPixmap pm = item->pixmap();
    if (pm.isNull()) return;

    QTransform t;
    t.rotate(90);
    QPixmap result = pm.transformed(t, Qt::SmoothTransformation);
    replacePixmapItem(result);
}

void MainWindow::mirrorImage()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || isAnimated) return;
    QPixmap pm = item->pixmap();
    if (pm.isNull()) return;

    QImage img = pm.toImage();
    QPixmap result = QPixmap::fromImage(img.flipped(Qt::Horizontal));
    replacePixmapItem(result);
}

void MainWindow::flipImage()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || isAnimated) return;
    QPixmap pm = item->pixmap();
    if (pm.isNull()) return;

    QImage img = pm.toImage();
    QPixmap result = QPixmap::fromImage(img.flipped(Qt::Vertical));
    replacePixmapItem(result);
}

void MainWindow::resizeImage()
{
    QGraphicsPixmapItem *item = pixmapItem();
    if (!item || isAnimated) return;
    QPixmap pm = item->pixmap();
    if (pm.isNull()) return;

    const int origW = pm.width();
    const int origH = pm.height();
    const double aspect = static_cast<double>(origW) / qMax(1, origH);

    QDialog dlg(this);
    dlg.setWindowTitle("Resize Image");
    dlg.setMinimumWidth(280);

    auto *lay = new QVBoxLayout(&dlg);

    auto *wLay = new QHBoxLayout();
    wLay->addWidget(new QLabel("Width:"));
    auto *wBox = new QSpinBox(&dlg);
    wBox->setRange(1, 100000);
    wBox->setValue(origW);
    wBox->setSuffix(" px");
    wLay->addWidget(wBox);
    lay->addLayout(wLay);

    auto *hLay = new QHBoxLayout();
    hLay->addWidget(new QLabel("Height:"));
    auto *hBox = new QSpinBox(&dlg);
    hBox->setRange(1, 100000);
    hBox->setValue(origH);
    hBox->setSuffix(" px");
    hLay->addWidget(hBox);
    lay->addLayout(hLay);

    auto *keepAspect = new QCheckBox("Maintain aspect ratio", &dlg);
    keepAspect->setChecked(true);
    lay->addWidget(keepAspect);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    lay->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    connect(wBox, QOverload<int>::of(&QSpinBox::valueChanged), [hBox, aspect, keepAspect](int newW) {
        if (keepAspect->isChecked()) {
            int newH = qMax(1, qRound(newW / aspect));
            hBox->blockSignals(true);
            hBox->setValue(newH);
            hBox->blockSignals(false);
        }
    });
    connect(hBox, QOverload<int>::of(&QSpinBox::valueChanged), [wBox, aspect, keepAspect](int newH) {
        if (keepAspect->isChecked()) {
            int newW = qMax(1, qRound(newH * aspect));
            wBox->blockSignals(true);
            wBox->setValue(newW);
            wBox->blockSignals(false);
        }
    });
    connect(keepAspect, &QCheckBox::toggled, [wBox, hBox, aspect](bool checked) {
        if (checked) {
            int newH = qMax(1, qRound(wBox->value() / aspect));
            hBox->blockSignals(true);
            hBox->setValue(newH);
            hBox->blockSignals(false);
        }
    });

    if (dlg.exec() != QDialog::Accepted) return;

    int newW = wBox->value();
    int newH = hBox->value();
    if (newW == origW && newH == origH) return;

    QPixmap result = pm.scaled(newW, newH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    replacePixmapItem(result);
}

MainWindow::~MainWindow() {
    cleanupMovie();
    if (rubberBand) {
        rubberBand->hide();
        delete rubberBand;
        rubberBand = nullptr;
    }
    cropRectItem = nullptr;
    cropHandles.clear();
    handleForItem.clear();

    if (cropPanel) {
        cropPanel->hide();
    }
}
