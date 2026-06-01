#pragma once
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>

class Sun : public QLabel
{
    Q_OBJECT
public:
    Sun(QWidget* parent = nullptr);

    // 专为“天上掉阳光”准备的下落接口
    void startFall(int startX, int targetY);
    // 【新增】：专为“向日葵产出阳光”准备的抛出弹跳接口
    void startJump(int startX, int startY);
protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QPropertyAnimation* fallAnim;   // 控制下落的动画对象
    QTimer* disappearTimer;         // 控制自动消失的定时器
};