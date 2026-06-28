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
    : QMainWindow(parent), ui(new Ui::MainWindowClass), currentStage(0)
{
    ui->setupUi(this);
    // ✅ 【神级修复】：让 Qt 默认的中心画布变透明，绝不阻挡鼠标点击！
    ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    // 【消灭横线】隐藏 QMainWindow 默认自带的菜单栏和状态栏
    this->menuBar()->hide();
    this->statusBar()->hide();

    // 1. 设置标准的原生分辨率 (1000x600)
    this->setFixedSize(1000, 600);
    this->setStyleSheet("QMainWindow { background-color: black; }");

    // ====================================================================================
    // 1. 【逻辑大舞台】：严格使用原图物理尺寸 (1400x600)
    // ====================================================================================
    gameScene = new QGraphicsScene(this);
    gameScene->setSceneRect(0, 0, 1400, 600); // 绝对真理坐标系

    // 2. 原图直接贴入，绝对不使用 .scaled()，节省大量内存！
    combatBgItem = new QGraphicsPixmapItem(QPixmap(":/res/images/Background.jpg"));
    combatBgItem->setZValue(-10);
    gameScene->addItem(combatBgItem);
    // ====================================================================================
    // 3. 【玩家摄像机】：尺寸必须和主窗口一致 (1000x600)
    // ====================================================================================
    gameView = new QGraphicsView(gameScene, this);
    gameView->setGeometry(0, 0, 1000, 600);
    gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gameView->setRenderHint(QPainter::Antialiasing);
    gameView->setStyleSheet("background: transparent; border: none;");
    gameView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // =========================================================
    // 🚀 【新增】：Qt 底层渲染性能优化大招！
    // =========================================================
    // 1. 开启背景缓存：1400x600的巨大草坪不再每帧重绘，直接存进显存！
    gameView->setCacheMode(QGraphicsView::CacheBackground);
    // 2. 最小化重绘区域：只有僵尸/子弹走过的几十个像素才刷新，拒绝全屏重绘！
    gameView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    gameView->hide();

    // 安装事件拦截器
    gameView->viewport()->installEventFilter(this);

    // ====================================================================================
    // 【开场动画 UI】：全面回归原图直出，不再拉伸！
    // ====================================================================================
    imageLabel = new QLabel(this);
    imageLabel->setGeometry(0, 0, 1000, 600); // 贴合新窗口
    imageLabel->setAlignment(Qt::AlignCenter); // ✅ 核心：原图多大就多大，自动居中显示！

    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    imageLabel->setGraphicsEffect(opacityEffect);

    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(800);

    floorLabel = new QLabel(this);
    // ✅ 原图直出，去掉所有的 scaled
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
    else {
        qDebug() << "!!!!! 严重错误：字体加载失败，请检查 qrc 路径 !!!!!";
    }

    startButton = new QPushButton(QStringLiteral("点击进入游戏"), this);
    // ✅ 适配 1000x600：X居中(1000-300)/2=350，Y放在偏下部
    startButton->setGeometry(350, 545, 300, 55);
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
    connect(startButton, &QPushButton::clicked, this, &mainwindow::startGame);

    // ---------------- 【构建主菜单界面】原图直出 ----------------
    menuBgLabel = new QLabel(this);
    menuBgLabel->setGeometry(0, 0, 1000, 600);
    // ✅ 原图直出，如果原图是800x600，居中显示
    menuBgLabel->setPixmap(QPixmap(":/res/images/Surface.png"));
    menuBgLabel->setAlignment(Qt::AlignCenter);
    menuBgLabel->hide();

    // ✅ 按钮尺寸和坐标回调到未拉伸前的原生比例，完美对齐 Surface.png
    btnAdventure = new QPushButton(this);
    btnAdventure->setGeometry(510, 80, 330, 110);
    btnAdventure->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/mx.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/mx1.png); }"
    );
    btnAdventure->hide();
    connect(btnAdventure, &QPushButton::clicked, this, &mainwindow::startAdventure);

    btnMiniGames = new QPushButton(this);
    btnMiniGames->setGeometry(510, 190, 300, 100);
    btnMiniGames->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/mini.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/mini1.png); }"
    );
    btnMiniGames->hide();

    btnPuzzle = new QPushButton(this);
    btnPuzzle->setGeometry(510, 290, 280, 90);
    btnPuzzle->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/yizi.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/yizi1.png); }"
    );
    btnPuzzle->hide();

    btnPlay = new QPushButton(this);
    btnPlay->setGeometry(510, 380, 270, 85);
    btnPlay->setStyleSheet(
        "QPushButton { border-image: url(:/res/images/play.png); border: none; }"
        "QPushButton:hover { border-image: url(:/res/images/play1.png); }"
    );
    btnPlay->hide();

    // ---------------- 【构建小推车】 ----------------
    for (int i = 0; i < 5; ++i) {
        lawnMowers[i] = new LawnMower(i);
    }

    // ---------------- 【构建顶部植物商店】 (终极父子绑定版) ----------------
    shopBoard = new QLabel(this);
    // 尺寸继续优化，回归经典长条比例
    shopBoard->setPixmap(QPixmap(":/res/images/Shop.png").scaled(520, 90, Qt::IgnoreAspectRatio));
    shopBoard->setGeometry(120, 0, 520, 90);
    shopBoard->hide();

    // ✅【关键】：把 parent 设为 shopBoard！以后坐标全都是相对木牌的内部坐标！
    sunLabel = new QLabel(shopBoard);
    sunLabel->setGeometry(15, 62, 55, 20); // 乖乖待在阳光图标的正下方
    sunLabel->setText(QString::number(sunCount));
    sunLabel->setAlignment(Qt::AlignCenter);
    // 强制透明背景，防止出现白底
    sunLabel->setStyleSheet("QLabel { color: black; font-size: 16px; font-weight: bold; background: transparent; }");

    sunCardBtn = new QPushButton(shopBoard); // 设为子控件
    sunCardBtn->setGeometry(82, 8, 50, 70);  // 完美卡进第一个槽位
    sunCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; background: transparent; }");
    sunCardBtn->setIcon(QIcon(":/res/images/SunFlower.png"));
    sunCardBtn->setIconSize(QSize(40, 40));

    peaCardBtn = new QPushButton(shopBoard); // 设为子控件
    peaCardBtn->setGeometry(135, 8, 50, 70); // 完美卡进第二个槽位
    peaCardBtn->setStyleSheet("QPushButton { border-image: url(:/res/images/Card.png); border: none; background: transparent; }");
    peaCardBtn->setIcon(QIcon(":/res/images/Peashooter.png"));
    peaCardBtn->setIconSize(QSize(40, 40));

    // 💰 卡片上的阳光金额显示
   // 💰 卡片上的阳光金额显示
    QLabel* sunCostTxt = new QLabel("50", sunCardBtn);
    // ✅ X坐标从 25 左移到 5。给宽度 25。
    sunCostTxt->setGeometry(5, 52, 25, 15);
    // ✅ 极其关键：设置文字靠右对齐！这样数字始终会紧贴着右边的阳光图标，不会越界
    sunCostTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sunCostTxt->setStyleSheet("color: black; font-size: 12px; font-weight: bold; background: transparent;");

    QLabel* peaCostTxt = new QLabel("100", peaCardBtn);
    // ✅ 三位数稍微给宽一点点 (宽度 30)，同样从 X=5 开始
    peaCostTxt->setGeometry(5, 52, 30, 15);
    peaCostTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    peaCostTxt->setStyleSheet("color: black; font-size: 12px; font-weight: bold; background: transparent;");

    // ⏳ 卡片的半透明 CD 遮罩
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
    plantSound->setSource(QUrl("qrc:/res/sound/plant.wav")); // 或 plant2.wav
    plantSound->setVolume(0.8f);

    // =========================================================
    // 🧠 【终极结算】：组装“僵尸吃掉了你的脑子”界面
    // =========================================================
    gameOverLabel = new QLabel(this);
    gameOverLabel->setGeometry(0, 0, 1000, 600);
    // 加上一层半透明的黑色遮罩，让背后的草坪变暗，突出中心的吃脑子图
    gameOverLabel->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    gameOverLabel->setAlignment(Qt::AlignCenter); // 图片完美居中

    // 贴上图，如果图片太大就稍微缩放一下，比如宽 600
    gameOverLabel->setPixmap(QPixmap(":/res/images/ZombiesWon.png").scaledToWidth(600, Qt::SmoothTransformation));
    gameOverLabel->hide(); // 开局必须藏好
}

