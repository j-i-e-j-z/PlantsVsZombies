#pragma once
#include <QGraphicsObject>
#include <QPixmap>
#include <QTimer>

class LawnMower : public QGraphicsObject {
    Q_OBJECT
public:
    LawnMower(int r, QGraphicsItem* parent = nullptr);
    ~LawnMower() override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private slots:
    void updateLogic(); // 每帧检测碰撞与移动

private:
    int row;            // 所在行号
    bool isTriggered;   // 是否被触发启动
    QMovie* mowerMovie;
    QTimer* driveTimer; // 引擎定时器
};