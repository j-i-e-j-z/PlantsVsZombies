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

// 引入图形视图框架头文件
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include <QMovie>

class LawnMower;

class mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    mainwindow(QWidget* parent = nullptr);
    ~mainwindow();
    void addSun(int amount);
    void gameOver(QGraphicsObject* winnerZombie);
    QSoundEffect* peaFireSound;

protected:
    // 事件过滤器
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void nextLoadingStage();
    void startGame();
    void startAdventure();
    void transitionToGame(); // 🎬 负责过场动画结束后的正式切图

private:
    Ui::MainWindowClass* ui;
    bool isGameEnding = false;
    QTimer* zombieSpawnTimer;

    // ==========================================
    // 图形视图框架代替 combatBgLabel
    // ==========================================
    QGraphicsScene* gameScene;
    QGraphicsView* gameView;
    QGraphicsPixmapItem* combatBgItem;

    // ==========================================
    // 现有 UI 与逻辑组件
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
    QPushButton* wallnutCardBtn; // ✅ 新增坚果墙卡片按钮

    QPushButton* shovelBankBtn;  // ✅ 新增：铲子底座按钮


    enum MouseState { None, HoldingSunflower, HoldingPeashooter, HoldingWallNut, HoldingShovel }; // ✅ 新增 HoldingShovel
    MouseState currentMouseState = None;

    int sunCount = 50;
    QLabel* sunLabel;

    QLabel* sunCardMask;
    QLabel* peaCardMask;
    QLabel* wallnutCardMask;     // ✅ 新增坚果墙 CD 遮罩

    void startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs);
    void tryBuyCard(int cost, MouseState state, const QString& cursorImgPath);

    LawnMower* lawnMowers[5];
    int grassGrid[5][9];

    QLabel* gameOverLabel;
    QSoundEffect* plantSound;

    // ==========================================
    // 🎬 冒险模式过渡动画组件
    // ==========================================
    QLabel* zombieHandLabel;
    QMovie* zombieHandMovie;
    QSoundEffect* evilLaughSound;
};