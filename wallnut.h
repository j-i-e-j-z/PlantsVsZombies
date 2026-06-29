#pragma once
#include "plant.h"

class WallNut : public Plant {
    Q_OBJECT
public:
    WallNut(int r, int c, QGraphicsItem* parent = nullptr);
    ~WallNut() override;

    // 坚果纯抗压，无需主动攻击或生产，但必须重写纯虚函数
    void act() override;

    // 重写受击逻辑，接管破损换图机制
    void takeDamage(int damage) override;

private:
    int crackState; // 记录当前破损状态 (0: 完好, 1: 轻度破损, 2: 重度破损)
};