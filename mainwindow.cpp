#include "mainwindow.h"
#include <QPixmap>
#include <QUrl>
#include <QMenuBar>   // 用于隐藏菜单栏
#include <QStatusBar> // 用于隐藏状态栏
#include <QRect>      // 用于设置控件的几何范围
#include <QFontDatabase> // 用于动态加载外部字体文件
#include <QDebug>     // 用于在控制台打印网格坐标和报错信息
#include <QIcon>
#include <QSize>
#include <QCursor>
#include "sunflower.h"  // 【关键修复】：必须加上这一行！告诉编译器向日葵长什么样
#include "peashooter.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), currentStage(0)
{
    ui->setupUi(this);

    // 【消灭横线】隐藏 QMainWindow 默认自带的菜单栏和状态栏
    this->menuBar()->hide();
    this->statusBar()->hide();

    // 1. 设置游戏窗口的分辨率
    this->setFixedSize(1200, 900);

    // 2. 把主窗口的底色强制设为纯黑
    this->setStyleSheet("QMainWindow { background-color: black; }");

    // 3. 创建一个占满全屏的 Label，作为所有开场过场图片的“画板”
    imageLabel = new QLabel(this);
    imageLabel->setGeometry(0, 0, 1200, 900);
    imageLabel->setAlignment(Qt::AlignCenter);

    // 4. 【动画魔法配置】给主画板加上透明度特效
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    imageLabel->setGraphicsEffect(opacityEffect);

    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(800);

    // 5. 草地画板配置（用于实现草地从左向右铺开的卷轴效果）
    floorLabel = new QLabel(this);
    floorLabel->setPixmap(QPixmap(":/res/images/floor.png").scaled(1200, 225, Qt::IgnoreAspectRatio));
    floorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    floorLabel->hide();

    rollAnimation = new QPropertyAnimation(floorLabel, "geometry", this);
    rollAnimation->setDuration(1500);

    // 6. 字体加载逻辑
    int fontId = QFontDatabase::addApplicationFont(":/res/font/pvz_btn.ttf");
    QString pvzFontFamily = "Arial";

    if (fontId != -1) {
        pvzFontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    }
    else {
        qDebug() << "!!!!! 严重错误：字体加载失败，请检查 qrc 路径 !!!!!";
    }

    // 7. 创建“点击进入游戏”交互按钮
    startButton = new QPushButton(QStringLiteral("点击进入游戏"), this);
    startButton->setGeometry(450, 780, 300, 75);

    QString btnStyle = QString(
        "QPushButton { "
        "   font-family: '%1'; "
        "   color: white; "
        "   font-size: 48px; "
        "   font-weight: bold; "
        "   background: transparent; "
        "   border: none; "
        "}"
        "QPushButton:hover { color: #84cc16; }"
    ).arg(pvzFontFamily);

    startButton->setStyleSheet(btnStyle);
    startButton->hide();
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startGame);

    // ---------------- 【构建主菜单界面】 ----------------
    menuBgLabel = new QLabel(this);
    menuBgLabel->setGeometry(0, 0, 1200, 900);
    menuBgLabel->setPixmap(QPixmap(":/res/images/Surface.png").scaled(1200, 900, Qt::IgnoreAspectRatio));
    menuBgLabel->hide();

    btnAdventure = new QPushButton(this);
    btnAdventure->setGeometry(615, 120, 480, 165);
    btnAdventure->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/mx.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/mx1.png); }"
    );
    btnAdventure->hide();
    connect(btnAdventure, &QPushButton::clicked, this, &MainWindow::startAdventure);

    btnMiniGames = new QPushButton(this);
    btnMiniGames->setGeometry(615, 270, 450, 150);
    btnMiniGames->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/mini.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/mini1.png); }"
    );
    btnMiniGames->hide();

    btnPuzzle = new QPushButton(this);
    btnPuzzle->setGeometry(615, 405, 420, 135);
    btnPuzzle->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/yizi.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/yizi1.png); }"
    );
    btnPuzzle->hide();

    btnPlay = new QPushButton(this);
    btnPlay->setGeometry(615, 525, 405, 127);
    btnPlay->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/play.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/play1.png); }"
    );
    btnPlay->hide();

    // ---------------- 【构建战斗草坪与小推车】 ----------------
    combatBgLabel = new QLabel(this);
    combatBgLabel->setPixmap(QPixmap(":/res/images/Background.jpg").scaled(1600, 900, Qt::IgnoreAspectRatio));
    combatBgLabel->setGeometry(-20, 0, 1600, 900);
    combatBgLabel->hide();

    for (int i = 0; i < 5; ++i) {
        lawnMowers[i] = new QLabel(this);
        lawnMowers[i]->setPixmap(QPixmap(":/res/images/LawnMower.png").scaled(105, 105, Qt::KeepAspectRatio));
        lawnMowers[i]->setGeometry(168, 150 + i * 145, 105, 105);
        lawnMowers[i]->hide();
    }

    // ---------------- 【构建顶部植物商店 (Shop UI) 与经济系统】 ----------------
    shopBoard = new QLabel(this);
    shopBoard->setPixmap(QPixmap(":/res/images/Shop.png").scaled(650, 115, Qt::IgnoreAspectRatio));
    shopBoard->setGeometry(150, 0, 650, 115);
    shopBoard->hide();

    // 【新增：阳光计分板显示】
    sunLabel = new QLabel(this);
    sunLabel->setGeometry(165, 75, 60, 30); // 精准定位在太阳图标上
    sunLabel->setText(QString::number(sunCount));
    sunLabel->setAlignment(Qt::AlignCenter);
    sunLabel->setStyleSheet("QLabel { color: black; font-size: 22px; font-weight: bold; }");
    sunLabel->hide();

    sunCardBtn = new QPushButton(this);
    sunCardBtn->setGeometry(255, 10, 65, 90);
    sunCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; }");
    sunCardBtn->setIcon(QIcon(":/res/images/SunFlower.png"));
    sunCardBtn->setIconSize(QSize(50, 50));
    sunCardBtn->hide();

    peaCardBtn = new QPushButton(this);
    peaCardBtn->setGeometry(325, 10, 65, 90);
    peaCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; }");
    peaCardBtn->setIcon(QIcon(":/res/images/Peashooter.png"));
    peaCardBtn->setIconSize(QSize(50, 50));
    peaCardBtn->hide();

    // ---------------- 【核心交互：买前验资】 ----------------
    connect(sunCardBtn, &QPushButton::clicked, [this]() {
        if (sunCount >= 50) { // 【验资】：有 50 阳光才能买向日葵！
            qDebug() << "【商店系统】选中向日葵！";
            currentMouseState = HoldingSunflower;
            this->setCursor(QCursor(QPixmap(":/res/images/SunFlower.png").scaled(50, 50)));
        }
        else {
            qDebug() << "【经济系统】阳光不足！向日葵需要 50 阳光。";
        }
        });

    connect(peaCardBtn, &QPushButton::clicked, [this]() {
        if (sunCount >= 100) { // 【验资】：有 100 阳光才能买豌豆射手！
            qDebug() << "【商店系统】选中豌豆射手！";
            currentMouseState = HoldingPeashooter;
            this->setCursor(QCursor(QPixmap(":/res/images/Peashooter.png").scaled(50, 50)));
        }
        else {
            qDebug() << "【经济系统】阳光不足！豌豆射手需要 100 阳光。";
        }
        });

    // ---------------- 【音效与定时器系统】 ----------------
    bgm = new QSoundEffect(this);
    bgm->setSource(QUrl("qrc:/res/sound/Grazy.wav"));
    bgm->setLoopCount(QSoundEffect::Infinite);
    bgm->setVolume(0.6f);
    bgm->play();

    loadingTimer = new QTimer(this);
    connect(loadingTimer, &QTimer::timeout, this, &MainWindow::nextLoadingStage);
    QTimer::singleShot(500, this, &MainWindow::nextLoadingStage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ====================================================================================
// 核心状态机：开场动画
// ====================================================================================
void MainWindow::nextLoadingStage()
{
    switch (currentStage) {
    case 0:
        imageLabel->setPixmap(QPixmap(":/res/images/init.png").scaled(1200, 900, Qt::KeepAspectRatio));
        fadeAnimation->setStartValue(0.0);
        fadeAnimation->setEndValue(1.0);
        fadeAnimation->start();
        loadingTimer->start(2000);
        break;

    case 1:
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);
        fadeAnimation->start();
        loadingTimer->start(800);
        break;

    case 2:
        imageLabel->setPixmap(QPixmap(":/res/images/LogoWord.jpg").scaled(900, 450, Qt::KeepAspectRatio));
        fadeAnimation->setStartValue(0.0);
        fadeAnimation->setEndValue(1.0);
        fadeAnimation->start();
        loadingTimer->start(2000);
        break;

    case 3:
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);
        fadeAnimation->start();
        loadingTimer->start(800);
        break;

    case 4:
        imageLabel->setPixmap(QPixmap(":/res/images/StartScreen.jpg").scaled(1200, 900, Qt::IgnoreAspectRatio));
        fadeAnimation->setStartValue(0.0);
        fadeAnimation->setEndValue(1.0);
        fadeAnimation->start();
        loadingTimer->start(1000);
        break;

    case 5:
        floorLabel->show();
        rollAnimation->setStartValue(QRect(0, 675, 0, 225));
        rollAnimation->setEndValue(QRect(0, 675, 1200, 225));
        rollAnimation->start();
        loadingTimer->start(1600);
        break;

    case 6:
        loadingTimer->stop();
        startButton->show();
        break;
    }
    currentStage++;
}

