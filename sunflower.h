#pragma once
#include "plant.h" // 继承自 Plant 基类，体现了面向对象的多态性
#include <QMovie>
#include <QPainter>
#include "sun.h"   // 引入阳光实体，用于产出操作

// 向日葵类：继承自基础植物类
class Sunflower : public Plant
{
    Q_OBJECT // 必须包含宏，以启用 Qt 的元对象系统（信号与槽）
public:
    Sunflower(int r, int c, QGraphicsItem* parent = nullptr);
    ~Sunflower();

    // 重写基类的纯虚函数，实现了多态。每个植物都有自己的 act 行为。
    void act() override;

signals:
    // 【架构亮点：解耦设计】
    // 向日葵只负责“产生”阳光，不负责阳光在场景中的管理和计分。
    // 通过抛出信号，通知外部（主窗口）去接管这个新生成的阳光实体。
    void sunProduced(Sun* newSun);

private:

};