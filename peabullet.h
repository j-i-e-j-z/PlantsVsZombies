// peabullet.h
#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include <QTimer>

class PeaBullet : public QGraphicsObject {
    Q_OBJECT
public:
    PeaBullet(int r, QGraphicsItem* parent = nullptr);
    ~PeaBullet();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    int row;             // 记录自己在哪一行飞行
    QPixmap bulletImage;
    QTimer* flyTimer;    // 控制飞行的引擎
};