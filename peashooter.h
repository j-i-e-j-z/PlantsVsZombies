#pragma once
#include "Plant.h"
#include <QTimer>

class PeaShooter : public Plant {
    Q_OBJECT
public:
    PeaShooter(int r, int c, QWidget* parent = nullptr);
    virtual ~PeaShooter();
    void act() override;
};