#pragma once
#include <QLabel>
#include <QTimer>
#include <QDebug>

// 植物基类，继承自 QLabel 以便直接显示贴图
class Plant : public QLabel
{
    Q_OBJECT
public:
    // 构造函数：需要传入逻辑坐标 row, col，以及父窗口指针
    Plant(int r, int c, QWidget* parent = nullptr);
    virtual ~Plant();

    // 核心公共属性
    int row;         // 所在的逻辑网格行 (0~4)
    int col;         // 所在的逻辑网格列 (0~8)
    int hp;          // 当前生命值
    int maxHp;       // 最大生命值

    // 纯虚函数：强制要求所有派生的子类（如向日葵）必须实现自己的专属动作
    virtual void act() = 0;

public slots:
    // 通用受击与死亡机制
    void takeDamage(int damage);
    virtual void die();

protected:
    // 核心定时器：控制植物的动作频率 (例如多久产一次阳光/射一次子弹)
    QTimer* actionTimer;
};