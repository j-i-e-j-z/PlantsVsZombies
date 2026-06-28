#include "sun.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QCursor>

Sun::Sun(QGraphicsItem* parent) : QGraphicsObject(parent), moveAnim(nullptr)
{
    this->setCursor(Qt::PointingHandCursor);
    // ✅ 【神级修复】：强制要求阳光实体竖起耳朵，接收玩家的鼠标左键点击！
    this->setAcceptedMouseButtons(Qt::LeftButton);

    // 1. 手动接管 GIF 动画
    sunMovie = new QMovie(":/res/images/Sun.gif");
    sunMovie->start();

    // 每当 GIF 刷新一帧，就通知当前对象重绘自己
    connect(sunMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

    // 2. 生命周期管理
    disappearTimer = new QTimer(this);
    connect(disappearTimer, &QTimer::timeout, this, &Sun::deleteLater);
    disappearTimer->start(10000); // 10秒后自动销毁
}

Sun::~Sun()
{
    if (sunMovie) {
        sunMovie->stop();
        delete sunMovie;
    }
}

// 物理边界大小：80x80
QRectF Sun::boundingRect() const
{
    return QRectF(0, 0, 80, 80);
}

// 渲染引擎
void Sun::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // 性能优化：直接把当前帧画到 80x80 的矩形里，避免生成新的 QPixmap 对象
    if (sunMovie && sunMovie->currentPixmap().isNull() == false) {
        painter->drawPixmap(0, 0, 80, 80, sunMovie->currentPixmap());
    }
}


// ====================================================================================
// 行为逻辑 (针对 1000x600 1:1 原生坐标重构版)
// ====================================================================================
void Sun::startFall(int startX, int targetY)
{
    this->setPos(startX, -80);

    moveAnim = new QPropertyAnimation(this, "pos", this);
    moveAnim->setStartValue(QPointF(startX, -80));
    moveAnim->setEndValue(QPointF(startX, targetY));
    moveAnim->setEasingCurve(QEasingCurve::OutQuad);
    moveAnim->setDuration(4000);
    moveAnim->start();
}

void Sun::startJump(int startX, int startY)
{
    this->setPos(startX, startY);

    // 随机一个弹跳落点
    int targetX = startX + QRandomGenerator::global()->bounded(-30, 30);
    int targetY = startY + QRandomGenerator::global()->bounded(20, 40);

    // ✅【关键修复】：底部安全锁。
    // 在 1000x600 窗口中，草坪最底部 Y 轴在 560 左右。
    // 如果计算出的落点太靠下，强制截断在 515 像素，确保最下面一排的阳光绝对清晰可见！
    if (targetY > 515) {
        targetY = 515;
    }

    moveAnim = new QPropertyAnimation(this, "pos", this);
    moveAnim->setStartValue(QPointF(startX, startY));
    moveAnim->setEndValue(QPointF(targetX, targetY));
    moveAnim->setEasingCurve(QEasingCurve::OutBounce); // ✅ 经典的 OutBounce 皮球弹跳曲线
    moveAnim->setDuration(800);
    moveAnim->start();
}

void Sun::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {

        // 防连点锁：拒绝接收后续鼠标事件，并恢复默认指针
        this->setAcceptedMouseButtons(Qt::NoButton);
        this->setCursor(Qt::ArrowCursor);

        // 打断现有动作
        if (disappearTimer && disappearTimer->isActive()) {
            disappearTimer->stop();
        }
        if (moveAnim && moveAnim->state() == QAbstractAnimation::Running) {
            moveAnim->stop();
        }

        // 重新分配动画接管飞行
        QPropertyAnimation* flyAnim = new QPropertyAnimation(this, "pos", this);
        flyAnim->setStartValue(this->pos());

        // ✅【核心修复】：目标飞向新版日光计分板的太阳图标中心（大约 X=145, Y=40）
        // 这样阳光就会划过一道完美的弧线，收缩进左上角计分板，而不是飞到卡槽外面！
        flyAnim->setEndValue(QPointF(145, 40));

        flyAnim->setDuration(600);
        flyAnim->setEasingCurve(QEasingCurve::InQuad);

        // 飞行结束后的回调
        connect(flyAnim, &QPropertyAnimation::finished, [this]() {
            qDebug() << "【经济系统】阳光飞抵计分板！触发信号 +50";

            // 直接发射信号
            emit collected(50);

            this->deleteLater();
            });

        flyAnim->start();
    }
}