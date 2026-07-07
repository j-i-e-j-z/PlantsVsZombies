#include "peashooter.h"
#include "peabullet.h"
#include "zombie.h" 
#include <QDebug>
#include <QTimer> 
#include <QGraphicsScene>

PeaShooter::PeaShooter(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 200;
    maxHp = 200;

    // 调整 Y 轴偏移，使植物本体视觉高度与向日葵对齐
    yOffset = 55;

    // 加载待机动图
    plantMovie->setFileName(":/res/images/plant/PeaShooterSingle/full_idle.gif");
    plantMovie->start();

    // 开启 1.5 秒轮询的开火引擎
    actionTimer->start(1500);
    connect(actionTimer, &QTimer::timeout, this, &PeaShooter::act);
}

PeaShooter::~PeaShooter()
{
}

//void PeaShooter::act()
//{
//     1. 生成并定位子弹 (工厂模式雏形)
//    PeaBullet* bullet = new PeaBullet(row);
//     精准计算枪口位置
//    bullet->setPos(this->pos().x() + 35, this->pos().y() - 35);
//
//     2. 📡 【解耦神器】：抛出信号！
//     射手只负责"发射"动作，不直接调用 Scene 去 addItem，
//     而是抛出信号，由外部总控(mainwindow)去挂载和播报音效，极大降低耦合度。
//    emit bulletFired(bullet);
//}
void PeaShooter::act()
{
    if (!this->scene()) return;

    // 扫描本行前方是否有存活的僵尸
    bool hasTarget = false;
    for (QGraphicsItem* item : this->scene()->items()) {
        if (Zombie* zombie = dynamic_cast<Zombie*>(item)) {
            // 判定条件：同在一行 + 僵尸在植物右边 + 僵尸还没走出屏幕右侧边界 + 僵尸还没死
            if (zombie->getRow() == this->row && zombie->x() > this->x() && zombie->x() < 1050) {
                hasTarget = true;
                break; // 只要发现一个目标，立马终止扫描，准备开火！
            }
        }
    }

    // 发现目标，执行开火指令
    if (hasTarget) {
        PeaBullet* bullet = new PeaBullet(row);
        bullet->setPos(this->pos().x() + 35, this->pos().y() - 35);
        emit bulletFired(bullet);
    }
}