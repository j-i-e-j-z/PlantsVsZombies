// ==================== sunflower.cpp ====================
#include "sunflower.h"
#include <QDebug>

Sunflower::Sunflower(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;
    yOffset = 40;

    plantMovie->setFileName(":/res/images/plant/SunFlower/SunFlower.gif");
    plantMovie->start();

    // 开启 10 秒产出阳光的引擎
    actionTimer->start(10000);
    connect(actionTimer, &QTimer::timeout, this, &Sunflower::act);
}

Sunflower::~Sunflower() {}

void Sunflower::act()
{
    qDebug() << "【阳光系统】向日葵在 第" << row << "行, 第" << col << "列 产出了阳光！";
    // 实例化阳光对象
    Sun* jumpSun = new Sun();

    // 计算生成坐标的偏移，使其从花盘中心弹出
    jumpSun->startJump(this->pos().x() - 10, this->pos().y() - 15);

    // 抛出信号交由主窗口托管
    emit sunProduced(jumpSun);
}