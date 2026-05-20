#include "plant.h"

// 构造函数实现：初始化行列坐标、默认血量，并把父对象传给 QLabel
Plant::Plant(int r, int c, QWidget* parent)
    : QLabel(parent), row(r), col(c), hp(300), maxHp(300)
{
    // 初始化核心动作定时器
    actionTimer = new QTimer(this);
}

// 析构函数实现：清理工作
Plant::~Plant()
{
    // C++ 中 new 出来的没有父对象的指针才需要 delete
    // actionTimer 的父对象是 this，所以 Qt 会自动回收它，这里留空即可
}

// 通用受击逻辑
void Plant::takeDamage(int damage)
{
    hp -= damage;
    if (hp <= 0) {
        die(); // 血量归零，触发死亡
    }
}

// 通用死亡逻辑
void Plant::die()
{
    // 1. 停止一切动作
    if (actionTimer->isActive()) {
        actionTimer->stop();
    }
    // 2. 从画面上隐藏
    this->hide();
    // 3. 告诉 Qt 在下一帧把这个植物的内存安全回收掉 (极其关键，防止内存泄漏！)
    this->deleteLater();
}