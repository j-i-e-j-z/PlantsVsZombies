#pragma once
#include "plant.h" // 确保 Plant 已经升级为 QGraphicsObject
#include <QMovie>
#include <QPainter>
#include "sun.h"   // 引入阳光实体

class Sunflower : public Plant
{
    Q_OBJECT
public:
    Sunflower(int r, int c, QGraphicsItem* parent = nullptr);
    ~Sunflower();

    // 重写基类的纯虚函数
    void act() override;

signals:
    // 【架构神技】：通知外部“我生了一个新阳光，请把它加入场景并接管”
    void sunProduced(Sun* newSun);

private:
   
};