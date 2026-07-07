#include "mainwindow.h"
#include <QPixmap>
#include <QUrl>
#include <QMenuBar>   
#include <QStatusBar> 
#include <QRect>      
#include <QFontDatabase> 
#include <QDebug>     
#include <QIcon>
#include <QSize>
#include <QCursor>
#include <QRandomGenerator> 
#include <QVBoxLayout>
#include <QApplication>
#include <QGraphicsProxyWidget>

// 引入场景中可能出现的所有具体派生类
#include "lawnmower.h"
#include "sunflower.h"
#include "peashooter.h"
#include "wallnut.h"
#include "peabullet.h"
#include "zombie.h" 
#include "sun.h"            
#include "repeater.h"
#include "cherrybomb.h"

// ====================================================================================
// 🛠️ 构造函数：游戏引擎与 UI 框架的初始化
// ====================================================================================
mainwindow::mainwindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), currentStage(0),
    zombieSpawnTimer(nullptr), skySunTimer(nullptr), progressTimer(nullptr),
    totalZombies(15), spawnedZombies(0), killedZombies(0),
    isPaused(false), isDaveTalking(false), daveStep(0),
    daveItem(nullptr), bubbleItem(nullptr), isGameInitialized(false)
{
    ui->setupUi(this);

    // 【数据结构】：初始化 5x9 的草坪逻辑网格，0代表当前地块为空
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) { grassGrid[i][j] = 0; }
    }

    // 锁定窗口尺寸，隐藏系统原生菜单栏，为绝对坐标计算提供稳定的 1000x600 视口
    ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->menuBar()->hide();
    this->statusBar()->hide();
    this->setFixedSize(1000, 600);
    this->setStyleSheet("QMainWindow { background-color: black; }");

    // 【MVC 架构：Scene 模型层】：管理所有游戏实体
    gameScene = new QGraphicsScene(this);
    gameScene->setSceneRect(0, 0, 1400, 600);

    // 绘制并垫底游戏战斗背景 (Z-Value = -10 确保在最底层)
    combatBgItem = new QGraphicsPixmapItem(QPixmap(":/res/images/Background.jpg"));
    combatBgItem->setZValue(-10);
    gameScene->addItem(combatBgItem);

    // 【MVC 架构：View 视图层】：负责渲染 Scene
    gameView = new QGraphicsView(gameScene, this);
    gameView->setGeometry(0, 0, 1000, 600);
    gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    gameView->setStyleSheet("background: transparent; border: none;");
    gameView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    gameView->setCacheMode(QGraphicsView::CacheBackground); // 开启背景缓存，优化渲染性能
    gameView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    gameView->hide();

    // 【架构亮点】：将 view 的事件委托给主窗口处理，实现鼠标点击的全局拦截
    gameView->viewport()->installEventFilter(this);

    // ====================================================================================
    // 🎨 UI 组件构建区：暂停菜单、开场动画、主菜单按钮与音效加载
    // ====================================================================================
    pauseWidget = new QWidget(gameView);
    pauseWidget->setGeometry(0, 0, 1000, 600);
    pauseWidget->setStyleSheet("background-color: rgba(0, 0, 0, 160);");
    pauseWidget->hide();

    QLabel* tombstoneBg = new QLabel(pauseWidget);
    tombstoneBg->setPixmap(QPixmap(":/res/images/options_menuback.png").scaled(400, 450, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    tombstoneBg->setGeometry(300, 75, 400, 450);
    tombstoneBg->setStyleSheet("background: transparent;");

    QPushButton* resumeBtn = new QPushButton("返回游戏", pauseWidget);
    resumeBtn->setGeometry(425, 290, 150, 45);
    resumeBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/button_down_middle.png); color: white; font-weight: bold; font-size: 18px; }");
    connect(resumeBtn, &QPushButton::clicked, this, &mainwindow::togglePause);

    QPushButton* exitBtn = new QPushButton("返回主菜单", pauseWidget);
    exitBtn->setGeometry(425, 360, 150, 45);
    exitBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/button_down_middle.png); color: white; font-weight: bold; font-size: 18px; }");
    connect(exitBtn, &QPushButton::clicked, this, &mainwindow::returnToMainMenu);

    imageLabel = new QLabel(this);
    imageLabel->setGeometry(0, 0, 1000, 600);
    imageLabel->setAlignment(Qt::AlignCenter);

    // 【动画引擎】：使用 QGraphicsOpacityEffect 配合 QPropertyAnimation 实现画面淡入淡出
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    imageLabel->setGraphicsEffect(opacityEffect);

    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(800);

    floorLabel = new QLabel(this);
    floorLabel->setPixmap(QPixmap(":/res/images/floor.png").scaled(1000, 225, Qt::IgnoreAspectRatio));
    floorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    floorLabel->hide();

    rollAnimation = new QPropertyAnimation(floorLabel, "geometry", this);
    rollAnimation->setDuration(1500);

    // 加载自定义字体
    int fontId = QFontDatabase::addApplicationFont(":/res/font/pvz_btn.ttf");
    QString pvzFontFamily = "Arial";
    if (fontId != -1) { pvzFontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0); }

    startButton = new QPushButton(QStringLiteral("点击进入游戏"), this);
    startButton->setGeometry(350, 545, 300, 55);
    startButton->setStyleSheet(QString("QPushButton { font-family: '%1'; color: white; font-size: 48px; font-weight: bold; background: transparent; border: none; } QPushButton:hover { color: #84cc16; }").arg(pvzFontFamily));
    startButton->hide();
    connect(startButton, &QPushButton::clicked, this, &mainwindow::startGame);

    menuBgLabel = new QLabel(this);
    menuBgLabel->setGeometry(0, 0, 1000, 600);
    menuBgLabel->setPixmap(QPixmap(":/res/images/Surface.png"));
    menuBgLabel->setAlignment(Qt::AlignCenter);
    menuBgLabel->hide();

    btnAdventure = new QPushButton(this);
    btnAdventure->setGeometry(510, 80, 330, 110);
    btnAdventure->setStyleSheet("QPushButton { border-image: url(:/res/images/mx.png); border: none; } QPushButton:hover { border-image: url(:/res/images/mx1.png); }");
    btnAdventure->hide();
    connect(btnAdventure, &QPushButton::clicked, this, &mainwindow::startAdventure);

    btnMiniGames = new QPushButton(this);
    btnMiniGames->setGeometry(510, 190, 300, 100);
    btnMiniGames->setStyleSheet("QPushButton { border-image: url(:/res/images/mini.png); border: none; } QPushButton:hover { border-image: url(:/res/images/mini1.png); }");
    btnMiniGames->hide();

    btnPuzzle = new QPushButton(this);
    btnPuzzle->setGeometry(510, 290, 280, 90);
    btnPuzzle->setStyleSheet("QPushButton { border-image: url(:/res/images/yizi.png); border: none; } QPushButton:hover { border-image: url(:/res/images/yizi1.png); }");
    btnPuzzle->hide();

    btnPlay = new QPushButton(this);
    btnPlay->setGeometry(510, 380, 270, 85);
    btnPlay->setStyleSheet("QPushButton { border-image: url(:/res/images/play.png); border: none; } QPushButton:hover { border-image: url(:/res/images/play1.png); }");
    btnPlay->hide();

    // 音乐系统初始化
    bgm = new QSoundEffect(this);
    bgm->setSource(QUrl("qrc:/res/sound/Grazy.wav"));
    bgm->setLoopCount(QSoundEffect::Infinite);
    bgm->setVolume(0.6f);
    bgm->play();

    // 启动 Loading 状态机
    loadingTimer = new QTimer(this);
    connect(loadingTimer, &QTimer::timeout, this, &mainwindow::nextLoadingStage);
    QTimer::singleShot(500, this, &mainwindow::nextLoadingStage);

    peaFireSound = new QSoundEffect(this);
    peaFireSound->setSource(QUrl("qrc:/res/sound/throw.wav"));
    peaFireSound->setVolume(0.5f);

    plantSound = new QSoundEffect(this);
    plantSound->setSource(QUrl("qrc:/res/sound/plant.wav"));
    plantSound->setVolume(0.8f);

    gameOverLabel = new QLabel(this);
    gameOverLabel->setGeometry(0, 0, 1000, 600);
    gameOverLabel->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->setPixmap(QPixmap(":/res/images/ZombiesWon.png").scaledToWidth(600, Qt::SmoothTransformation));
    gameOverLabel->hide();

    evilLaughSound = new QSoundEffect(this);
    evilLaughSound->setSource(QUrl("qrc:/res/sound/evillaugh.wav"));
    evilLaughSound->setVolume(1.0f);

    zombieHandLabel = new QLabel(this);
    zombieHandLabel->setGeometry(350, 200, 300, 400);
    zombieHandLabel->setAlignment(Qt::AlignCenter);
    zombieHandLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    zombieHandLabel->setStyleSheet("background: transparent;");

    zombieHandMovie = new QMovie(":/res/images/other/Zombie_hand/Zombie_hand.gif");
    zombieHandLabel->setMovie(zombieHandMovie);
    zombieHandLabel->hide();
    connect(zombieHandMovie, &QMovie::frameChanged, [this]() {
        if (zombieHandMovie->currentFrameNumber() == (zombieHandMovie->frameCount() - 1)) zombieHandMovie->stop();
        });

    daveSound = new QSoundEffect(this);
    daveSound->setVolume(1.0f);
}

