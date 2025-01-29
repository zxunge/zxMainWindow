#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN   // Windows 下使用不同界面
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QMouseEvent>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QDebug>
#include <QPropertyAnimation>
#include <QScreen>
#include <QCoreApplication>
#include <QTimer>
#include <QDateTime>
#include <QClipboard>
#include <QParallelAnimationGroup>
#include <Windows.h>
#include <qt_windows.h>
#include <QSystemTrayIcon>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QShortcut>
#include <QDesktopServices>
#include <QFile>
#include <QListView>
#include <QFormLayout>
#include <QDesktopWidget>
#include "windows.h"
#include "windowsx.h"
#endif // Q_OS_WIN

zxMainWindow::zxMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    #ifdef Q_OS_WIN
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    // QMainWindow透明显示，当设置主显示窗口的外边距时，防止外边距显示出来。
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    setContentsMargins(10, 10, 10, 10);
    ui->toolButton_max->setIcon(QIcon(":/wndimg/icon_max.png"));

    //设置阴影效果
    defaultShadow = new QGraphicsDropShadowEffect();
    //模糊半径
    defaultShadow->setBlurRadius(15);
    //颜色值
    defaultShadow->setColor(QColor(0, 0, 0));
    //横纵偏移量
    defaultShadow->setOffset(0, 0);
    //不要直接给this，会报UpdateLayeredWindowIndirect failed
    ui->centralWidget->setGraphicsEffect(defaultShadow);

    //这里包含对双击标题栏的处理
    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | CS_DBLCLKS);


    //快捷键 F11 全屏
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(rece_toolButton_fullScreen_sign()));

    setSupportStretch(true);
    #endif // Q_OS_WIN
}

