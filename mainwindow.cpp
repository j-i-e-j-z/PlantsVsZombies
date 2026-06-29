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
#include <QGraphicsProxyWidget> 
#include <QSoundEffect>
#include "lawnmower.h"
#include "sunflower.h"
#include "peashooter.h"
#include "peabullet.h"
#include "zombie.h" 
#include "sun.h"            

mainwindow::mainwindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), currentStage(0), zombieSpawnTimer(nullptr)
{
    ui->setupUi(this);

    // ✅ 初始化网格，防止野指针报错
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) {
            grassGrid[i][j] = 0;
        }
    }

    // ✅ 【神级修复】：让 Qt 默认的中心画布变透明，绝不阻挡鼠标点击！
    ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->menuBar()->hide();
    this->statusBar()->hide();

    this->setFixedSize(1000, 600);
    this->setStyleSheet("QMainWindow { background-color: black; }");

    // ====================================================================================
    // 1. 【逻辑大舞台】：严格使用原图物理尺寸 (1400x600)
    // ====================================================================================
    gameScene = new QGraphicsScene(this);
    gameScene->setSceneRect(0, 0, 1400, 600);

    combatBgItem = new QGraphicsPixmapItem(QPixmap(":/res/images/Background.jpg"));
    combatBgItem->setZValue(-10);
    gameScene->addItem(combatBgItem);

    // ====================================================================================
    // 2. 【玩家摄像机】：性能优化拉满
    // ====================================================================================
    gameView = new QGraphicsView(gameScene, this);
    gameView->setGeometry(0, 0, 1000, 600);
    gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setRenderHint(QPainter::Antialiasing);
    gameView->setStyleSheet("background: transparent; border: none;");
    gameView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    gameView->setCacheMode(QGraphicsView::CacheBackground);
    gameView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    gameView->hide();
    gameView->viewport()->installEventFilter(this);

    // ====================================================================================
    // 3. 【开场动画 UI】
    // ====================================================================================
    imageLabel = new QLabel(this);
    imageLabel->setGeometry(0, 0, 1000, 600);
    imageLabel->setAlignment(Qt::AlignCenter);

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

    int fontId = QFontDatabase::addApplicationFont(":/res/font/pvz_btn.ttf");
    QString pvzFontFamily = "Arial";
    if (fontId != -1) {
        pvzFontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    }

    startButton = new QPushButton(QStringLiteral("点击进入游戏"), this);
    startButton->setGeometry(350, 545, 300, 55);
    QString btnStyle = QString(
        "QPushButton { font-family: '%1'; color: white; font-size: 48px; font-weight: bold; background: transparent; border: none; }"
        "QPushButton:hover { color: #84cc16; }"
    ).arg(pvzFontFamily);
    startButton->setStyleSheet(btnStyle);
    startButton->hide();
    connect(startButton, &QPushButton::clicked, this, &mainwindow::startGame);

    // ---------------- 【构建主菜单界面】 ----------------
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

    // ---------------- 【构建小推车】 ----------------
    for (int i = 0; i < 5; ++i) {
        lawnMowers[i] = new LawnMower(i);
    }

    // ---------------- 【构建顶部植物商店】 ----------------
    shopBoard = new QLabel(this);
    shopBoard->setPixmap(QPixmap(":/res/images/Shop.png").scaled(520, 90, Qt::IgnoreAspectRatio));
    shopBoard->setGeometry(120, 0, 520, 90);
    shopBoard->hide();

    sunLabel = new QLabel(shopBoard);
    sunLabel->setGeometry(15, 62, 55, 20);
    sunLabel->setText(QString::number(sunCount));
    sunLabel->setAlignment(Qt::AlignCenter);
    sunLabel->setStyleSheet("QLabel { color: black; font-size: 16px; font-weight: bold; background: transparent; }");

    sunCardBtn = new QPushButton(shopBoard);
    sunCardBtn->setGeometry(82, 8, 50, 70);
    sunCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; background: transparent; }");
    sunCardBtn->setIcon(QIcon(":/res/images/SunFlower.png"));
    sunCardBtn->setIconSize(QSize(40, 40));

    peaCardBtn = new QPushButton(shopBoard);
    peaCardBtn->setGeometry(135, 8, 50, 70);
    peaCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; background: transparent; }");
    peaCardBtn->setIcon(QIcon(":/res/images/Peashooter.png"));
    peaCardBtn->setIconSize(QSize(40, 40));

    QLabel* sunCostTxt = new QLabel("50", sunCardBtn);
    sunCostTxt->setGeometry(5, 52, 25, 15);
    sunCostTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sunCostTxt->setStyleSheet("color: black; font-size: 12px; font-weight: bold; background: transparent;");

    QLabel* peaCostTxt = new QLabel("100", peaCardBtn);
    peaCostTxt->setGeometry(5, 52, 30, 15);
    peaCostTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    peaCostTxt->setStyleSheet("color: black; font-size: 12px; font-weight: bold; background: transparent;");

    sunCardMask = new QLabel(sunCardBtn);
    sunCardMask->setStyleSheet("background-color: rgba(0, 0, 0, 160);");
    sunCardMask->setGeometry(0, 0, 50, 70);
    sunCardMask->hide();

    peaCardMask = new QLabel(peaCardBtn);
    peaCardMask->setStyleSheet("background-color: rgba(0, 0, 0, 160);");
    peaCardMask->setGeometry(0, 0, 50, 70);
    peaCardMask->hide();

    connect(sunCardBtn, &QPushButton::clicked, [this]() {
        tryBuyCard(50, HoldingSunflower, ":/res/images/SunFlower.png");
        });

    connect(peaCardBtn, &QPushButton::clicked, [this]() {
        tryBuyCard(100, HoldingPeashooter, ":/res/images/Peashooter.png");
        });

    bgm = new QSoundEffect(this);
    bgm->setSource(QUrl("qrc:/res/sound/Grazy.wav"));
    bgm->setLoopCount(QSoundEffect::Infinite);
    bgm->setVolume(0.6f);
    bgm->play();

    loadingTimer = new QTimer(this);
    connect(loadingTimer, &QTimer::timeout, this, &mainwindow::nextLoadingStage);
    QTimer::singleShot(500, this, &mainwindow::nextLoadingStage);

    peaFireSound = new QSoundEffect(this);
    peaFireSound->setSource(QUrl("qrc:/res/sound/throw.wav"));
    peaFireSound->setVolume(0.5f);

    plantSound = new QSoundEffect(this);
    plantSound->setSource(QUrl("qrc:/res/sound/plant.wav"));
    plantSound->setVolume(0.8f);

    // =========================================================
    // 🧠 【终极结算 UI】
    // =========================================================
    gameOverLabel = new QLabel(this);
    gameOverLabel->setGeometry(0, 0, 1000, 600);
    gameOverLabel->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->setPixmap(QPixmap(":/res/images/ZombiesWon.png").scaledToWidth(600, Qt::SmoothTransformation));
    gameOverLabel->hide();

    // =========================================================
    // 🎬 【新增：过场动画组件初始化】
    // =========================================================
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

    // ✅ 新增：控制僵尸手 GIF 只播放一次（停在最后一帧）
    connect(zombieHandMovie, &QMovie::frameChanged, [this]() {
        // 如果当前播放到了最后一帧
        if (zombieHandMovie->currentFrameNumber() == (zombieHandMovie->frameCount() - 1)) {
            zombieHandMovie->stop(); // 强行刹车，保持定格在举着手的状态！
        }
        });
}

