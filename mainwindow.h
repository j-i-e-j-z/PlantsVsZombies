#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <QTimer>
#include <QLabel>
#include <QSoundEffect> // 播放wav音频的头文件
#include <QGraphicsOpacityEffect> // 包含透明度特效和属性动画的头文件
#include <QPropertyAnimation>
#include <QPushButton> // 修复了之前的拼写大小写错误
#include <QMouseEvent> // 【新增】用于处理鼠标点击事件

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // 重写父类的鼠标按下事件，用于捕获玩家在战斗草坪上的点击动作
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    // 自定义槽函数：用于处理定时器超时后的画面切换
    void nextLoadingStage();
    void startGame();      // 处理点击草地“进入游戏”的槽函数
    void startAdventure(); // 【新增】处理点击墓碑“开始冒险吧”的槽函数，用于加载战斗草坪

private:
    Ui::MainWindowClass* ui;

    QTimer* loadingTimer;   // 控制画面的定时器
    QLabel* imageLabel;     // 用来显示图片的标签
    QSoundEffect* bgm;      // 背景音乐播放器
    int currentStage;       // 记录当前播放到了哪一张图

    // 透明度特效和动画控制器
    QGraphicsOpacityEffect* opacityEffect;
    QPropertyAnimation* fadeAnimation;

    // 草地与按钮
    QLabel* floorLabel; // 承载草地的标签
    QPropertyAnimation* rollAnimation; // 控制草地铺开的动画
    QPushButton* startButton; // 进入游戏的按钮

    // 主菜单界面元素
    QLabel* menuBgLabel;         // 墓碑背景 (Surface.png)
    QPushButton* btnAdventure;   // 冒险模式按钮 (mx)
    QPushButton* btnMiniGames;   // 迷你游戏按钮 (mini)
    QPushButton* btnPuzzle;      // 解谜模式按钮 (yizi)
    QPushButton* btnPlay;        // 玩玩小游戏按钮 (play)

    // 【新增：商店卡槽系统 UI】
    QLabel* shopBoard;          // 商店背景木板
    QPushButton* sunCardBtn;    // 向日葵卡片按钮
    QPushButton* peaCardBtn;    // 豌豆射手卡片按钮

    // ---------------------------------------------------------
    // 【必须补上的这几行】：种植状态机定义
    // ---------------------------------------------------------
    // 定义鼠标当前处于什么状态（手里没东西、拿着向日葵、拿着豌豆）
    enum MouseState { None, HoldingSunflower, HoldingPeashooter };
    
    // 记录当前鼠标手里的状态，一开始默认是空手 (None)
    MouseState currentMouseState = None; 
    // ---------------------------------------------------------

    // 【新增：阳光经济系统】
    int sunCount = 50; // 开局送 50 阳光
    QLabel* sunLabel;  // 显示阳光数值的文本标签

    // 【新增】核心战斗场景元素
    QLabel* combatBgLabel;       // 真正的战斗草坪背景 (Background.jpg)
    QLabel* lawnMowers[5];       // 5辆保护底线的小推车
    int grassGrid[5][9];         // 5行9列的草地状态数组 (0=空地, 1=有植物)
};