mainwindow::~mainwindow()
{
    delete ui;
}

// ====================================================================
// 开场动画状态机：全部改为原图直出！
// ====================================================================
void mainwindow::nextLoadingStage()
{
    switch (currentStage) {
    case 0:
        imageLabel->setPixmap(QPixmap(":/res/images/init.png")); // ✅ 原图直出
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
        imageLabel->setPixmap(QPixmap(":/res/images/LogoWord.jpg")); // ✅ 原图直出
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
        imageLabel->setPixmap(QPixmap(":/res/images/StartScreen.jpg")); // 显示主背景
        fadeAnimation->setStartValue(0.0);
        fadeAnimation->setEndValue(1.0);
        fadeAnimation->start();
        loadingTimer->start(1000); // 留 1 秒让背景淡入
        break;

    case 5:
        // ✅ 恢复原配的土层地板显示
        floorLabel->show();

        // ✅ 恢复地板从左向右展开的滚轴动画
        rollAnimation->setStartValue(QRect(80, 430, 200, 225));
        rollAnimation->setEndValue(QRect(80, 430, 820, 225));
        rollAnimation->start();

        // 留出 1.6 秒让它滚完
        loadingTimer->start(1600);
        break;

    case 6:
        loadingTimer->stop();
        // ✅ 滚完之后，弹出开始按钮！
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

void mainwindow::startAdventure()
{
    menuBgLabel->hide();
    btnAdventure->hide();
    btnMiniGames->hide();
    btnPuzzle->hide();
    btnPlay->hide();

    gameView->show();
    gameView->lower();
    // ✅ 【神级修复】：强行把摄像机的中心点按在 X=500, Y=300 的位置！
    // 这会让 1000x600 的镜头死死锁在 1400x600 背景图的最左侧，完美露出房子全貌！
    gameView->centerOn(500, 300);

    for (int i = 0; i < 5; ++i) {
        gameScene->addItem(lawnMowers[i]);
        lawnMowers[i]->setPos(130, 100 + i * 85);
        lawnMowers[i]->setZValue(800);
    }

    auto shopProxy = gameScene->addWidget(shopBoard);
    shopProxy->setPos(120, 0);   // ✅ 匹配新坐标
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

        qDebug() << "【天气系统】天上掉下了一颗阳光！落点 Y:" << targetY;
        });
    skySunTimer->start(8000);

    // ==========================================================
    // 🎵 【音乐系统】：主菜单切歌到战斗 BGM
    // ==========================================================
    bgm->stop(); // 先停掉 Grazy.wav
    bgm->setSource(QUrl("qrc:/res/sound/Daytime.wav")); // 换上白天的磁带
    bgm->setVolume(0.5f);
    bgm->play(); // 重新开始播放白天战斗曲！

    // ✅ 新代码：直接使用成员变量，让大门遥控器握在整个类手里！
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

        qDebug() << "【生成系统】一只普通僵尸从第" << randRow << "行摇摇晃晃地走来了！";
        });
    zombieSpawnTimer->start(3000);
}

