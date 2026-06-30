#pragma once
#include "plant.h"

class PeaBullet;

class Repeater : public Plant {
    Q_OBJECT
public:
    Repeater(int r, int c, QGraphicsItem* parent = nullptr);
    ~Repeater() override;

    void act() override;

signals:
    void bulletFired(PeaBullet* bullet);
};