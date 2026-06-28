#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

#include <QTimer>
#include <QLabel>
#include <QSoundEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QMouseEvent>

// 【新增】引入图形视图框架头文件
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>

class LawnMower;

class mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    void addSun(int amount);
    void gameOver(QGraphicsObject* winnerZombie); // ✅ 接收胜利者指针
    QSoundEffect* peaFireSound;
protected:
    // 【新增这行新架构代码】：事件过滤器
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void nextLoadingStage();
    void startGame();
    void startAdventure();
    
   
private:
    Ui::MainWindowClass* ui;
    bool isGameEnding = false; // ✅ 游戏结束锁
    // 顺便确保你的 zombieSpawnTimer 声明为了类成员变量，方便在这里关闭它
    QTimer* zombieSpawnTimer;

    // ==========================================
    // 【核心重构：图形视图框架代替 combatBgLabel】
    // ==========================================
    QGraphicsScene* gameScene;       // 逻辑大舞台 (1600x900)
    QGraphicsView* gameView;         // 玩家摄像机视口 (1200x900)
    QGraphicsPixmapItem* combatBgItem; // 战斗背景图形项

    // ==========================================
    // 现有 UI 与逻辑组件 (保持不变)
    // ==========================================
    QTimer* loadingTimer;
    QLabel* imageLabel;
    QSoundEffect* bgm;
    int currentStage;

    QGraphicsOpacityEffect* opacityEffect;
    QPropertyAnimation* fadeAnimation;

    QLabel* floorLabel;
    QPropertyAnimation* rollAnimation;
    QPushButton* startButton;

    QLabel* menuBgLabel;
    QPushButton* btnAdventure;
    QPushButton* btnMiniGames;
    QPushButton* btnPuzzle;
    QPushButton* btnPlay;

    QLabel* shopBoard;
    QPushButton* sunCardBtn;
    QPushButton* peaCardBtn;

    enum MouseState { None, HoldingSunflower, HoldingPeashooter };
    MouseState currentMouseState = None;

    int sunCount = 50;
    QLabel* sunLabel;

    // ✅ 【新增】：卡片的 CD 遮罩
    QLabel* sunCardMask;
    QLabel* peaCardMask;

    // ✅ 【新增】：通用的触发冷却动画函数
    void startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs);
    void tryBuyCard(int cost, MouseState state, const QString& cursorImgPath);

    LawnMower* lawnMowers[5];
    int grassGrid[5][9];

    QLabel* gameOverLabel;

    QSoundEffect* plantSound;
   
};