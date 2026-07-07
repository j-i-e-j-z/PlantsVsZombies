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
    // 枚举类型定义僵尸种类，便于后期扩展其他特种僵尸
    enum ZombieType { NormalZombie, ConeheadZombie, BucketheadZombie };

    Zombie(int r, ZombieType type = NormalZombie, QGraphicsItem* parent = nullptr);
    ~Zombie() override;

    // 重写 Qt 图形框架接口
    virtual QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    int getRow() const { return row; }
    void takeDamage(int damage, bool isFire = false);

    // 时空冻结核心控制接口
    void pauseBehavior();
    void resumeBehavior();

private slots:
    void move(); // 周期性动作，包含移动、碰撞索敌与攻击判定

signals:
    // 将游戏失败信号抛出给主窗口接管处理
    void gameLost(QGraphicsObject* attacker);

private:
    // 有限状态机 (FSM) 状态集合
    enum ZombieState { Normal, LostArm, Eating, Dead, Burned };
    ZombieState state;

    ZombieType type;
    int hp;
    int maxHp;
    int armorHp; // 防具血量分离，方便结算伤害优先级
    QString currentFolder; // 动态资源路径，实现换皮复用逻辑

    int row;
    qreal speed;

    QMovie* zombieMovie;
    QTimer* moveTimer;

    // 独立音效管理
    QSoundEffect* eatSound;
    QSoundEffect* splatSound;
    QSoundEffect* shieldHitSound;
    QSoundEffect* plasticHitSound;
};