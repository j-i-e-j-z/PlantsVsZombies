#include "repeater.h"
#include "peabullet.h"
#include "zombie.h" 
#include <QTimer>
#include <QGraphicsScene>

Repeater::Repeater(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;
    yOffset = 40;

    plantMovie->setFileName(":/res/images/Repeater.gif");
    plantMovie->start();

    actionTimer->start(1500);
    connect(actionTimer, &QTimer::timeout, this, &Repeater::act);
}

Repeater::~Repeater() {}

//void Repeater::act() {
//    // 💡 第一发子弹
//    PeaBullet* bullet1 = new PeaBullet(row);
//    bullet1->setPos(this->pos().x() + 35, this->pos().y() - 35);
//    emit bulletFired(bullet1);
//
//    // 💡 第二发子弹：利用单次定时器制造 150ms 的异步延时开火效果
//    QTimer::singleShot(150, this, [this]() {
//        // 安全锁：如果在这 150ms 间隙植物被咬死了，则中断第二发子弹的生成
//        if (!this->scene()) return;
//
//        PeaBullet* bullet2 = new PeaBullet(row);
//        bullet2->setPos(this->pos().x() + 35, this->pos().y() - 35);
//        emit bulletFired(bullet2);
//        });
//}
void Repeater::act() {
    if (!this->scene()) return;

    // 🔍 【索敌雷达系统】：扫描本行前方是否有存活的僵尸
    bool hasTarget = false;
    for (QGraphicsItem* item : this->scene()->items()) {
        if (Zombie* zombie = dynamic_cast<Zombie*>(item)) {
            if (zombie->getRow() == this->row && zombie->x() > this->x() && zombie->x() < 1050) {
                hasTarget = true;
                break;
            }
        }
    }

    // 🎯 发现目标，执行双发指令
    if (hasTarget) {
        // 第一发
        PeaBullet* bullet1 = new PeaBullet(row);
        bullet1->setPos(this->pos().x() + 35, this->pos().y() - 35);
        emit bulletFired(bullet1);

        // 第二发：延迟 150ms，制造原汁原味的连发错落感
        QTimer::singleShot(150, this, [this]() {
            if (!this->scene()) return;
            PeaBullet* bullet2 = new PeaBullet(row);
            bullet2->setPos(this->pos().x() + 35, this->pos().y() - 35);
            emit bulletFired(bullet2);
            });
    }
}