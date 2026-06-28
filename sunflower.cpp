#include "sunflower.h"
#include <QDebug>

Sunflower::Sunflower(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;

    yOffset = 40;
    // ✅ 直接继承父类的播放器！千万不要加 new QMovie！
    // ⚠️ 顺便核对你的 qrc 资源路径里 F 是大写还是小写！
    plantMovie->setFileName(":/res/images/plant/SunFlower/SunFlower.gif");
    plantMovie->start();

    // 10秒产一次阳光
    actionTimer->start(10000);
    connect(actionTimer, &QTimer::timeout, this, &Sunflower::act);
}

Sunflower::~Sunflower()
{
}

void Sunflower::act()
{
    qDebug() << "【阳光系统】向日葵在 第" << row << "行, 第" << col << "列 产出了阳光！";
    Sun* jumpSun = new Sun();

    // ✅【视觉修复】：调整偏移量。
    // 让初始坐标往上移 15 像素（Y-15），往左挪 10 像素（X-10），刚好对应金黄色花盘的正中心！
    jumpSun->startJump(this->pos().x() - 10, this->pos().y() - 15);

    emit sunProduced(jumpSun);
}