#ifdef Q_OS_WIN
void zxMainWindow::calculateCurrentStrechRect()
{

    // 四个角Rect;
    m_leftTopRect = QRect(0, 0, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    m_leftBottomRect = QRect(0, ththis-is->height() - STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, STRETCH_RECT_WIDTH);
    m_rightTopRect = QRect(this->width() - STRETCH_RECT_WIDTH, 0, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    m_rightBottomRect = QRect(this->width() - STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

    // 四条边Rect;
    m_topBorderRect = QRect(STRETCH_RECT_WIDTH, 0, this->width() - STRETCH_RECT_WIDTH * 2, STRETCH_RECT_HEIGHT);
    m_rightBorderRect = QRect(this->width() - STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT * 2);
    m_bottomBorderRect = QRect(STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT, this->width() - STRETCH_RECT_WIDTH * 2, STRETCH_RECT_HEIGHT);
    m_leftBorderRect = QRect(0, STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT * 2);
}

WindowStretchRectState zxMainWindow::getCurrentStretchState(QPoint cursorPos)
{
    WindowStretchRectState stretchState;
    if (m_leftTopRect.contains(cursorPos)) {
        stretchState = LEFT_TOP_RECT;
    } else if (m_rightTopRect.contains(cursorPos)) {
        stretchState = RIGHT_TOP_RECT;
    } else if (m_rightBottomRect.contains(cursorPos)) {
        stretchState = RIGHT_BOTTOM_RECT;
    } else if (m_leftBottomRect.contains(cursorPos)) {
        stretchState = LEFT_BOTTOM_RECT;
    } else if (m_topBorderRect.contains(cursorPos)) {
        stretchState = TOP_BORDER;
    } else if (m_rightBorderRect.contains(cursorPos)) {
        stretchState = RIGHT_BORDER;
    } else if (m_bottomBorderRect.contains(cursorPos)) {
        stretchState = BOTTOM_BORDER;
    } else if (m_leftBorderRect.contains(cursorPos)) {
        stretchState = LEFT_BORDER;
    } else {
        stretchState = NO_SELECT;
    }
    return stretchState;
}

void zxMainWindow::updateMouseStyle(WindowStretchRectState stretchState)
{

    switch (stretchState)
    {
    case NO_SELECT:
        setCursor(Qt::ArrowCursor);
        break;
    case LEFT_TOP_RECT:
    case RIGHT_BOTTOM_RECT:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TOP_BORDER:
    case BOTTOM_BORDER:
        setCursor(Qt::SizeVerCursor);
        break;
    case RIGHT_TOP_RECT:
    case LEFT_BOTTOM_RECT:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case LEFT_BORDER:
    case RIGHT_BORDER:
        setCursor(Qt::SizeHorCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }

}

void zxMainWindow::updateWindowSize()
{
    // 拉伸时要注意设置窗口最小值;
    QRect windowRect = m_windowRectBeforeStretch;
    int delValue_X = m_startPoint.x() - m_endPoint.x();
    int delValue_Y = m_startPoint.y() - m_endPoint.y();
    int m_windowMinWidth = 600;
    int m_windowMinHeight = 600;
    if (m_stretchRectState == LEFT_BORDER) {
        if (this->geometry().width() <= m_windowMinWidth && delValue_X <= 0) {
            return;
        }
        QPoint bottomLeftPoint = windowRect.bottomLeft();
        bottomLeftPoint.setX(bottomLeftPoint.x() - delValue_X);
        windowRect.setBottomLeft(bottomLeftPoint);
        this->setGeometry(windowRect);

    } else if (m_stretchRectState == RIGHT_BORDER) {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setX(bottomRightPoint.x() - delValue_X);
        windowRect.setBottomRight(bottomRightPoint);
        this->setGeometry(windowRect);
    } else if (m_stretchRectState == TOP_BORDER) {
        if (this->geometry().height() <= m_windowMinHeight && delValue_Y <= 0) {
            return;
        }
        QPoint topLeftPoint = windowRect.topLeft();
        topLeftPoint.setY(topLeftPoint.y() - delValue_Y);
        windowRect.setTopLeft(topLeftPoint);
        this->setGeometry(windowRect);
    } else if (m_stretchRectState == BOTTOM_BORDER)  {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setY(bottomRightPoint.y() - delValue_Y);
        windowRect.setBottomRight(bottomRightPoint);
        this->setGeometry(windowRect);
    } else if (m_stretchRectState == LEFT_TOP_RECT) {
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0
                && this->geometry().height() -3 <= m_windowMinHeight && delValue_Y <= 0) {
            return;
        }
        int a = 0;
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0) {
            a = 1;
        }
        if (this->geometry().height() - 3 <= m_windowMinHeight && delValue_Y <= 0) {
            a = 2;
        }
        if (a == 0) {
            //qDebug() << "走1";
            QPoint topLeftPoint = windowRect.topLeft();
            topLeftPoint.setX(topLeftPoint.x() - delValue_X);
            topLeftPoint.setY(topLeftPoint.y() - delValue_Y);
            windowRect.setTopLeft(topLeftPoint);
            this->setGeometry(windowRect);
        } else if (a == 1) {
            QPoint topLeftPoint = windowRect.topLeft();
            topLeftPoint.setX(this->geometry().x());
            topLeftPoint.setY(topLeftPoint.y() - delValue_Y);
            windowRect.setTopLeft(topLeftPoint);
            this->setGeometry(windowRect);
        } else if (a == 2) {
            QPoint topLeftPoint = windowRect.topLeft();
            topLeftPoint.setX(topLeftPoint.x() - delValue_X);
            topLeftPoint.setY(this->geometry().y());
            windowRect.setTopLeft(topLeftPoint);
            this->setGeometry(windowRect);
        }
    } else if (m_stretchRectState == RIGHT_TOP_RECT) {
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0
                && this->geometry().height() -3 <= m_windowMinHeight && delValue_Y <= 0) {
            return;
        }
        int a = 0;
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0) {
            a = 1;
        }
        if (this->geometry().height() - 3 <= m_windowMinHeight && delValue_Y <= 0) {
            a = 2;
        }

        if (a == 0) {
            QPoint topRightPoint = windowRect.topRight();
            topRightPoint.setX(topRightPoint.x() - delValue_X);
            topRightPoint.setY(topRightPoint.y() - delValue_Y);
            windowRect.setTopRight(topRightPoint);
            this->setGeometry(windowRect);
        } else if (a == 1) {
            QPoint topRightPoint = windowRect.topRight();
            topRightPoint.setX(this->geometry().x());
            topRightPoint.setY(topRightPoint.y() - delValue_Y);
            windowRect.setTopRight(topRightPoint);
            this->setGeometry(windowRect);
        } else if (a == 2) {
            QPoint topRightPoint = windowRect.topRight();
            topRightPoint.setX(topRightPoint.x() - delValue_X);
            topRightPoint.setY(this->geometry().y());
            windowRect.setTopRight(topRightPoint);
            this->setGeometry(windowRect);
        }
    } else if (m_stretchRectState == RIGHT_BOTTOM_RECT) {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setX(bottomRightPoint.x() - delValue_X);
        bottomRightPoint.setY(bottomRightPoint.y() - delValue_Y);
        windowRect.setBottomRight(bottomRightPoint);
        this->setGeometry(windowRect);
    } else if (m_stretchRectState == LEFT_BOTTOM_RECT) {
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0
                && this->geometry().height() -3 <= m_windowMinHeight && delValue_Y <= 0) {
            return;
        }
        int a = 0;
        if (this->geometry().width() - 3 <= m_windowMinWidth && delValue_X <= 0) {
            a = 1;
        }
        if (this->geometry().height() - 3 <= m_windowMinHeight && delValue_Y <= 0) {
            a = 2;
        }
        if (a == 0) {
            QPoint bottomLeftPoint = windowRect.bottomLeft();
            bottomLeftPoint.setX(bottomLeftPoint.x() - delValue_X);
            bottomLeftPoint.setY(bottomLeftPoint.y() - delValue_Y);
            windowRect.setBottomLeft(bottomLeftPoint);
            this->setGeometry(windowRect);
        } else if (a == 1) {
            QPoint bottomLeftPoint = windowRect.bottomLeft();
            bottomLeftPoint.setX(this->geometry().x());
            bottomLeftPoint.setY(bottomLeftPoint.y() - delValue_Y);
            windowRect.setBottomLeft(bottomLeftPoint);
            this->setGeometry(windowRect);
        } else if (a == 2) {
            QPoint bottomLeftPoint = windowRect.bottomLeft();
            bottomLeftPoint.setX(bottomLeftPoint.x() - delValue_X);
            bottomLeftPoint.setY(this->geometry().y());
            windowRect.setBottomLeft(bottomLeftPoint);
            this->setGeometry(windowRect);
        }
    }
}

