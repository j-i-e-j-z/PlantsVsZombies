#pragma once
#include <QGraphicsObject> 
#include <QTimer>
#include <QMovie>
#include <QPainter>

class Plant : public QGraphicsObject
{
    Q_OBJECT
public:
    Plant(int r, int c, QGraphicsItem* parent = nullptr);
    virtual ~Plant();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    virtual void act() = 0;

    int getRow() const { return row; }
    virtual void takeDamage(int damage);
    virtual void die();

    // ✅ 新增：植物时空冻结机制
    virtual void pauseBehavior();
    virtual void resumeBehavior();

protected:
    int row;
    int col;
    int hp;
    int maxHp;
    QTimer* actionTimer;
    QMovie* plantMovie;
    int yOffset;
};