/*
 * SimpleViewer
 * Copyright (C) 2026 Ethan McCall
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPoint>
#include <QPointF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QRubberBand>
#include <QMap>
#include <QHash>
#include <QRectF>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMovie>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &fileToOpen = QString(), QWidget *parent = nullptr);
    ~MainWindow();

    void loadImage(const QString &filePath);

private slots:
    void openImage();
    void fitImageToWindow();
    void zoomIn();
    void zoomOut();
    void applyCrop();
    void resetToOriginal();
    void saveImage();
    void saveImageAs();
    void copyImage();
    void copyImageTo();
    void moveImageTo();
    void deleteImage();
    void renameImage();
    void showAbout();
    void showProperties();
    void onMovieFrameChanged(int frameNumber);

    QString loadWelcomeHtml() const;
    void toggleAnimation();
    void goToNextImage();
    void goToPreviousImage();
    void rotateLeft();
    void rotateRight();
    void mirrorImage();
    void flipImage();
    void resizeImage();
    void toggleFullScreen();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsTextItem *welcomeItem = nullptr;
    QString currentFilePath;

    QPixmap originalPixmap;
    bool isCurrentlyCropped = false;

    QMovie *currentMovie = nullptr;
    bool isAnimated = false;

    bool originalWasAnimated = false;
    QString originalSourcePath;

    bool dragging = false;
    QPointF lastMousePos;

    bool pendingInitialFit = false;

    bool pendingAutoFit = false;

    bool m_exitingFullMode = false;

    QSize m_lastFullViewSize;

    QSize m_lastAutoResizeViewSize;

    QGraphicsRectItem *cropRectItem = nullptr;
    QRubberBand *rubberBand = nullptr;
    QPoint cropRubberStart;
    bool cropping = false;

    enum Handle {
        NoHandle,
        TopLeftHandle, TopHandle, TopRightHandle,
        LeftHandle, RightHandle,
        BottomLeftHandle, BottomHandle, BottomRightHandle
    };
    QMap<Handle, QGraphicsEllipseItem*> cropHandles;
    QHash<QGraphicsItem*, Handle> handleForItem;
    Handle activeHandle = NoHandle;
    bool isMovingRect = false;
    QRectF originalRect;
    QPointF dragStartPos;

    qreal cropAspectRatio = 0.0;

    enum class Background {
        Grey,
        Black,
        White
    };
    Background currentBackground = Background::Grey;

    Qt::CursorShape defaultCursorShape = Qt::ArrowCursor;

    QWidget *cropPanel = nullptr;
    QPushButton *cancelButton = nullptr;
    QPushButton *cropButton = nullptr;

    QGraphicsPixmapItem* pixmapItem() const;
    void updateSceneRectAroundItem(QGraphicsPixmapItem *item);
    void centerWelcomeItem();

    QPoint viewportLocalPos(QMouseEvent *event) const;

    void createResizeHandles();
    void updateResizeHandles();
    void removeResizeHandles();
    void setCursorForHandle(Handle h);
    QRectF computeResizedRect(const QRectF& orig, Handle h, const QPointF& mouseScene) const;
    void clearCropSelection();

    void createCropPanel();
    void showCropPanel();
    void hideCropPanel();
    void updateCropPanelPosition();
    void onCropPanelCancel();

    QRectF computeFittedCropRect(QGraphicsPixmapItem* item, qreal aspect) const;
    QRect computeConstrainedRubberRect(const QPoint& start, const QPoint& current, qreal aspect) const;
    void startCropWithRatio(qreal aspect);

    void setBackgroundColor(Background bg);

    void prepareForNewImage();
    void cleanupMovie();

    void replacePixmapItem(const QPixmap &newPix);

    void performZoom(qreal factor, const QPointF &sceneAnchor);

    QAction *openAction = nullptr;
    QAction *fitAction = nullptr;
    QAction *zoomInAction = nullptr;
    QAction *zoomOutAction = nullptr;
    QAction *autoResizeAction = nullptr;
    QAction *resetAction = nullptr;
    QAction *saveAction = nullptr;
    QAction *saveAsAction = nullptr;
    QAction *copyAction = nullptr;
    QAction *copyToAction = nullptr;
    QAction *moveToAction = nullptr;
    QAction *deleteAction = nullptr;
    QAction *renameAction = nullptr;
    QAction *aboutAction = nullptr;
    QAction *propertiesAction = nullptr;
    QAction *animationToggleAction = nullptr;

    QAction *prevAction = nullptr;
    QAction *nextAction = nullptr;

    QAction *rotateLeftAction = nullptr;
    QAction *rotateRightAction = nullptr;
    QAction *mirrorAction = nullptr;
    QAction *flipAction = nullptr;
    QAction *resizeAction = nullptr;

    void showContextMenu(const QPoint &globalPos);

    QStringList folderImages;
    int currentImageIndex = -1;

    QPushButton *prevImageButton = nullptr;
    QPushButton *nextImageButton = nullptr;
    QPushButton *fullscreenButton = nullptr;

    QAction *fullscreenAction = nullptr;

    void updateFolderImages(const QString &filePath);
    void updateNavigationState();

    void positionFloatingButtons();
};

#endif
