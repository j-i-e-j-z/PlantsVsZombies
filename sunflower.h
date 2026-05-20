#pragma once
#include "plant.h" // 必须包含基类头文件

class Sunflower : public Plant
{
    Q_OBJECT // Qt 宏
public:
    Sunflower(int r, int c, QWidget* parent = nullptr);

    // 重写基类的纯虚函数
    void act() override;
};