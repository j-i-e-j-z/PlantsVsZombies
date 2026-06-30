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
    enum ZombieType { NormalZombie, ConeheadZombie, BucketheadZombie };

    Zombie(int r, ZombieType type = NormalZombie, QGraphicsItem* parent = nullptr);
    ~Zombie() override;

    virtual QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    int getRow() const { return row; }
    void takeDamage(int damage, bool isFire = false);

    // 核心控制接口
    void pauseBehavior();
    void resumeBehavior(); // ✅ 新增：恢复行为

private slots:
    void move();

signals:
    void gameLost(QGraphicsObject* attacker);

private:
    enum ZombieState { Normal, LostArm, Eating, Dead, Burned };
    ZombieState state;

    ZombieType type;
    int hp;
    int maxHp;
    int armorHp;
    QString currentFolder;

    int row;
    qreal speed;

    QMovie* zombieMovie;
    QTimer* moveTimer;
    QSoundEffect* eatSound;
    QSoundEffect* splatSound;
    QSoundEffect* shieldHitSound;
    QSoundEffect* plasticHitSound;
};