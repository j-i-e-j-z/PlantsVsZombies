// ==================== sun.cpp ====================
#include "sun.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QCursor>

Sun::Sun(QGraphicsItem* parent) : QGraphicsObject(parent), moveAnim(nullptr)
{
    this->setCursor(Qt::PointingHandCursor);
    // 🖱️ 强制接收鼠标左键事件，准备劫持玩家的收集操作
    this->setAcceptedMouseButtons(Qt::LeftButton);

    sunMovie = new QMovie(":/res/images/Sun.gif");
    sunMovie->start();
    connect(sunMovie, &QMovie::frameChanged, this, [this]() { this->update(); });

    // 10秒后未被拾取自动消失
    disappearTimer = new QTimer(this);
    connect(disappearTimer, &QTimer::timeout, this, &Sun::deleteLater);
    disappearTimer->start(10000);
}

Sun::~Sun() {
    if (sunMovie) { sunMovie->stop(); delete sunMovie; }
}

QRectF Sun::boundingRect() const { return QRectF(0, 0, 80, 80); }
void Sun::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (sunMovie && sunMovie->currentPixmap().isNull() == false) {
        painter->drawPixmap(0, 0, 80, 80, sunMovie->currentPixmap());
    }
}

// 模拟抛物线弹跳 (向日葵产出时调用)
void Sun::startJump(int startX, int startY)
{
    this->setPos(startX, startY);

    // 计算随机落地坐标
    int targetX = startX + QRandomGenerator::global()->bounded(-30, 30);
    int targetY = startY + QRandomGenerator::global()->bounded(20, 40);

    // 边界钳制：防止底部一排的阳光飞出草坪不可见
    if (targetY > 500) targetY = 500;

    // 📈 【动画引擎】：使用 QPropertyAnimation 修改 pos 属性，并结合 OutBounce 实现真实皮球弹跳曲线
    moveAnim = new QPropertyAnimation(this, "pos", this);
    moveAnim->setStartValue(QPointF(startX, startY));
    moveAnim->setEndValue(QPointF(targetX, targetY));
    moveAnim->setEasingCurve(QEasingCurve::OutBounce);
    moveAnim->setDuration(800);
    moveAnim->start();
}

// 阳光自由落体 (主窗口定时器调用)
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

// 🖱️ 【交互核心】：重写鼠标按压事件
void Sun::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {

        // 防抖锁：被点击后拒绝后续点击，防止被重复收集产生计分 Bug
        this->setAcceptedMouseButtons(Qt::NoButton);
        this->setCursor(Qt::ArrowCursor);

        // 强行打断现有的销毁定时器和下落/弹跳动画
        if (disappearTimer && disappearTimer->isActive()) disappearTimer->stop();
        if (moveAnim && moveAnim->state() == QAbstractAnimation::Running) moveAnim->stop();

        // 重新分配一段飞行曲线，使其被吸入左上角计分板
        QPropertyAnimation* flyAnim = new QPropertyAnimation(this, "pos", this);
        flyAnim->setStartValue(this->pos());
        flyAnim->setEndValue(QPointF(145, 40));
        flyAnim->setDuration(600);
        flyAnim->setEasingCurve(QEasingCurve::InQuad); // 使用缓入曲线产生吸附感

        // 动画播完的 Lambda 回调：向主界面发信号结算金额，并释放自身内存
        connect(flyAnim, &QPropertyAnimation::finished, [this]() {
            emit collected(25);
            this->deleteLater();
            });

        flyAnim->start();
    }
}