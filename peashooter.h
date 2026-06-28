#pragma once
#include "plant.h"
#include <QMovie>
#include <QPainter>

class PeaBullet;

class PeaShooter : public Plant {
    Q_OBJECT
public:
    PeaShooter(int r, int c, QGraphicsItem* parent = nullptr);
    ~PeaShooter() override;

    void act() override;

signals:
    void bulletFired(PeaBullet* bullet);

private:
    
};