// RAII 资源回收：析构函数负责销毁 UI 指针
mainwindow::~mainwindow() { delete ui; }

// =========================================================================
// 🎬 开场 Loading 状态机 (利用 switch 配合 QTimer 轮转状态)
// =========================================================================
void mainwindow::nextLoadingStage() {
    switch (currentStage) {
    case 0: imageLabel->setPixmap(QPixmap(":/res/images/init.png")); fadeAnimation->setStartValue(0.0); fadeAnimation->setEndValue(1.0); fadeAnimation->start(); loadingTimer->start(2000); break;
    case 1: fadeAnimation->setStartValue(1.0); fadeAnimation->setEndValue(0.0); fadeAnimation->start(); loadingTimer->start(800); break;
    case 2: imageLabel->setPixmap(QPixmap(":/res/images/LogoWord.jpg")); fadeAnimation->setStartValue(0.0); fadeAnimation->setEndValue(1.0); fadeAnimation->start(); loadingTimer->start(2000); break;
    case 3: fadeAnimation->setStartValue(1.0); fadeAnimation->setEndValue(0.0); fadeAnimation->start(); loadingTimer->start(800); break;
    case 4: imageLabel->setPixmap(QPixmap(":/res/images/StartScreen.jpg")); fadeAnimation->setStartValue(0.0); fadeAnimation->setEndValue(1.0); fadeAnimation->start(); loadingTimer->start(1000); break;
    case 5: floorLabel->show(); rollAnimation->setStartValue(QRect(80, 430, 200, 225)); rollAnimation->setEndValue(QRect(80, 430, 820, 225)); rollAnimation->start(); loadingTimer->start(1600); break;
    case 6: loadingTimer->stop(); startButton->show(); startButton->raise(); break;
    }
    currentStage++;
}

