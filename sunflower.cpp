#include "sunflower.h"
#include <QMovie> // 【新增】引入 QMovie 头文件以便播放 GIF
#include <QDebug>

// 构造函数：初始化向日葵的属性和动画
Sunflower::Sunflower(int r, int c, QWidget* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;

    // 1. 强行锁死 QLabel 的物理尺寸，防止图片被裁切
    this->resize(80, 80);

    // 2. 加载 GIF 动态图 (请确保 .qrc 资源里是 SunFlower.gif)
    QMovie* anim = new QMovie(":/res/images/SunFlower.gif");

    // 让 GIF 动画按比例缩放，填满咱们规定的 80x80 框框
    anim->setScaledSize(QSize(80, 80));

    // 将动画挂载到当前的 QLabel 上并开始播放！
    this->setMovie(anim);
    anim->start();

    // 3. 启动定时器，每隔 5 秒产出一次阳光
    actionTimer->start(5000);
    connect(actionTimer, &QTimer::timeout, this, &Sunflower::act);
}

// 具体动作：产出阳光
void Sunflower::act()
{
    qDebug() << "【阳光系统】向日葵在 第" << row << "行, 第" << col << "列 产出了阳光！";
}