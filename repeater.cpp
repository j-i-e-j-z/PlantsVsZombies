#include "repeater.h"
#include "peabullet.h"
#include <QTimer>

Repeater::Repeater(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;

    // ✅ 完美同步你 peashooter.cpp 中的专属鞋垫高度
    yOffset = 55;

    // 载入双发射手动图
    plantMovie->setFileName(":/res/images/Repeater.gif");
    plantMovie->start();

    // 攻击频率与单发豌豆一致，每 1.5 秒触发一次连发
    actionTimer->start(1500);
    connect(actionTimer, &QTimer::timeout, this, &Repeater::act);
}

Repeater::~Repeater() {}

void Repeater::act() {
    // 💡 第一发子弹：完美调用你的带参构造 new PeaBullet(row) 和枪口坐标 (+35, -35)
    PeaBullet* bullet1 = new PeaBullet(row);
    bullet1->setPos(this->pos().x() + 35, this->pos().y() - 35);
    emit bulletFired(bullet1);

    // 💡 第二发子弹：延迟 150ms 发射，形成原汁原味的连发感
    QTimer::singleShot(150, this, [this]() {
        // 如果植物在 150ms 内被吃掉了，就不发射第二颗
        if (!this->scene()) return;

        PeaBullet* bullet2 = new PeaBullet(row);
        bullet2->setPos(this->pos().x() + 35, this->pos().y() - 35);
        emit bulletFired(bullet2);
        });
}