void mainwindow::startGame() {
    imageLabel->hide(); floorLabel->hide(); startButton->hide();
    menuBgLabel->show(); btnAdventure->show(); btnMiniGames->show(); btnPuzzle->show(); btnPlay->show();
}

void mainwindow::startAdventure() {
    btnAdventure->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 防多点触控
    evilLaughSound->play();
    zombieHandLabel->show(); zombieHandLabel->raise();
    zombieHandMovie->start();
    QTimer::singleShot(3500, this, &mainwindow::transitionToGame);
}

// =========================================================================
// 🚀 场景切换与单例 UI 初始化 (解决返回主菜单时的内存泄漏和重复加载问题)
// =========================================================================
void mainwindow::transitionToGame() {
    zombieHandMovie->stop(); zombieHandLabel->hide(); menuBgLabel->hide();
    btnAdventure->hide(); btnMiniGames->hide(); btnPuzzle->hide(); btnPlay->hide();

    gameView->show();
    gameView->lower(); // 确保不遮挡上层的 UI 弹窗
    gameView->centerOn(500, 300);

    // ✅ 数据初始化：重置经济、击杀数据与网格占用，保证复玩时的状态纯净
    sunCount = 150;
    spawnedZombies = 0;
    killedZombies = 0;
    isGameEnding = false;
    isPaused = false;
    for (int i = 0; i < 5; ++i) { for (int j = 0; j < 9; ++j) { grassGrid[i][j] = 0; } }

    // 【单例模式思想】：静态 UI 组件在整个应用生命周期只 new 一次
    if (!isGameInitialized) {
        isGameInitialized = true;

        // 初始化 5 辆小推车实体
        for (int i = 0; i < 5; ++i) {
            lawnMowers[i] = new LawnMower(i);
            gameScene->addItem(lawnMowers[i]);
            lawnMowers[i]->setPos(130, 100 + i * 85);
            lawnMowers[i]->setZValue(800); // 极高的渲染层级
        }

        // 顶栏卡槽与铲子 UI 的构建
        shopBoard = new QLabel();
        shopBoard->setPixmap(QPixmap(":/res/images/Shop.png").scaled(520, 90, Qt::IgnoreAspectRatio));
        shopBoard->setGeometry(0, 0, 520, 90);

        sunLabel = new QLabel(shopBoard);
        sunLabel->setGeometry(15, 62, 55, 20);
        sunLabel->setAlignment(Qt::AlignCenter);
        sunLabel->setStyleSheet("QLabel { color: black; font-size: 16px; font-weight: bold; background: transparent; }");

        // 使用 Lambda 表达式封闭卡片创建的重复逻辑
        auto createCard = [this](int x, const QString& iconPath, const QString& costText, QPushButton*& btn, QLabel*& mask) {
            btn = new QPushButton(shopBoard);
            btn->setGeometry(x, 8, 50, 70);
            btn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; background: transparent; }");
            btn->setIcon(QIcon(iconPath));
            btn->setIconSize(QSize(40, 40));
            QLabel* costLbl = new QLabel(costText, btn);
            costLbl->setGeometry(5, 52, 30, 15);
            costLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            costLbl->setStyleSheet("color: black; font-size: 12px; font-weight: bold; background: transparent;");

            mask = new QLabel(btn);
            mask->setStyleSheet("background-color: rgba(0, 0, 0, 160);");
            mask->setGeometry(0, 0, 50, 70);
            mask->hide();
            };

        createCard(82, ":/res/images/SunFlower.png", "50", sunCardBtn, sunCardMask);
        createCard(135, ":/res/images/Peashooter.png", "100", peaCardBtn, peaCardMask);
        createCard(188, ":/res/images/WallNut.png", "50", wallnutCardBtn, wallnutCardMask);
        createCard(241, ":/res/images/CherryBomb.png", "150", cherryCardBtn, cherryCardMask);
        createCard(294, ":/res/images/Repeater.png", "200", repeaterCardBtn, repeaterCardMask);

        // 信号与槽：绑定卡牌点击事件与植物买卖逻辑
        connect(sunCardBtn, &QPushButton::clicked, [this]() { tryBuyCard(50, HoldingSunflower, ":/res/images/SunFlower.png"); });
        connect(peaCardBtn, &QPushButton::clicked, [this]() { tryBuyCard(100, HoldingPeashooter, ":/res/images/Peashooter.png"); });
        connect(wallnutCardBtn, &QPushButton::clicked, [this]() { tryBuyCard(50, HoldingWallNut, ":/res/images/WallNut.png"); });
        connect(cherryCardBtn, &QPushButton::clicked, [this]() { tryBuyCard(150, HoldingCherry, ":/res/images/CherryBomb.png"); });
        connect(repeaterCardBtn, &QPushButton::clicked, [this]() { tryBuyCard(200, HoldingRepeater, ":/res/images/Repeater.png"); });

        shovelBankBtn = new QPushButton();
        shovelBankBtn->setGeometry(0, 0, 70, 72);
        shovelBankBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/ShovelBank.png); border: none; background: transparent; }");
        shovelBankBtn->setIcon(QIcon(":/res/images/Shovel.png"));
        shovelBankBtn->setIconSize(QSize(50, 50));
        connect(shovelBankBtn, &QPushButton::clicked, [this]() {
            currentMouseState = HoldingShovel;
            gameView->viewport()->setCursor(QCursor(QPixmap(":/res/images/Shovel.png").scaled(50, 50)));
            });

        gameScene->addWidget(shopBoard)->setPos(120, 0);
        gameScene->addWidget(shovelBankBtn)->setPos(645, 0);

        menuButton = new QPushButton(gameView);
        menuButton->setGeometry(870, 5, 120, 35);
        menuButton->setStyleSheet("QPushButton { border-image: url(:/res/images/Button.png); background-color: transparent; }");
        menuButton->setIcon(QIcon(":/res/images/SelectorScreen_Options1.png"));
        menuButton->setIconSize(QSize(90, 25));
        menuButton->show();
        connect(menuButton, &QPushButton::clicked, this, &mainwindow::togglePause);

        waveProgressBar = new ProgressBar(gameView);
        waveProgressBar->move(780, 575);
        waveProgressBar->show();

        // 戴夫相关模型初始化
        daveItem = new QGraphicsPixmapItem();
        daveItem->setPos(-50, 70);
        daveItem->setZValue(2000);
        gameScene->addItem(daveItem);

        daveMovie = new QMovie(this);
        connect(daveMovie, &QMovie::frameChanged, [this]() {
            if (daveItem && daveMovie->isValid()) {
                QPixmap scaledPix = daveMovie->currentPixmap().scaledToWidth(350, Qt::SmoothTransformation);
                daveItem->setPixmap(scaledPix);
            }
            });

        bubbleItem = new QGraphicsPixmapItem(QPixmap(":/res/images/Store_SpeechBubble2.png").scaled(300, 180, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        bubbleItem->setPos(260, 20);
        bubbleItem->setZValue(2000);
        gameScene->addItem(bubbleItem);

        daveTextItem = new QGraphicsTextItem(bubbleItem);
        daveTextItem->setPos(40, 25);
        daveTextItem->setTextWidth(220);
        daveTextItem->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
        daveTextItem->setDefaultTextColor(Qt::black);
    }

    // 更新界面数据
    sunLabel->setText(QString::number(sunCount));
    waveProgressBar->setProgress(0, totalZombies);
    daveItem->hide();
    bubbleItem->hide();

    // 切换日间 BGM
    bgm->stop();
    bgm->setSource(QUrl("qrc:/res/sound/Daytime.wav"));
    bgm->setVolume(0.5f);

    // ====================================================================================
    // 游戏三大引擎绑定 (阳光生成、僵尸刷新、进度结算)
    // ====================================================================================

    // 天气阳光生成引擎
    if (!skySunTimer) skySunTimer = new QTimer(this);
    disconnect(skySunTimer, nullptr, nullptr, nullptr); // 防重入挂载
    connect(skySunTimer, &QTimer::timeout, [this]() {
        int randX = QRandomGenerator::global()->bounded(250, 1000);
        int targetY = QRandomGenerator::global()->bounded(200, 500);
        Sun* skySun = new Sun();
        connect(skySun, &Sun::collected, this, &mainwindow::addSun);
        gameScene->addItem(skySun);
        skySun->setZValue(100);
        skySun->startFall(randX, targetY);
        });

    // 僵尸生成引擎 (包含加权随机算法分配不同种类的僵尸)
    if (!zombieSpawnTimer) zombieSpawnTimer = new QTimer(this);
    disconnect(zombieSpawnTimer, nullptr, nullptr, nullptr);
    connect(zombieSpawnTimer, &QTimer::timeout, [this]() {
        if (spawnedZombies >= totalZombies) return; // 数量达到阈值则停止刷新

        int randRow = QRandomGenerator::global()->bounded(0, 5);
        int randType = QRandomGenerator::global()->bounded(100);

        // 概率分布：20%路障，15%铁桶，65%普通
        Zombie::ZombieType zType = Zombie::NormalZombie;
        if (randType < 20) zType = Zombie::ConeheadZombie;
        else if (randType < 35) zType = Zombie::BucketheadZombie;

        Zombie* zombie = new Zombie(randRow, zType);
        gameScene->addItem(zombie);
        connect(zombie, &Zombie::gameLost, this, &mainwindow::gameOver);

        // 计算僵尸初始 Y 坐标，确保其踩在对应的行轨道上
        int startY = 85;
        int cellH = 95;
        int placeX = 1000;
        int placeY = startY + randRow * cellH - 25;

        zombie->setPos(placeX, placeY);
        zombie->setZValue(randRow * 100 + 60); // 渲染层级防穿帮

        spawnedZombies++;
        });

    // 波次结算引擎
    if (!progressTimer) progressTimer = new QTimer(this);
    disconnect(progressTimer, nullptr, nullptr, nullptr);
    connect(progressTimer, &QTimer::timeout, this, &mainwindow::updateProgress);

    // 唤醒戴夫
    isDaveTalking = true;
    daveStep = 0;
    advanceDaveStateMachine();
}

