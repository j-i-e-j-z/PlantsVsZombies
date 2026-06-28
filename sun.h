#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QMovie>

class Sun : public QGraphicsObject
{
    Q_OBJECT
public:
    Sun(QGraphicsItem* parent = nullptr);
    ~Sun();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 天气系统：从天而降
    void startFall(int startX, int targetY);

    // 物理引擎：向日葵产出时的“抛出+弹跳”
    void startJump(int startX, int startY);

signals:
    // 解耦神器：通知外部（主窗口）加阳光
    void collected(int amount);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QMovie* sunMovie;                 // 负责解析 GIF 动画
    QTimer* disappearTimer;           // 10秒生命周期
    QPropertyAnimation* moveAnim;     // 统一管理降落、弹跳或飞行的动画指针
};