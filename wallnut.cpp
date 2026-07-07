#include "wallnut.h"
#include <QDebug>

WallNut::WallNut(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent), crackState(0) // 初始化破损状态为 0（完好）
{
    // 赋予坚果墙极高的生命值，体现不同派生类的数据差异
    hp = 4000;
    maxHp = 4000;

    // Y轴偏移量调整：确保坚果的物理碰撞体积与草坪网格底边完美对齐
    yOffset = 45;

    // 加载初始完好状态的 GIF 动画
    plantMovie->setFileName(":/res/images/WallNut.gif");
    plantMovie->start();
}

WallNut::~WallNut()
{
}

void WallNut::act()
{
    // 坚果墙作为纯防御植物，无需执行周期性动作，因此保留空实现。
    // 这也是多态的一种体现，由基类的 actionTimer 调用。
}

void WallNut::takeDamage(int damage)
{
    // 1. 代码复用：先调用父类的扣血、受击闪红和死亡逻辑
    Plant::takeDamage(damage);

    // 2. 破损状态机机制：根据当前剩余生命值比例，动态切换外观
    if (hp > 0) {
        int newState = 0;
        if (hp <= 1333) {
            newState = 2; // 血量 <= 1/3，进入重度破损状态
        }
        else if (hp <= 2666) {
            newState = 1; // 血量 <= 2/3，进入轻度破损状态
        }

        // =========================================================
        // 🚀 【性能优化亮点】：防抖与阻断高频重载
        // 僵尸啃咬是非常高频的事件。只有当计算出的新状态与当前记录的
        // 状态不一致时，才允许重新从磁盘读取 GIF，极大地节省了系统开销！
        // =========================================================
        if (newState != crackState) {
            crackState = newState;

            plantMovie->stop();
            if (crackState == 1) {
                plantMovie->setFileName(":/res/images/WallNut1.gif");
                qDebug() << "【防线警报】坚果墙轻度破损！(HP < 2666)";
            }
            else if (crackState == 2) {
                plantMovie->setFileName(":/res/images/WallNut2.gif");
                qDebug() << "【防线警报】坚果墙重度破损，即将崩溃！(HP < 1333)";
            }
            plantMovie->start();
        }
    }
}