// =========================================================================
// 🎬 戴夫有限状态机 (FSM)：严密控制对话流程，杜绝音频冲突与进场卡死
// =========================================================================
void mainwindow::advanceDaveStateMachine() {
    if (!isDaveTalking) return;

    if (daveItem && !daveItem->isVisible()) daveItem->show();
    if (daveSound->isPlaying()) daveSound->stop(); // 切除上一步未播完的音频
    if (daveMovie->isValid()) daveMovie->stop();

    daveStep++;

    // 状态流转
    if (daveStep == 1) {
        daveMovie->setFileName(":/res/images/other/CrazyDave/enter.gif");
        daveMovie->start();
        QTimer::singleShot(1000, this, [this]() { if (isDaveTalking && daveStep == 1) advanceDaveStateMachine(); });
    }
    else if (daveStep == 2) {
        showDaveDialog("邻居，好久不见！那些不长脑子的家伙又要来了！");
        daveMovie->setFileName(":/res/images/other/CrazyDave/smalltalk.gif");
        daveMovie->start();
        daveSound->setSource(QUrl("qrc:/res/sound/crazydavelong1.wav"));
        daveSound->play();
    }
    else if (daveStep == 3) {
        showDaveDialog("听说他们这次戴了路障和铁桶，装备非常精良！");
        daveMovie->setFileName(":/res/images/other/CrazyDave/blahblah.gif");
        daveMovie->start();
        daveSound->setSource(QUrl("qrc:/res/sound/crazydaveextralong1.wav"));
        daveSound->play();
    }
    else if (daveStep == 4) {
        showDaveDialog("你得保护好我的房子！准备好了吗？Wabi Wabbo!");
        daveMovie->setFileName(":/res/images/other/CrazyDave/crazy.gif");
        daveMovie->start();
        daveSound->setSource(QUrl("qrc:/res/sound/crazydavecrazy.wav"));
        daveSound->play();
    }
    else if (daveStep >= 5) {
        hideDaveDialog();
        daveMovie->setFileName(":/res/images/other/CrazyDave/leave.gif");
        daveMovie->start();

        // 戴夫离开后，移交控制权给“Ready Set Plant”动画
        QTimer::singleShot(1500, this, [this]() {
            daveItem->hide();
            isDaveTalking = false;
            startReadySetPlant();
            });
    }
}