void zxMainWindow::setSupportStretch(bool isSupportStretch)
{
    // 因为需要在鼠标未按下的情况下通过mouseMoveEvent事件捕捉鼠标位置，所以需要设置setMouseTracking为true（如果窗口支持拉伸）;

        m_isSupportStretch = isSupportStretch;
        this->setMouseTracking(isSupportStretch);
        // 这里对子控件也进行了设置，是因为如果不对子控件设置，当鼠标移动到子控件上时，不会发送mouseMoveEvent事件，也就获取不到当前鼠标位置，无法判断鼠标状态及显示样式了。
        QList<QWidget*> widgetList = this->findChildren<QWidget*>();
        for(int i = 0; i < widgetList.length(); i ++) {
            widgetList[i]->setMouseTracking(isSupportStretch);
        }

}

void zxMainWindow::mousePressEvent(QMouseEvent *event)
{
    if(ui->widget_title->underMouse() && !showFlag) {
        isPressedWidget = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    }
    last = event->globalPos();
    // 当前鼠标进入了以上指定的8个区域，并且是左键按下时才开始进行窗口拉伸;
    if (m_stretchRectState != NO_SELECT && event->button() == Qt::LeftButton)
    {
        m_isMousePressed = true;
        // 记录下当前鼠标位置，为后面计算拉伸位置;
        m_startPoint = this->mapToGlobal(event->pos());
        // 保存下拉伸前的窗口位置及大小;
        m_windowRectBeforeStretch = this->geometry();
    }
}

void zxMainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (isPressedWidget) {
        int dx = event->globalX() - last.x();
        int dy = event->globalY() - last.y();
        last = event->globalPos();
        move(x()+dx, y()+dy);
    }

    if (!m_isMousePressed) {
        QPoint cursorPos = event->pos();
        // 根据当前鼠标的位置显示不同的样式;
        m_stretchRectState = getCurrentStretchState(cursorPos);
        updateMouseStyle(m_stretchRectState);
    } else {
        // 如果当前鼠标左键已经按下，则记录下第二个点的位置，并更新窗口的大小;
        m_endPoint = this->mapToGlobal(event->pos());
        updateWindowSize();
    }
}

void zxMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isPressedWidget = false; // 鼠标松开时，置为false
    // 鼠标松开后意味之窗口拉伸结束，置标志位，并且重新计算用于拉伸的8个区域Rect;
    m_isMousePressed = false;
    calculateCurrentStrechRect();
}

void zxMainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    //不走这里
}

void zxMainWindow::showEvent(QShowEvent *event)
{
    calculateCurrentStrechRect();
    //防止假死
    setAttribute(Qt::WA_Mapped);
    QMainWindow::showEvent(event);
    QSize oldSize = this->size();
    resize(oldSize + QSize(10, 10));
    resize(oldSize);
    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | CS_DBLCLKS);
}

void zxMainWindow::changeEvent(QEvent *event)
{
    if(QEvent::WindowStateChange == event->type())
    {
        //判断为窗口状态改变事件
        QWindowStateChangeEvent * stateEvent = dynamic_cast<QWindowStateChangeEvent*>(event);
        if(Q_NULLPTR != stateEvent)
        {
            if(this->windowState() == Qt::WindowMinimized)
            {
                //qDebug() << "当前最小化";
            } else if (this->windowState() == Qt::WindowNoState && stateEvent->oldState() == Qt::WindowMaximized) {
                //qDebug() << "当前正常";
                ui->centralWidget->setStyleSheet("#centralWidget {background-color: rgb(67, 67, 67);border-image: url(:/wndimg/back.png);border-radius:10px;}");
                ui->toolButton_close->setStyleSheet("QToolButton {\
                                                        color: rgb(255, 255, 255);\
                                                        border-top-right-radius: 9px;\
                                                        background-color: rgba(94, 255, 210, 0);\
                                                        border: none;\
                                                    }\
                                                    QToolButton::menu-indicator { \
                                                        image: None;\
                                                    }\
                                                    QToolButton:hover {\
                                                        color: rgb(255, 255, 255);\
                                                        background-color: rgb(200, 0, 0);\
                                                        border: none;\
                                                    }");
                ui->toolButton_max->setIcon(QIcon(":/wndimg/icon_max.png"));
            } else if (this->windowState() == Qt::WindowMaximized && stateEvent->oldState() == Qt::WindowNoState) {
                //qDebug() << "当前最大化";
                ui->centralWidget->setStyleSheet("#centralWidget {background-color: rgb(67, 67, 67);border-image: url(:/wndimg/back.png);border-radius:0px;}");
                ui->toolButton_close->setStyleSheet("QToolButton {\
                                                        color: rgb(255, 255, 255);\
                                                        border-top-right-radius: 0px;\
                                                        background-color: rgba(94, 255, 210, 0);\
                                                        border: none;\
                                                    }\
                                                    QToolButton::menu-indicator { \
                                                        image: None;\
                                                    }\
                                                    QToolButton:hover {\
                                                        color: rgb(255, 255, 255);\
                                                        background-color: rgb(200, 0, 0);\
                                                        border: none;\
                                                    }");
                ui->toolButton_max->setIcon(QIcon(":/wndimg/icon_normal.png"));
            }
        }
    }
}

