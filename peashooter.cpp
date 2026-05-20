#include "peashooter.h"
#include <QPixmap>
#include <QDebug>

PeaShooter::PeaShooter(int r, int c, QWidget* parent)
    : Plant(r, c, parent)
{
    // 1. 设置豌豆射手的专属属性
    hp = 200;
    maxHp = 200;

    // 【关键修复】：强行撑开物理外框，防止下半截被截断
    this->resize(80, 80);

    // 2. 加载图片并锁定缩放比例（保持和向日葵一样的 80x80 规范尺寸，防止变形）
    // 使用平滑缩放提高显示质量
    this->setPixmap(QPixmap(":/res/images/Peashooter.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 3. 启动动作定时器
    // 注意：复用基类创建好的 actionTimer
    connect(actionTimer, &QTimer::timeout, this, &PeaShooter::act);
    actionTimer->start(1500); // 每 1.5 秒发射一次豌豆
}

PeaShooter::~PeaShooter()
{
    // 如果基类的 die() 函数处理得当，这里其实不需要手动 stop。
    // 但加上这层防御性代码也是个好习惯，保持不变。
    if (actionTimer && actionTimer->isActive()) {
        actionTimer->stop();
    }
}

void PeaShooter::act()
{
    qDebug() << QString("【战斗系统】豌豆射手在 [%1][%2] 发射了子弹！").arg(row).arg(col);
}