void mainwindow::showDaveDialog(const QString& text) {
    if (bubbleItem && !bubbleItem->isVisible()) bubbleItem->show();
    if (daveTextItem) daveTextItem->setPlainText(text);
}

void mainwindow::hideDaveDialog() {
    if (bubbleItem) bubbleItem->hide();
}

// =========================================================================
// 🚀 战斗准备开场：Ready-Set-Plant
// =========================================================================
void mainwindow::startReadySetPlant() {
    QSoundEffect* rspSound = new QSoundEffect(this);
    rspSound->setSource(QUrl("qrc:/res/sound/readysetplant.wav"));
    rspSound->setVolume(0.8f);
    rspSound->play();

    QGraphicsPixmapItem* rspItem = new QGraphicsPixmapItem();
    rspItem->setPos(300, 200);
    rspItem->setZValue(2000);
    gameScene->addItem(rspItem);

    rspItem->setPixmap(QPixmap(":/res/images/StartReady.png").scaled(400, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QTimer::singleShot(600, this, [=]() {
        rspItem->setPixmap(QPixmap(":/res/images/StartSet.png").scaled(400, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        });

    QTimer::singleShot(1200, this, [=]() {
        rspItem->setPixmap(QPixmap(":/res/images/StartPlant.png").scaled(400, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        });

    // 动画结束，全面开启游戏引擎
    QTimer::singleShot(1800, this, [=]() {
        delete rspItem;
        rspSound->deleteLater();

        if (bgm) bgm->play();
        if (skySunTimer) skySunTimer->start(8000); // 8秒产生一个阳光
        if (zombieSpawnTimer) zombieSpawnTimer->start(4500); // 4.5秒刷一只僵尸
        if (progressTimer) progressTimer->start(500); // 进度条半秒轮询一次
        });
}

// ====================================================================================
// ✅ 【架构高光】：原汁原味的精准网格放置拦截体系 (O(1) 像素级算法)
// ====================================================================================
bool mainwindow::eventFilter(QObject* watched, QEvent* event)
{
    // 全局拦截鼠标点击事件
    if (watched == gameView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        // 如果戴夫在讲话，点击鼠标直接跳到下一句
        if (isDaveTalking && mouseEvent->button() == Qt::LeftButton) {
            advanceDaveStateMachine();
            return true;
        }

        // 处于暂停或结算时禁止交互
        if (isPaused || isGameEnding) return true;

        if (mouseEvent->button() == Qt::LeftButton && currentMouseState != None) {
            QPointF scenePos = gameView->mapToScene(mouseEvent->pos());

            // 🚀 【O(1) 空间映射算法】：将连续的像素坐标转换为离散的二维数组索引
            // 无需遍历，直接利用整除截断特性计算出行列
            int col = (scenePos.x() - 250) / 80;
            int row = (scenePos.y() - 85) / 95;

            // 1. 【种植逻辑】：目标地块必须为空（樱桃炸弹除外，它不需要格子占用）
            if (row >= 0 && row < 5 && col >= 0 && col < 9 && (grassGrid[row][col] == 0 || currentMouseState == HoldingCherry)) {

                if (currentMouseState == HoldingShovel) {
                    currentMouseState = None;
                    gameView->viewport()->setCursor(Qt::ArrowCursor);
                    return true; // 空地上用铲子无效，直接取消动作
                }

                // 修正为该网格的中心物理锚点坐标
                int placeX = 250 + col * 80 + 40;
                int placeY = 85 + row * 95 + 47;

                Plant* newPlant = nullptr;
                int cost = 0;

                // 利用工厂模式思想，根据玩家手持状态实例化对应植物，并挂载专属信号槽
                if (currentMouseState == HoldingSunflower) {
                    newPlant = new Sunflower(row, col); cost = 50;
                    connect(static_cast<Sunflower*>(newPlant), &Sunflower::sunProduced, this, [this](Sun* sun) {
                        gameScene->addItem(sun); sun->setZValue(100);
                        connect(sun, &Sun::collected, this, &mainwindow::addSun);
                        });
                }
                else if (currentMouseState == HoldingPeashooter) {
                    newPlant = new PeaShooter(row, col); cost = 100;
                    connect(static_cast<PeaShooter*>(newPlant), &PeaShooter::bulletFired, this, [this](PeaBullet* bullet) {
                        gameScene->addItem(bullet); bullet->setZValue(10); peaFireSound->play();
                        });
                }
                else if (currentMouseState == HoldingWallNut) {
                    newPlant = new WallNut(row, col); cost = 50;
                }
                else if (currentMouseState == HoldingRepeater) {
                    newPlant = new Repeater(row, col); cost = 200;
                    connect(static_cast<Repeater*>(newPlant), &Repeater::bulletFired, this, [this](PeaBullet* bullet) {
                        gameScene->addItem(bullet); bullet->setZValue(10); peaFireSound->play();
                        });
                }
                else if (currentMouseState == HoldingCherry) {
                    newPlant = new CherryBomb(row, col); cost = 150;
                }

                // 校验余额并种植
                if (newPlant && sunCount >= cost) {
                    sunCount -= cost;
                    sunLabel->setText(QString::number(sunCount));
                    gameScene->addItem(newPlant);

                    // 🌟 经典原版法则：Z-Value = 行号 * 100 + 基数，完美解决上下遮挡穿帮问题
                    newPlant->setZValue(row * 100 + 50);
                    newPlant->setPos(placeX, placeY);

                    // 占据该网格
                    if (currentMouseState != HoldingCherry) grassGrid[row][col] = 1;

                    // 触发对应卡牌的遮罩冷却动画
                    if (currentMouseState == HoldingSunflower) startCardCooldown(sunCardBtn, sunCardMask, 7500);
                    else if (currentMouseState == HoldingPeashooter) startCardCooldown(peaCardBtn, peaCardMask, 7500);
                    else if (currentMouseState == HoldingWallNut) startCardCooldown(wallnutCardBtn, wallnutCardMask, 30000);
                    else if (currentMouseState == HoldingCherry) startCardCooldown(cherryCardBtn, cherryCardMask, 30000);
                    else if (currentMouseState == HoldingRepeater) startCardCooldown(repeaterCardBtn, repeaterCardMask, 7500);

                    plantSound->play();
                }
                else {
                    delete newPlant; // 余额不足，安全释放已创建的内存对象
                    if (sunCount < cost) tryBuyCard(cost, currentMouseState, "");
                }

                // 恢复默认指针形态
                currentMouseState = None;
                gameView->viewport()->setCursor(Qt::ArrowCursor);
                return true;
            }
            // 2. 【铲除逻辑】：点击到已种植地块且手持铲子
            else if (row >= 0 && row < 5 && col >= 0 && col < 9 && grassGrid[row][col] == 1) {
                if (currentMouseState == HoldingShovel) {
                    int placeX = 250 + col * 80 + 40;
                    int placeY = 85 + row * 95 + 47;
                    // 反推坐标并在场景中获取该点的所有碰撞实体
                    QList<QGraphicsItem*> itemsAtClick = gameScene->items(QPointF(placeX, placeY));

                    for (QGraphicsItem* item : itemsAtClick) {
                        // 🔍 RTTI 检查：确保只铲除植物基类派生出的对象（过滤掉地上的阳光、子弹）
                        if (Plant* targetPlant = dynamic_cast<Plant*>(item)) {
                            targetPlant->die();      // 触发植物内部的销毁逻辑
                            grassGrid[row][col] = 0; // 释放该网格

                            QSoundEffect* digSound = new QSoundEffect();
                            digSound->setSource(QUrl("qrc:/res/sound/plant.wav"));
                            digSound->setVolume(0.8f);
                            digSound->play();
                            connect(digSound, &QSoundEffect::playingChanged, [digSound]() { if (!digSound->isPlaying()) digSound->deleteLater(); });

                            break;
                        }
                    }
                }
                currentMouseState = None;
                gameView->viewport()->setCursor(Qt::ArrowCursor);
                return true;
            }
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            // 右键取消手持状态
            currentMouseState = None;
            gameView->viewport()->setCursor(Qt::ArrowCursor);
        }
    }
    return QMainWindow::eventFilter(watched, event); // 继续向上抛出事件
}

// 响应键盘空格键进行快捷暂停
void mainwindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !isDaveTalking && !isGameEnding) {
        togglePause();
    }
    QMainWindow::keyPressEvent(event);
}

// ====================================================================================
// 🛑 【核心设计】：时空冻结控制（接管所有实体的内部行为而非暂停主线程）
// ====================================================================================
void mainwindow::togglePause() {
    isPaused = !isPaused;

    if (isPaused) {
        // 1. 关停全局生成引擎与音频
        if (zombieSpawnTimer && zombieSpawnTimer->isActive()) zombieSpawnTimer->stop();
        if (skySunTimer && skySunTimer->isActive()) skySunTimer->stop();
        if (progressTimer && progressTimer->isActive()) progressTimer->stop();
        if (bgm && bgm->isPlaying()) bgm->setMuted(true);

        // 2. 遍历多态场景图，利用 dynamic_cast 强转，向不同类别的实体发送暂停指令
        for (QGraphicsItem* item : gameScene->items()) {
            if (Zombie* z = dynamic_cast<Zombie*>(item)) {
                z->pauseBehavior();
            }
            else if (Plant* p = dynamic_cast<Plant*>(item)) {
                p->pauseBehavior(); // 完美暂停植物内部动图与发弹定时器
            }
            // 针对动画类的基类做普遍拦截（如冷却遮罩、阳光抛物线等）
            else if (QGraphicsObject* obj = dynamic_cast<QGraphicsObject*>(item)) {
                for (QPropertyAnimation* anim : obj->findChildren<QPropertyAnimation*>()) {
                    if (anim->state() == QAbstractAnimation::Running) anim->pause();
                }
                for (QMovie* m : obj->findChildren<QMovie*>()) m->setPaused(true);
            }
        }
        // 呼出遮罩 UI
        pauseWidget->show();
        pauseWidget->raise();
    }
    else {
        // 恢复所有实体的时钟与动画逻辑
        if (zombieSpawnTimer && !zombieSpawnTimer->isActive()) zombieSpawnTimer->start(4500);
        if (skySunTimer && !skySunTimer->isActive()) skySunTimer->start(8000);
        if (progressTimer && !progressTimer->isActive()) progressTimer->start(500);
        if (bgm) bgm->setMuted(false);

        for (QGraphicsItem* item : gameScene->items()) {
            if (Zombie* z = dynamic_cast<Zombie*>(item)) {
                z->resumeBehavior();
            }
            else if (Plant* p = dynamic_cast<Plant*>(item)) {
                p->resumeBehavior();
            }
            else if (QGraphicsObject* obj = dynamic_cast<QGraphicsObject*>(item)) {
                for (QPropertyAnimation* anim : obj->findChildren<QPropertyAnimation*>()) {
                    if (anim->state() == QAbstractAnimation::Paused) anim->resume();
                }
                for (QMovie* m : obj->findChildren<QMovie*>()) m->setPaused(false);
            }
        }
        pauseWidget->hide();
    }
}

// 统一控制阳光资源累加
void mainwindow::addSun(int amount) {
    sunCount += amount;
    if (sunLabel) sunLabel->setText(QString::number(sunCount));

    QSoundEffect* sunSound = new QSoundEffect();
    sunSound->setSource(QUrl("qrc:/res/sound/points.wav"));
    sunSound->setVolume(0.8f);
    sunSound->play();
    connect(sunSound, &QSoundEffect::playingChanged, [sunSound]() {
        if (!sunSound->isPlaying()) sunSound->deleteLater(); // 自动析构音效
        });
}

// 判断阳光能否购买卡牌，如果不能则播放错误提示音
void mainwindow::tryBuyCard(int cost, MouseState state, const QString& cursorImgPath) {
    if (sunCount >= cost) {
        currentMouseState = state;
        gameView->viewport()->setCursor(QCursor(QPixmap(cursorImgPath).scaled(50, 50)));
    }
    else {
        QSoundEffect* errorSound = new QSoundEffect(this);
        errorSound->setSource(QUrl("qrc:/res/sound/buzzer.wav"));
        errorSound->setVolume(0.8f);
        errorSound->play();
        connect(errorSound, &QSoundEffect::playingChanged, [errorSound]() {
            if (!errorSound->isPlaying()) errorSound->deleteLater();
            });
    }
}

// 利用 QPropertyAnimation 控制遮罩层的高低，实现卡牌冷却 UI
void mainwindow::startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs) {
    btn->setEnabled(false);
    mask->show();
    QPropertyAnimation* cdAnim = new QPropertyAnimation(mask, "geometry");
    cdAnim->setDuration(durationMs);
    cdAnim->setStartValue(QRect(0, 0, btn->width(), btn->height())); // 满遮罩
    cdAnim->setEndValue(QRect(0, btn->height(), btn->width(), 0));   // 遮罩消失
    connect(cdAnim, &QPropertyAnimation::finished, [btn, mask, cdAnim]() {
        mask->hide();
        btn->setEnabled(true);
        cdAnim->deleteLater();
        });
    cdAnim->start();
}

// 当僵尸突破防线（到达最左端）时调用
void mainwindow::gameOver(QGraphicsObject* winnerZombie) {
    if (isGameEnding) return;
    isGameEnding = true;

    // 剥夺生成引擎
    if (zombieSpawnTimer) zombieSpawnTimer->stop();
    if (skySunTimer) skySunTimer->stop();
    if (progressTimer) progressTimer->stop();
    if (bgm) bgm->stop();

    // 冻结除赢家僵尸外的一切事物，制造游戏结束特写感
    for (QGraphicsItem* item : gameScene->items()) {
        if (Zombie* nz = dynamic_cast<Zombie*>(item)) {
            if (nz == winnerZombie) continue;
            nz->pauseBehavior();
        }
        else if (Plant* p = dynamic_cast<Plant*>(item)) {
            p->pauseBehavior();
        }
        else if (QGraphicsObject* gObj = dynamic_cast<QGraphicsObject*>(item)) {
            for (QPropertyAnimation* anim : gObj->findChildren<QPropertyAnimation*>()) {
                if (anim->state() == QAbstractAnimation::Running) anim->pause();
            }
            for (QMovie* m : gObj->findChildren<QMovie*>()) m->setPaused(true);
        }
    }

    QSoundEffect* loseSound = new QSoundEffect(this);
    loseSound->setSource(QUrl("qrc:/res/sound/failure.wav"));
    loseSound->setVolume(1.0f);
    loseSound->play();

    gameOverLabel->raise();
    gameOverLabel->show();
}

// =========================================================================
// ✅ 【内存管理金牌操作】：完美清理与返回主菜单逻辑，杜绝资源泄漏
// =========================================================================
void mainwindow::returnToMainMenu() {
    if (zombieSpawnTimer) zombieSpawnTimer->stop();
    if (skySunTimer) skySunTimer->stop();
    if (progressTimer) progressTimer->stop();
    if (bgm) bgm->stop();

    isGameEnding = false;
    isPaused = false;
    if (pauseWidget) pauseWidget->hide();
    gameView->hide();

    // ⚠️ 极其关键的安全析构：只清理动态生成的战斗实体。
    // 绝对不能调用 scene->clear()，否则会导致挂载在 Scene 上的静态 UI（如卡槽、小推车）变成野指针！
    for (QGraphicsItem* item : gameScene->items()) {
        if (dynamic_cast<Zombie*>(item) || dynamic_cast<Plant*>(item) ||
            dynamic_cast<Sun*>(item) || dynamic_cast<PeaBullet*>(item)) {
            delete item;
        }
    }

    btnAdventure->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    menuBgLabel->show();
    btnAdventure->show();
    btnMiniGames->show();
    btnPuzzle->show();
    btnPlay->show();
}

void mainwindow::restartGame() {}

// =========================================================================
// 🏆 进度条与战役结算系统
// =========================================================================
void mainwindow::updateProgress() {
    int currentZombiesInScene = 0;
    for (auto item : gameScene->items()) {
        if (dynamic_cast<Zombie*>(item)) currentZombiesInScene++;
    }

    // 通过生成总数扣除存活数量来推演击杀数，安全且高效
    killedZombies = spawnedZombies - currentZombiesInScene;
    if (waveProgressBar) waveProgressBar->setProgress(killedZombies, totalZombies);

    // 🏆 判断通关条件：僵尸全部生成完毕，且场上已经没有任何存活的僵尸实体
    if (spawnedZombies >= totalZombies && currentZombiesInScene == 0 && !isGameEnding) {
        gameWon();
    }
}

// 战役胜利结算演出
void mainwindow::gameWon() {
    isGameEnding = true;
    if (zombieSpawnTimer) zombieSpawnTimer->stop();
    if (skySunTimer) skySunTimer->stop();
    if (progressTimer) progressTimer->stop();

    // 冻结全场植物，停止无意义的开火动作
    for (QGraphicsItem* item : gameScene->items()) {
        if (Plant* p = dynamic_cast<Plant*>(item)) {
            p->pauseBehavior();
        }
    }

    // 🏆 构造奖杯掉落的交互按钮
    QPushButton* trophyBtn = new QPushButton();
    trophyBtn->setIcon(QIcon(":/res/images/trophy.png"));
    trophyBtn->setIconSize(QSize(80, 80));
    trophyBtn->setStyleSheet("background: transparent; border: none;");

    // 将 QWidget 包装进 QGraphicsScene
    QGraphicsProxyWidget* trophyProxy = gameScene->addWidget(trophyBtn);
    trophyProxy->setPos(500, 0);
    trophyProxy->setZValue(9999);

    // 物理掉落动画
    QPropertyAnimation* dropAnim = new QPropertyAnimation(trophyProxy, "pos");
    dropAnim->setStartValue(QPointF(500, 0));
    dropAnim->setEndValue(QPointF(500, 300));
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);
    dropAnim->setDuration(1500);
    dropAnim->start(QAbstractAnimation::DeleteWhenStopped); // 播放后自动清理动画对象

    // 奖杯点击事件：进入最终结算
    connect(trophyBtn, &QPushButton::clicked, this, [=]() {
        trophyBtn->setEnabled(false); // 拿了奖杯就不许再点了！
        if (bgm) bgm->stop();

        QSoundEffect* winSound = new QSoundEffect(this);
        winSound->setSource(QUrl("qrc:/res/sound/winmusic.wav"));
        winSound->setVolume(1.0f);
        winSound->play();

        // 🏆 动画组合：奖杯放大
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(trophyProxy, "scale");
        scaleAnim->setStartValue(1.0);
        scaleAnim->setEndValue(2.5);
        scaleAnim->setDuration(2000);
        scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        // 🏆 动画组合：奖杯移至中央
        QPropertyAnimation* moveAnim = new QPropertyAnimation(trophyProxy, "pos");
        moveAnim->setStartValue(trophyProxy->pos());
        moveAnim->setEndValue(QPointF(380, 180));
        moveAnim->setDuration(2000);
        moveAnim->start(QAbstractAnimation::DeleteWhenStopped);

        // 🌟 圣光降临：白屏淡出过场遮罩
        QWidget* whiteScreen = new QWidget(gameView);
        whiteScreen->setGeometry(0, 0, 1000, 600);
        whiteScreen->setStyleSheet("background-color: white;");
        whiteScreen->show();

        QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect();
        eff->setOpacity(0.0);
        whiteScreen->setGraphicsEffect(eff);

        QPropertyAnimation* fadeAnim = new QPropertyAnimation(eff, "opacity");
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setDuration(4000);

        connect(fadeAnim, &QPropertyAnimation::finished, this, [=]() {
            trophyProxy->deleteLater();
            whiteScreen->deleteLater();
            showVictoryScreen();
            });
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
}

// 展示最后的制作人员/结语名单
void mainwindow::showVictoryScreen() {
    QWidget* victoryWidget = new QWidget(gameView);
    victoryWidget->setGeometry(0, 0, 1000, 600);

    QLabel* bg = new QLabel(victoryWidget);
    bg->setPixmap(QPixmap(":/res/images/Challenge_Background.jpg").scaled(1000, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    bg->setGeometry(0, 0, 1000, 600);

    QLabel* textLabel = new QLabel(victoryWidget);
    textLabel->setText("我们的小组大作业到此不那么圆满的展示结束了，感谢每一位成员的辛苦付出。");
    textLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent; font-family: 'Microsoft YaHei';");
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);
    textLabel->setGeometry(100, 200, 800, 100);

    QPushButton* closeBtn = new QPushButton(victoryWidget);
    closeBtn->setGeometry(870, 15, 100, 45);
    closeBtn->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/Almanac_CloseButton.png); } "
        "QPushButton:hover { border-image: url(:/res/images/Almanac_CloseButtonHighlight.png); }"
    );
    // 绑定关闭事件安全返回主菜单
    connect(closeBtn, &QPushButton::clicked, this, [=]() {
        victoryWidget->deleteLater();
        returnToMainMenu();
        });

    victoryWidget->show();
}