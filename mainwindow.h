﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#include <QGraphicsDropShadowEffect>
#include <QVector>
#include <QRect>
#include <QEvent>
#define STRETCH_RECT_HEIGHT 10       // 拉伸小矩形的高度;
#define STRETCH_RECT_WIDTH 10        // 拉伸小矩形的宽度;

#define DARK_THEME 0   //暗黑主题
#define LIGHT_THEME 1  //浅色主题

enum WindowStretchRectState
{
    NO_SELECT = 0,              // 鼠标未进入下方矩形区域;
    LEFT_TOP_RECT,              // 鼠标在左上角区域;
    TOP_BORDER,                 // 鼠标在上边框区域;
    RIGHT_TOP_RECT,             // 鼠标在右上角区域;
    RIGHT_BORDER,               // 鼠标在右边框区域;
    RIGHT_BOTTOM_RECT,          // 鼠标在右下角区域;
    BOTTOM_BORDER,              // 鼠标在下边框区域;
    LEFT_BOTTOM_RECT,           // 鼠标在左下角区域;
    LEFT_BORDER                 // 鼠标在左边框区域;
};
#endif // Q_OS_WIN

namespace Ui {
class zxMainWindow;
}

class zxMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit zxMainWindow(QWidget *parent = 0);
    ~zxMainWindow();

    #ifdef Q_OS_WIN
    void calculateCurrentStrechRect();

    WindowStretchRectState getCurrentStretchState(QPoint cursorPos);
    void updateMouseStyle(WindowStretchRectState stretchState);
    void updateWindowSize();
    void setSupportStretch(bool isSupportStretch);

    void mousePressEvent(QMouseEvent *event);    //鼠标点击
    void mouseMoveEvent(QMouseEvent *event);     //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event);  //鼠标释放
    void mouseDoubleClickEvent(QMouseEvent *event); //鼠标双击
    void showEvent(QShowEvent *event);
    void changeEvent(QEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    //创建托盘显示
    void createSystemTray();

    //根据设置窗口样式
    void setWindowsByConf();

private slots:
    void on_toolButton_close_clicked();

    void on_toolButton_min_clicked();

    void on_toolButton_max_clicked();

    void closeWindow();

    void minWindow();

    void maxWindow();

    void rece_toolButton_fullScreen_sign();

private:
    QRect m_leftTopRect;
    QRect m_leftBottomRect;
    QRect m_rightTopRect;
    QRect m_rightBottomRect;
    QRect m_topBorderRect;
    QRect m_rightBorderRect;
    QRect m_bottomBorderRect;
    QRect m_leftBorderRect;

    QRect mWindow;
    QSystemTrayIcon* trayIcon;

    QGraphicsDropShadowEffect * defaultShadow;
    QPoint last;                //窗口拖动用变量
    QPoint m_startPoint;
    QPoint m_endPoint;
    bool showFlag = false;      //窗口显示标志位 默认false 正常显示
    bool isPressedWidget = false;

    WindowStretchRectState m_stretchRectState;
    bool m_isMousePressed;
    QRect m_windowRectBeforeStretch;
    bool m_isSupportStretch;

    bool isShowToolKit = false; //是否显示工具栏
    bool isFullScreen = false;  //是否全屏
    bool isMaxShow = false;     //是否最大化显示
    #endif // Q_OS_WIN

private:
    Ui::zxMainWindow *ui;
};

#endif // MAINWINDOW_H
