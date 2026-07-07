#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

// 引入 Qt 核心组件库，体现了基于组件开发的思想
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

class LawnMower; // 前置声明，减少头文件依赖，提升编译速度

// 主窗口类：扮演了游戏引擎中的 Game Manager 和 UI Controller 的双重角色
class mainwindow : public QMainWindow
{
    Q_OBJECT // 启用 Qt 元对象系统，支持信号与槽
public:
    mainwindow(QWidget* parent = nullptr);
    ~mainwindow();

    // 全局资源接口：供其他实体调用的系统级函数
    void addSun(int amount);
    void gameOver(QGraphicsObject* winnerZombie);
    QSoundEffect* peaFireSound;

protected:
    // 【架构亮点：事件拦截器】
    // 重写事件过滤器与键盘事件，用于实现精准的全局鼠标点击拦截（如种植植物）和快捷键（空格暂停）
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // 槽函数机制：响应游戏状态切换
    void nextLoadingStage(); // 驱动开场动画状态机
    void startGame();
    void startAdventure();
    void transitionToGame(); // 核心：切换到战斗场景并初始化内存实体

    void togglePause();      // 时空冻结核心引擎
    void returnToMainMenu(); // 内存安全回收与场景重置
    void restartGame();
    void startReadySetPlant();
    void updateProgress();   // 战役进度结算机制

    // 战役胜利核心流程
    void gameWon();
    void showVictoryScreen();

private:
    Ui::MainWindowClass* ui;

    // 【状态控制标志位】：防止高频事件造成逻辑冲突或 UI 重复生成
    bool isGameEnding = false;
    bool isGameInitialized = false;

    // 【三大核心引擎驱动器】：利用 Qt 的 QTimer 模拟游戏主循环(Game Loop)
    QTimer* zombieSpawnTimer; // 僵尸生成引擎
    QTimer* skySunTimer;      // 天气阳光生成引擎
    QTimer* progressTimer;    // 波次结算引擎

    int totalZombies;
    int spawnedZombies;
    int killedZombies;

    // 【MVC 架构体系】：View(视图) 与 Scene(场景模型) 分离
    QGraphicsScene* gameScene; // 负责管理所有的物理实体（植物、僵尸、子弹）
    QGraphicsView* gameView;   // 负责渲染场景并处理视口交互
    QGraphicsPixmapItem* combatBgItem;

    // ... (UI 组件指针定义，包含按钮、背景、动画等)
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

    // 阳光经济面板与卡槽系统
    QLabel* shopBoard;
    QPushButton* sunCardBtn;
    QPushButton* peaCardBtn;
    QPushButton* wallnutCardBtn;
    QPushButton* cherryCardBtn;
    QPushButton* repeaterCardBtn;
    QPushButton* shovelBankBtn;

    // 鼠标手持状态枚举 (FSM思想)
    enum MouseState { None, HoldingSunflower, HoldingPeashooter, HoldingWallNut, HoldingCherry, HoldingRepeater, HoldingShovel };
    MouseState currentMouseState = None;

    int sunCount = 150;
    QLabel* sunLabel;

    // 卡牌冷却遮罩
    QLabel* sunCardMask;
    QLabel* peaCardMask;
    QLabel* wallnutCardMask;
    QLabel* cherryCardMask;
    QLabel* repeaterCardMask;

    void startCardCooldown(QPushButton* btn, QLabel* mask, int durationMs);
    void tryBuyCard(int cost, MouseState state, const QString& cursorImgPath);

    // 【核心数据结构】：草坪网格的 O(1) 状态映射矩阵
    LawnMower* lawnMowers[5];
    int grassGrid[5][9]; // 记录草坪每个格子的占用状态 (0:空, 1:有植物)

    // ... 杂项特效与UI组件
    QLabel* gameOverLabel;
    QSoundEffect* plantSound;
    QLabel* zombieHandLabel;
    QMovie* zombieHandMovie;
    QSoundEffect* evilLaughSound;

    // 戴夫的对话状态机
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