mainwindow::~mainwindow()
{
    delete ui;
}

// ====================================================================
// 开场动画状态机
// ====================================================================
void mainwindow::nextLoadingStage()
{
    switch (currentStage) {
    case 0:
        imageLabel->setPixmap(QPixmap(":/res/images/init.png"));
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
        imageLabel->setPixmap(QPixmap(":/res/images/LogoWord.jpg"));
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
        imageLabel->setPixmap(QPixmap(":/res/images/StartScreen.jpg"));
        fadeAnimation->setStartValue(0.0);
        fadeAnimation->setEndValue(1.0);
        fadeAnimation->start();
        loadingTimer->start(1000);
        break;
    case 5:
        floorLabel->show();
        rollAnimation->setStartValue(QRect(80, 430, 200, 225));
        rollAnimation->setEndValue(QRect(80, 430, 820, 225));
        rollAnimation->start();
        loadingTimer->start(1600);
        break;
    case 6:
        loadingTimer->stop();
        startButton->show();
        startButton->raise();
        break;
    }
    currentStage++;
}

void mainwindow::startGame()
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

// ====================================================================
// 🎬 拦截过渡：触发大笑与僵尸手动画
// ====================================================================
void mainwindow::startAdventure()
{
    // ✅ 修复 1：不要隐藏按钮（保留墓碑文字），而是让它们“变成透明幽灵”，
    // 这样既不会变灰影响观感，又能完美无视玩家的鼠标点击，防止手抖重复触发！
    btnAdventure->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    btnMiniGames->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    btnPuzzle->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    btnPlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    evilLaughSound->play();

    zombieHandLabel->show();
    zombieHandLabel->raise();
    zombieHandMovie->start();

    // 延时 3.5 秒后切换场景
    QTimer::singleShot(3500, this, &mainwindow::transitionToGame);
}

