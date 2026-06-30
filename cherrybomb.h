#pragma once
#include "plant.h"

class CherryBomb : public Plant {
    Q_OBJECT
public:
    CherryBomb(int r, int c, QGraphicsItem* parent = nullptr);
    ~CherryBomb() override;

    // 樱桃炸弹没有周期性攻击，act() 仅用于起爆
    void act() override;
};