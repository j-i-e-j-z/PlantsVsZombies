#include "peashooter.h"
#include "peabullet.h"
#include <QDebug>
#include <QTimer> // 确保引入了 QTimer 头文件

PeaShooter::PeaShooter(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 200;
    maxHp = 200;

    // =========================================================
    // 🎯 【专属鞋垫】：把豌豆射手往下拽，和向日葵对齐！
    // =========================================================
    yOffset = 55; // (如果发现还是偏高，就改成 70 或 75 试试)

    // 加载待机动图
    plantMovie->setFileName(":/res/images/plant/PeaShooterSingle/full_idle.gif");
    plantMovie->start();

    // 1.5秒开一次火
    actionTimer->start(1500);
    connect(actionTimer, &QTimer::timeout, this, &PeaShooter::act);
}

PeaShooter::~PeaShooter()
{
}

void PeaShooter::act()
{
    // 1. 生成并定位子弹
    PeaBullet* bullet = new PeaBullet(row);
    bullet->setPos(this->pos().x() + 35, this->pos().y() - 35);

    // 2. 发射子弹
    emit bulletFired(bullet);

    // ⚠️ 删掉下面所有切换 plantMovie 的代码！
    // 不要 setFileName("...shooting.gif")
    // 也不要 QTimer::singleShot 恢复动画
}