// ====================================================================
// 🎮 正式切入战斗场景
// ====================================================================
void mainwindow::transitionToGame()
{
    zombieHandMovie->stop();
    zombieHandLabel->hide();
    menuBgLabel->hide();

    // ✅ 修复 1 扫尾：真正进入游戏时，再把这些按钮彻底隐藏
    btnAdventure->hide();
    btnMiniGames->hide();
    btnPuzzle->hide();
    btnPlay->hide();

    // （顺手清理：把点击响应恢复回来，防止下次退回主菜单时点不动）
    btnAdventure->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    btnMiniGames->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    btnPuzzle->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    btnPlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    gameView->show();
    gameView->lower();
    gameView->centerOn(500, 300);

    for (int i = 0; i < 5; ++i) {
        gameScene->addItem(lawnMowers[i]);
        lawnMowers[i]->setPos(130, 100 + i * 85);
        lawnMowers[i]->setZValue(800);
    }

    auto shopProxy = gameScene->addWidget(shopBoard);
    shopProxy->setPos(120, 0);
    shopProxy->setZValue(1000);
    shopBoard->show();

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 9; ++j) {
            grassGrid[i][j] = 0;
        }
    }

    QTimer* skySunTimer = new QTimer(this);
    connect(skySunTimer, &QTimer::timeout, [this]() {
        int randX = QRandomGenerator::global()->bounded(250, 1000);
        int targetY = QRandomGenerator::global()->bounded(200, 500);

        Sun* skySun = new Sun();
        connect(skySun, &Sun::collected, this, &mainwindow::addSun);

        gameScene->addItem(skySun);
        skySun->setZValue(100);
        skySun->startFall(randX, targetY);
        });
    skySunTimer->start(8000);

    bgm->stop();
    bgm->setSource(QUrl("qrc:/res/sound/Daytime.wav"));
    bgm->setVolume(0.5f);
    bgm->play();

    zombieSpawnTimer = new QTimer(this);
    connect(zombieSpawnTimer, &QTimer::timeout, [this]() {
        int randRow = QRandomGenerator::global()->bounded(0, 5);

        Zombie* zombie = new Zombie(randRow);
        gameScene->addItem(zombie);
        connect(zombie, &Zombie::gameLost, this, &mainwindow::gameOver);

        int startY = 85;
        int cellH = 95;
        int placeX = 1000;
        int placeY = startY + randRow * cellH - 25;

        zombie->setPos(placeX, placeY);
        zombie->setZValue(60);
        });
    zombieSpawnTimer->start(3000);
}

