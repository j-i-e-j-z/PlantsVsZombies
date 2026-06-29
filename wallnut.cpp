#include "wallnut.h"
#include <QDebug>

WallNut::WallNut(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent), crackState(0)
{
    // 赋予坚果墙极高的生命值 (普通植物通常是 300)
    hp = 4000;
    maxHp = 4000;

    // 坚果体型偏圆矮，Y轴偏移量设置在 45 左右，完美贴合我们底边居中的物理锚点算法
    yOffset = 45;

    // 加载初始完好状态的 GIF
    plantMovie->setFileName(":/res/images/WallNut.gif");
    plantMovie->start();
}

WallNut::~WallNut()
{
}

void WallNut::act()
{
    // 坚果墙作为纯防御植物，像一块石头一样坚挺即可，保持空实现
}

void WallNut::takeDamage(int damage)
{
    // 1. 先调用父类的扣血、受击闪红和死亡逻辑
    Plant::takeDamage(damage);

    // 2. 如果坚果还没被吃掉，计算当前血量对应的破损形态
    if (hp > 0) {
        int newState = 0;
        if (hp <= 1333) {
            newState = 2; // 血量 <= 1/3，重度破损
        }
        else if (hp <= 2666) {
            newState = 1; // 血量 <= 2/3，轻度破损
        }

        // =========================================================
        // 🚀 【性能红线防护】：严格阻断高频重载！
        // 只有当计算出的新状态与当前记录的状态不一致时，才允许重新读取磁盘动图！
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