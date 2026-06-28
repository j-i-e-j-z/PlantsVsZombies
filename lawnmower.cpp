#include "lawnmower.h"
#include "zombie.h"
#include <QPainter>
#include <QList>
#include <QDebug>
#include <QMovie>

LawnMower::LawnMower(int r, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), isTriggered(false)
{
    // =========================================================
    // 🚗 【V8引擎装载】：加载待机动画 normal.gif
    // =========================================================
    mowerMovie = new QMovie(":/res/images/other/LawnMower/normal.gif");
    mowerMovie->start();

    // 绑定帧更新
    connect(mowerMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

    driveTimer = new QTimer(this);
    connect(driveTimer, &QTimer::timeout, this, &LawnMower::updateLogic);
    driveTimer->start(30); // 30ms 刷新一次物理判定
}

LawnMower::~LawnMower()
{
    if (mowerMovie) {
        mowerMovie->stop();
        delete mowerMovie;
    }
}

QRectF LawnMower::boundingRect() const
{
    return QRectF(0, 0, 105, 105);
}

void LawnMower::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (mowerMovie && mowerMovie->isValid()) {
        QPixmap pix = mowerMovie->currentPixmap();
        // 保持 105x105 的完美大小
        QPixmap scaledPix = pix.scaled(105, 105, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, scaledPix);
    }
}

void LawnMower::updateLogic()
{
    // 1. 获取所有与小推车重叠的物体
    QList<QGraphicsItem*> items = this->collidingItems();
    bool hitZombie = false;

    for (QGraphicsItem* item : items) {
        Zombie* zombie = dynamic_cast<Zombie*>(item);
        // 如果碰到了僵尸，且在同一行
        if (zombie && zombie->getRow() == this->row) {
            hitZombie = true;
            // 造成毁天灭地的真实伤害，直接秒杀！
            zombie->takeDamage(9999);
        }
    }

    // 2. 如果还没启动，且碰到了僵尸，则点火启动！
    if (!isTriggered && hitZombie) {
        isTriggered = true;
        qDebug() << "【防线警报】第" << row << "行小推车启动！碾碎他们！";

        // =========================================================
        // 🔥 【形态切换】：变成炫酷的冲刺形态 tricked.gif！
        // =========================================================
        mowerMovie->stop();
        mowerMovie->setFileName(":/res/images/other/LawnMower/tricked.gif");
        mowerMovie->start();
    }

    // 3. 启动后的狂飙逻辑
    if (isTriggered) {
        // 向右以极快速度冲刺
        this->moveBy(25, 0);

        // 冲出屏幕最右侧后，功成身退，销毁自己
        if (this->pos().x() > 1400) {
            this->deleteLater();
        }
    }
}