bool zxMainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = (MSG*)message;
    switch (msg->message) {
    //没有这一段，将不会显示窗口
    case WM_NCCALCSIZE:
        return true;

    case WM_NCHITTEST:
    {
        //qDebug() << "触发WM_NCHITTEST";
        qreal ratio = 1.0;
        long x = GET_X_LPARAM(msg->lParam) / ratio;
        long y = GET_Y_LPARAM(msg->lParam) / ratio;
        QPoint pos = mapFromGlobal(QPoint(x, y));
        //qDebug() << "pos = " << pos;
        if (pos.y() > 10 && ui->widget_title2->rect().contains(pos)) {
            //qDebug() << "标题栏被按下";
            // 根据当前鼠标的位置显示不同的样式;
            *result = HTCAPTION;
            return true;
        }
    }   
    case WM_GETMINMAXINFO:
    {
        if (::IsZoomed(msg->hwnd)) {
            isMaxShow = true;
            showFlag = true;
            // 最大化时会超出屏幕，所以填充边框间距
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            frame.left = abs(frame.left);
            frame.top = abs(frame.bottom);
            this->setContentsMargins(frame.left, frame.top, frame.right, frame.bottom);
        } else {
            isMaxShow = false;
            showFlag = false;
        }
        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        return true;
    }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void zxMainWindow::createSystemTray()
{
    // 创建系统托盘图标
    trayIcon = new QSystemTrayIcon(QIcon("// Your code here"), this);
    trayIcon->setToolTip("// Your code here");
    trayIcon->show();

    // 创建一个菜单
    QMenu* menu = new QMenu();
    menu->setWindowFlags(menu->windowFlags()  | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    QAction* openAction = new QAction(tr("Open"));
    QAction* closeAction = new QAction(tr("Exit"));
    menu->addAction(openAction);
    menu->addAction(closeAction);
    menu->setStyleSheet(getStyleFile(":/qss/menu.qss"));
    // 将菜单设置给系统托盘图标
    trayIcon->setContextMenu(menu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(rece_systemTrayMenu()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(rece_systemTrayMenu()));
}

void zxMainWindow::setWindowsByConf()
{
}

void zxMainWindow::on_toolButton_close_clicked()
{
    this->close();
}

void zxMainWindow::on_toolButton_min_clicked()
{
    this->showMinimized();
}

void zxMainWindow::on_toolButton_max_clicked()
{
    if (!showFlag) {
        setContentsMargins(0, 0, 0, 0);
        this->setWindowState(Qt::WindowState::WindowMaximized);
        isMaxShow = true;
        showFlag = true;
    } else {
        setContentsMargins(10, 10, 10, 10);
        this->setWindowState(Qt::WindowState::WindowNoState);
        isMaxShow = false;
        showFlag = false;
    }
}

void zxMainWindow::closeWindow()
{
    this->showMinimized();
}

void zxMainWindow::minWindow()
{
    this->showMinimized();
}

void zxMainWindow::maxWindow()
{
    //maxWindow
}

void zxMainWindow::rece_toolButton_fullScreen_sign()
{
    qDebug() << "isFullScreen = " <<isFullScreen;
    if (!isFullScreen) {
        setContentsMargins(0, 0, 0, 0);
        ui->centralWidget->setStyleSheet("#centralWidget {border-image: url(:/wndimg/back.png);border-radius:0px;}");
        ui->toolButton_max_2->setIcon(QIcon(":/wndimg/icon_normal.png"));
        //this->showNormal();
        ui->toolButton_min->hide();
        ui->toolButton_max->hide();
        ui->toolButton_close->hide();
        this->showFullScreen();
        isFullScreen = true;
    } else {
        ui->toolButton_min->show();
        ui->toolButton_max->show();
        ui->toolButton_close->show();
        setContentsMargins(10, 10, 10, 10); //rgb(67, 77, 88)
        ui->centralWidget->setStyleSheet("#centralWidget {border-image: url(:/wndimg/back.png);border-radius:10px;}");
        ui->toolButton_max->setIcon(QIcon(":/wndimg/icon_max.png"));
        if (showFlag) {
            ui->centralWidget->setStyleSheet("#centralWidget {border-image: url(:/wndimg/back.png);border-radius:0px;}");
            setContentsMargins(0, 0, 0, 0);
            ui->toolButton_max->setIcon(QIcon(":/wndimg/icon_normal.png"));
            this->showMaximized();
        } else {
            this->showNormal();
        }
        isFullScreen = false;
    }

}
#endif // Q_OS_WIN

zxMainWindow::~zxMainWindow()
{
    delete ui;