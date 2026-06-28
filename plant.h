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

    // ✅ 由父类统一接管的碰撞体积与渲染，子类不要再重写它们了！
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 核心动作虚函数
    virtual void act() = 0;

    int getRow() const { return row; }
    virtual void takeDamage(int damage);
    virtual void die();

protected:
    int row;
    int col;
    int hp;
    int maxHp;
    QTimer* actionTimer;   // 动作定时器
    QMovie* plantMovie;    // ✅ 统一的动画播放器
    int yOffset;
};