// =========================================================
// 🎯 鼠标事件拦截与种植逻辑
// =========================================================
bool mainwindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == gameView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            if (currentMouseState != None) {
                QPointF scenePos = gameView->mapToScene(mouseEvent->pos());

                int col = (scenePos.x() - 250) / 80;
                int row = (scenePos.y() - 85) / 95;

                if (row >= 0 && row < 5 && col >= 0 && col < 9 && grassGrid[row][col] == 0) {

                    int placeX = 250 + col * 80 + 40;
                    int placeY = 85 + row * 95 + 47;

                    if (currentMouseState == HoldingSunflower) {
                        Sunflower* sunPlant = new Sunflower(row, col);
                        gameScene->addItem(sunPlant);
                        sunPlant->setZValue(50);
                        sunPlant->setPos(placeX, placeY);

                        connect(sunPlant, &Sunflower::sunProduced, this, [this](Sun* sun) {
                            gameScene->addItem(sun);
                            sun->setZValue(100);
                            connect(sun, &Sun::collected, this, &mainwindow::addSun);
                            });

                        sunCount -= 50;
                        startCardCooldown(sunCardBtn, sunCardMask, 7500);
                    }
                    else if (currentMouseState == HoldingPeashooter) {
                        PeaShooter* peaPlant = new PeaShooter(row, col);
                        gameScene->addItem(peaPlant);
                        peaPlant->setZValue(50);
                        peaPlant->setPos(placeX, placeY);

                        connect(peaPlant, &PeaShooter::bulletFired, this, [this](PeaBullet* bullet) {
                            gameScene->addItem(bullet);
                            bullet->setZValue(10);
                            peaFireSound->play();
                            });

                        sunCount -= 100;
                        startCardCooldown(peaCardBtn, peaCardMask, 7500);
                    }

                    sunLabel->setText(QString::number(sunCount));
                    grassGrid[row][col] = 1;

                    // 播放种植声并清理指针释放内存
                    QSoundEffect* tmpPlantSound = new QSoundEffect();
                    tmpPlantSound->setSource(QUrl("qrc:/res/sound/plant.wav"));
                    tmpPlantSound->setVolume(0.8f);
                    tmpPlantSound->play();
                    connect(tmpPlantSound, &QSoundEffect::playingChanged, [tmpPlantSound]() {
                        if (!tmpPlantSound->isPlaying()) tmpPlantSound->deleteLater();
                        });

                    currentMouseState = None;
                    gameView->viewport()->setCursor(Qt::ArrowCursor);
                    return true;
                }
                else {
                    qDebug() << "【战场系统】位置非法或已被占用！种植取消。";
                    currentMouseState = None;
                    gameView->viewport()->setCursor(Qt::ArrowCursor);
                }
            }
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            if (currentMouseState != None) {
                currentMouseState = None;
                gameView->viewport()->setCursor(Qt::ArrowCursor);
                qDebug() << "【商店系统】右键取消种植！";
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void mainwindow::addSun(int amount)
{
    sunCount += amount;
    if (sunLabel) {
        sunLabel->setText(QString::number(sunCount));
    }
    QSoundEffect* sunSound = new QSoundEffect();
    sunSound->setSource(QUrl("qrc:/res/sound/points.wav"));
    sunSound->setVolume(0.8f);
    sunSound->play();
    connect(sunSound, &QSoundEffect::playingChanged, [sunSound]() {
        if (!sunSound->isPlaying()) sunSound->deleteLater();
        });
}

void mainwindow::tryBuyCard(int cost, MouseState state, const QString& cursorImgPath)
{
    if (sunCount >= cost) {
        currentMouseState = state;
        gameView->viewport()->setCursor(QCursor(QPixmap(cursorImgPath).scaled(50, 50)));
        qDebug() << "【商店系统】成功拿起植物，准备种植！";
    }
    else {
        qDebug() << "【经济系统】阳光不足！需要：" << cost << " 当前只有：" << sunCount;
        QSoundEffect* errorSound = new QSoundEffect(this);
        errorSound->setSource(QUrl("qrc:/res/sound/buzzer.wav"));
        errorSound->setVolume(0.8f);
        errorSound->play();

        connect(errorSound, &QSoundEffect::playingChanged, [errorSound]() {
            if (!errorSound->isPlaying()) errorSound->deleteLater();
            });
    }
}

void mainwindow::startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs)
{
    btn->setEnabled(false);
    mask->show();

    QPropertyAnimation* cdAnim = new QPropertyAnimation(mask, "geometry");
    cdAnim->setDuration(durationMs);

    cdAnim->setStartValue(QRect(0, 0, btn->width(), btn->height()));
    cdAnim->setEndValue(QRect(0, btn->height(), btn->width(), 0));

    connect(cdAnim, &QPropertyAnimation::finished, [btn, mask, cdAnim]() {
        mask->hide();
        btn->setEnabled(true);
        cdAnim->deleteLater();
        });

    cdAnim->start();
}

// =========================================================
// ☠️ 战败清算：僵尸进屋，游戏结束！
// =========================================================
void mainwindow::gameOver(QGraphicsObject* winnerZombie)
{
    if (isGameEnding) return;
    isGameEnding = true;

    if (zombieSpawnTimer && zombieSpawnTimer->isActive()) {
        zombieSpawnTimer->stop();
    }

    if (bgm) bgm->stop();

    QList<QGraphicsItem*> allItems = gameScene->items();
    for (QGraphicsItem* item : allItems) {
        Zombie* nz = dynamic_cast<Zombie*>(item);
        if (nz) {
            if (nz == winnerZombie) continue;
            nz->pauseBehavior();
            continue;
        }

        QGraphicsObject* gObj = dynamic_cast<QGraphicsObject*>(item);
        if (gObj) {
            for (QObject* child : gObj->children()) {
                QTimer* childTimer = qobject_cast<QTimer*>(child);
                if (childTimer) {
                    childTimer->stop();
                }
            }
        }
    }

    // 播放最新的战败音效
    QSoundEffect* loseSound = new QSoundEffect(this);
    loseSound->setSource(QUrl("qrc:/res/sound/failure.wav"));
    loseSound->setVolume(1.0f);
    loseSound->play();

    gameOverLabel->raise();
    gameOverLabel->show();
}