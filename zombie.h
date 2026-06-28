#pragma once
#include <QGraphicsObject>
#include <QMovie>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QSoundEffect>

class Zombie : public QGraphicsObject {
    Q_OBJECT
public:
    Zombie(int r, QGraphicsItem* parent = nullptr);
    ~Zombie() override;

    virtual QPainterPath shape() const override;
    // 图形框架核心重写
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    int getRow() const { return row; }
    void takeDamage(int damage, bool isFire = false);
    void pauseBehavior();

private slots:
    void move(); // 控制僵尸每帧向左挪动的槽函数

signals:
    void gameLost(QGraphicsObject* attacker);

private:
    enum ZombieState { Normal, LostArm, Eating, Dead, Burned };
    ZombieState state;
    int hp;             // 当前血量
    int maxHp;          // 最大血量
    int row;            // 僵尸当前所在的行号 (0-4)
    
    qreal speed;        // 移动速度

    QMovie* zombieMovie; // 僵尸行走 GIF 动图
    QTimer* moveTimer;   // 移动心跳定时器
    QSoundEffect* eatSound;    // 专属啃咬音效
    QSoundEffect* splatSound;  // 专属挨打音效
}; 
