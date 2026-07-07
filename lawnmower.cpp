#include "lawnmower.h"
#include "zombie.h"
#include <QPainter>
#include <QList>
#include <QDebug>
#include <QMovie>

LawnMower::LawnMower(int r, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), isTriggered(false) // 初始状态为静止
{
    mowerMovie = new QMovie(":/res/images/other/LawnMower/normal.gif");
    mowerMovie->start();

    // 绑定帧更新重绘
    connect(mowerMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

    // 设置 30ms 轮询一次物理判定引擎
    driveTimer = new QTimer(this);
    connect(driveTimer, &QTimer::timeout, this, &LawnMower::updateLogic);
    driveTimer->start(30);
}

LawnMower::~LawnMower()
{
    if (mowerMovie) {
        mowerMovie->stop();
        delete mowerMovie;
    }
}

QRectF LawnMower::boundingRect() const {
    return QRectF(0, 0, 105, 105);
}

void LawnMower::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (mowerMovie && mowerMovie->isValid()) {
        QPixmap pix = mowerMovie->currentPixmap();
        QPixmap scaledPix = pix.scaled(105, 105, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(0, 0, scaledPix);
    }
}

// 核心物理与逻辑更新引擎
void LawnMower::updateLogic()
{
    // 1. 碰撞索敌
    QList<QGraphicsItem*> items = this->collidingItems();
    bool hitZombie = false;

    for (QGraphicsItem* item : items) {
        Zombie* zombie = dynamic_cast<Zombie*>(item);
        if (zombie && zombie->getRow() == this->row) {
            hitZombie = true;
            // 造成毁天灭地的真实伤害，直接秒杀！
            zombie->takeDamage(9999);
        }
    }

    // 2. 状态机切换：点火启动
    if (!isTriggered && hitZombie) {
        isTriggered = true; // 锁定状态，防止重复触发
        qDebug() << "【防线警报】第" << row << "行小推车启动！碾碎他们！";

        // 切换冲刺形态图
        mowerMovie->stop();
        mowerMovie->setFileName(":/res/images/other/LawnMower/tricked.gif");
        mowerMovie->start();
    }

    // 3. 启动后的狂飙逻辑
    if (isTriggered) {
        // 向右以极快速度冲刺
        this->moveBy(25, 0);

        // 越出屏幕边界后自我销毁释放内存
        if (this->pos().x() > 1400) {
            this->deleteLater();
        }
    }
}