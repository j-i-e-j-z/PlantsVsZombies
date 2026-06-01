#include "sun.h"
#include "mainwindow.h"
#include <QMovie>
#include <QDebug>
#include <QRandomGenerator> 

Sun::Sun(QWidget* parent) : QLabel(parent), fallAnim(nullptr)
{
    this->resize(80, 80);

    QMovie* anim = new QMovie(":/res/images/Sun.gif");
    anim->setScaledSize(QSize(80, 80));
    this->setMovie(anim);
    anim->start();

    this->setCursor(Qt::PointingHandCursor);

    disappearTimer = new QTimer(this);
    connect(disappearTimer, &QTimer::timeout, this, &Sun::deleteLater);
    disappearTimer->start(10000);
}

void Sun::startFall(int startX, int targetY)
{
    this->move(startX, -80);

    fallAnim = new QPropertyAnimation(this, "pos", this);
    fallAnim->setStartValue(QPoint(startX, -80));
    fallAnim->setEndValue(QPoint(startX, targetY));
    fallAnim->setEasingCurve(QEasingCurve::OutQuad);
    fallAnim->setDuration(4000);
    fallAnim->start();
}

// ====================================================================================
// 物理引擎：控制向日葵产出阳光时的“抛出+弹跳”效果
// ====================================================================================
void Sun::startJump(int startX, int startY)
{
    this->move(startX, startY);

    int targetX = startX + QRandomGenerator::global()->bounded(-40, 40);
    int targetY = startY + QRandomGenerator::global()->bounded(30, 60);

    QPropertyAnimation* jumpAnim = new QPropertyAnimation(this, "pos", this);
    jumpAnim->setStartValue(QPoint(startX, startY));
    jumpAnim->setEndValue(QPoint(targetX, targetY));

    jumpAnim->setEasingCurve(QEasingCurve::OutBounce);
    jumpAnim->setDuration(800);
    jumpAnim->start();
}

void Sun::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {

        this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        this->setCursor(Qt::ArrowCursor);

        if (disappearTimer && disappearTimer->isActive()) {
            disappearTimer->stop();
        }

        if (fallAnim && fallAnim->state() == QAbstractAnimation::Running) {
            fallAnim->stop();
        }

        QPropertyAnimation* flyAnim = new QPropertyAnimation(this, "pos", this);
        flyAnim->setStartValue(this->pos());
        flyAnim->setEndValue(QPoint(150, 15));
        flyAnim->setDuration(600);
        flyAnim->setEasingCurve(QEasingCurve::InQuad);

        connect(flyAnim, &QPropertyAnimation::finished, [this]() {
            qDebug() << "【经济系统】阳光吸收完毕，成功入账！+50";

            // 严格使用 mainwindow 小写规范
            mainwindow* mainWin = (mainwindow*)(this->parentWidget());
            if (mainWin) {
                mainWin->addSun(50);
            }

            this->deleteLater();
            });

        flyAnim->start();
    }
}