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
#include <QKeyEvent> 
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem> 
#include <QMovie>
#include "progressbar.h" 

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
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void nextLoadingStage();
    void startGame();
    void startAdventure();
    void transitionToGame();

    void togglePause();
    void returnToMainMenu();
    void restartGame();
    void startReadySetPlant();
    void updateProgress();

    // ✅ 新增：战役胜利核心流程
    void gameWon();
    void showVictoryScreen();

private:
    Ui::MainWindowClass* ui;
    bool isGameEnding = false;
    bool isGameInitialized = false; // ✅ 新增：防止反复进入游戏造成UI重复生成

    QTimer* zombieSpawnTimer;
    QTimer* skySunTimer;
    QTimer* progressTimer;
    int totalZombies;
    int spawnedZombies;
    int killedZombies;

    QGraphicsScene* gameScene;
    QGraphicsView* gameView;
    QGraphicsPixmapItem* combatBgItem;

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
    QPushButton* wallnutCardBtn;
    QPushButton* cherryCardBtn;
    QPushButton* repeaterCardBtn;
    QPushButton* shovelBankBtn;

    enum MouseState { None, HoldingSunflower, HoldingPeashooter, HoldingWallNut, HoldingCherry, HoldingRepeater, HoldingShovel };
    MouseState currentMouseState = None;
    int sunCount = 150;
    QLabel* sunLabel;

    QLabel* sunCardMask;
    QLabel* peaCardMask;
    QLabel* wallnutCardMask;
    QLabel* cherryCardMask;
    QLabel* repeaterCardMask;

    void startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs);
    void tryBuyCard(int cost, MouseState state, const QString& cursorImgPath);

    LawnMower* lawnMowers[5];
    int grassGrid[5][9];

    QLabel* gameOverLabel;
    QSoundEffect* plantSound;

    QLabel* zombieHandLabel;
    QMovie* zombieHandMovie;
    QSoundEffect* evilLaughSound;

    bool isPaused;
    bool isDaveTalking;
    int daveStep;

    QGraphicsPixmapItem* daveItem;
    QMovie* daveMovie;
    QSoundEffect* daveSound;

    QGraphicsPixmapItem* bubbleItem;
    QGraphicsTextItem* daveTextItem;

    QPushButton* menuButton;
    ProgressBar* waveProgressBar;

    QWidget* pauseWidget;

    void advanceDaveStateMachine();
    void showDaveDialog(const QString& text);
    void hideDaveDialog();
};