// ====================================================================================
// UI 切换与底层核心逻辑
// ====================================================================================
void MainWindow::startGame()
{
    imageLabel->hide();
    floorLabel->hide();
    startButton->hide();

    menuBgLabel->show();
    btnAdventure->show();
    btnMiniGames->show();
    btnPuzzle->show();
    btnPlay->show();

    btnAdventure->raise();
    btnMiniGames->raise();
    btnPuzzle->raise();
    btnPlay->raise();
}

void MainWindow::startAdventure()
{
    menuBgLabel->hide();
    btnAdventure->hide();
    btnMiniGames->hide();
    btnPuzzle->hide();
    btnPlay->hide();

    combatBgLabel->show();
    for (int i = 0; i < 5; ++i) {
        lawnMowers[i]->show();
    }

    // 显示顶部商店
    shopBoard->show();
    shopBoard->raise();

    // 【新增：显示阳光计分板】
    sunLabel->show();
    sunLabel->raise();

    sunCardBtn->show();
    sunCardBtn->raise();

    peaCardBtn->show();
    peaCardBtn->raise();

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) {
            grassGrid[i][j] = 0;
        }
    }
}

// ====================================================================================
// 核心输入：重写鼠标点击事件，精准识别玩家点击的 5x9 草地网格坐标
// ====================================================================================
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (!combatBgLabel->isVisible()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {

        int x = event->position().x();
        int y = event->position().y();

        int startY = 135;
        int cellH = 145;

        // 【列边界数组查表法】
        int colEdges[10] = { 232, 325, 420, 510, 595, 680, 765, 845, 920, 1000 };
        int col = -1;

        for (int i = 0; i < 9; ++i) {
            if (x >= colEdges[i] && x < colEdges[i + 1]) {
                col = i;
                break;
            }
        }

        int row = (y - startY) / cellH;

        if (y >= startY && row >= 0 && row < 5 && col >= 0) {

            if (grassGrid[row][col] == 0) {

                if (currentMouseState == None) {
                    qDebug() << "【核心层响应】手里没植物，无法种植！";
                    return;
                }

                qDebug() << "【核心层响应】精准命中逻辑网格！ -> 第" << row << "行, 第" << col << "列";

                int offsetX = 40;
                int offsetY = 45;

                int placeX = colEdges[col] + offsetX;
                int placeY = startY + row * cellH + offsetY;

                // ---------------------------------------------------------
                // 【状态分发与扣款】：实例化植物并扣除对应的阳光
                // ---------------------------------------------------------
                if (currentMouseState == HoldingSunflower) {
                    Sunflower* sun = new Sunflower(row, col, this);
                    sun->move(placeX, placeY);
                    sun->show();

                    sunCount -= 50; // 【核心经济逻辑】：扣款 50
                }
                else if (currentMouseState == HoldingPeashooter) {
                    PeaShooter* pea = new PeaShooter(row, col, this);
                    pea->move(placeX, placeY);
                    pea->show();

                    sunCount -= 100; // 【核心经济逻辑】：扣款 100
                }

                // 锁定草地状态
                grassGrid[row][col] = 1;

                // 清空鼠标状态，恢复指针
                currentMouseState = None;
                this->unsetCursor();

                // 【刷新计分板】：更新屏幕上的阳光数值
                sunLabel->setText(QString::number(sunCount));

            }
            else {
                qDebug() << "【核心层响应】无效操作：该格子已被其他植物占用！";
            }
        }
        else {
            qDebug() << "【核心层响应】无效点击：点击位置已超出可种植草坪范围！";
        }
    }
}