// =========================================================
// 🎯 【核心操作层：鼠标事件拦截与种植逻辑】
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

                            // ✅ 性能拉满：直接播放预加载好的声音，没有任何内存分配开销！
                            peaFireSound->play();
                            });

                        sunCount -= 100;
                        startCardCooldown(peaCardBtn, peaCardMask, 7500);
                    }

                    sunLabel->setText(QString::number(sunCount));
                    grassGrid[row][col] = 1;

                    // =========================================================
                    // 🎵 【新增】：种下植物的泥土声！
                    // =========================================================
                    QSoundEffect* plantSound = new QSoundEffect();
                    // 这里可以用 plant.wav 或者 plant2.wav
                    plantSound->play();

                    currentMouseState = None;
                    gameView->viewport()->setCursor(Qt::ArrowCursor);
                    return true;

                    currentMouseState = None;
                    gameView->viewport()->setCursor(Qt::ArrowCursor);
                    return true;

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
// ☠️ 【战败清算】：僵尸进屋，游戏结束！
// =========================================================
void mainwindow::gameOver(QGraphicsObject* winnerZombie)
{
    // 1. 防重复触发锁
    if (isGameEnding) return;
    isGameEnding = true;

    // 2. 终止全自动刷怪流水线，大门关闭，不再出兵！
    if (zombieSpawnTimer && zombieSpawnTimer->isActive()) {
        zombieSpawnTimer->stop();
    }

    // 3. 关掉白天战斗 BGM
    if (bgm) bgm->stop();

    // 4. 🔥【降维打击：时空冻结】遍历场景内所有物体，实行绝对静止
    QList<QGraphicsItem*> allItems = gameScene->items();
    for (QGraphicsItem* item : allItems) {

        // A. 处理其他僵尸
        Zombie* nz = dynamic_cast<Zombie*>(item);
        if (nz) {
            // 如果是获胜进屋的那只僵尸，放过它，让它继续走/继续啃咬
            if (nz == winnerZombie) {
                continue;
            }
            // 其他僵尸全部立正、动画冻结
            nz->pauseBehavior();
            continue;
        }

        // B. 处理植物及其他动态场景项
        // 如果你的植物继承自 QGraphicsObject 且内部有控制攻击/生产的子 QTimer
        QGraphicsObject* gObj = dynamic_cast<QGraphicsObject*>(item);
        if (gObj) {
            // 强行扒掉该物体身上所有子定时器的电源（向日葵、豌豆瞬间罢工）
            for (QObject* child : gObj->children()) {
                QTimer* childTimer = qobject_cast<QTimer*>(child);
                if (childTimer) {
                    childTimer->stop();
                }
            }

            // 如果植物类里有 QMovie 动画，也可以在这里通过转换为具体植物类型进行 stop()
        }
    }

    // 5. 播放全新小写命名的原版战败音乐
    QSoundEffect* loseSound = new QSoundEffect(this);
    loseSound->setSource(QUrl("qrc:/res/sound/losemusic.wav"));
    loseSound->setVolume(1.0f);
    loseSound->play();

    // 6. 升起战败界面
    gameOverLabel->raise();
    gameOverLabel->show();
}