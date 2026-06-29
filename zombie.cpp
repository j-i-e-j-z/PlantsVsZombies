#include "zombie.h"
#include "plant.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>  // ✅ 新增：用于闪白打击感
#include <QGraphicsScene>           // ✅ 新增：用于向场景中添加飞出去的头
#include <QGraphicsProxyWidget>     // ✅ 新增：用于包装掉落的头
#include <QLabel>                   // ✅ 新增：用于承载头的GIF
#include <QSoundEffect>
#include <QUrl>
#include <QTimer>

Zombie::Zombie(int r, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), hp(270), maxHp(270), speed(30), state(Normal)
{
    // 1. 加载僵尸行走 GIF
    // ⚠️ 极其重要：请一定要确保你的 qrc 资源文件里有 ZombieWalk1.gif 这个文件
    zombieMovie = new QMovie(":/res/images/Zombie/Zombie_normal/walk.gif");
    zombieMovie->start();

    // 监听 GIF 帧变化触发重绘
    connect(zombieMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

    // =========================================================
    // 🎵 【预加载发声器官】：只 new 一次，绝对不卡！
    // =========================================================
    eatSound = new QSoundEffect(this); // 绑定 this，僵尸死的时候它自动销毁
    eatSound->setSource(QUrl("qrc:/res/sound/chomp.wav"));
    eatSound->setVolume(0.5f);

    splatSound = new QSoundEffect(this);
    splatSound->setSource(QUrl("qrc:/res/sound/splat.wav"));
    splatSound->setVolume(0.7f);

    // 2. 启动移动心跳：每 30 毫秒向左挪动一点点
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &Zombie::move);
    moveTimer->start(30);
}

Zombie::~Zombie()
{
    if (zombieMovie) {
        zombieMovie->stop();
        delete zombieMovie;
    }
}

// ====================================================================
// 【体型定义】：僵尸一般比植物高大，这里定义为宽 80，高 115 的瘦高矩形
// ====================================================================
// =========================================================
// 📦 【物理画布】：必须巨大，包住所有的渲染像素，绝对不留残影
// =========================================================
QRectF Zombie::boundingRect() const
{
    // 把画图区域开得极大（左右各 100 像素），确保僵尸的任何动作都不会画到外面去
    return QRectF(-100, -50, 200, 250);
}

// =========================================================
// ⚔️ 【真实肉体】：极窄的碰撞箱，解决隔空啃咬
// =========================================================
QPainterPath Zombie::shape() const
{
    QPainterPath path;
    // ✅ 核心大招：这是僵尸真实的物理肉体！
    // 只有这宽 40 像素的躯干碰到植物，才会触发啃咬！
    // 假设僵尸的 X 原点在中心，它的肉体范围就是左右各 20 像素。
    path.addRect(-20, 0, 40, 130);
    return path;
}

// =========================================================
// 🖌️ 【渲染逻辑】：智能居中的等比缩放
// =========================================================
void Zombie::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (zombieMovie && zombieMovie->isValid()) {
        QPixmap pix = zombieMovie->currentPixmap();

        // 强制高度对齐草坪(140)，宽度自适应防变形
        QPixmap scaledPix = pix.scaledToHeight(140, Qt::SmoothTransformation);

        // ✅ 智能居中：把计算出的图片宽度除以 2，往左偏移，让僵尸的视觉中心死死对准 X 原点
        painter->drawPixmap(-scaledPix.width() / 2, 0, scaledPix);

        // (可选测试：你可以取消下面这行代码的注释，运行游戏就能看到僵尸真实的碰撞框！)
        // painter->setPen(Qt::red);
        // painter->drawRect(-20, 0, 40, 130);
    }
}

void Zombie::move()
{
    // 如果已经死了或者被烧成了灰，直接罢工
    if (state == Dead || state == Burned) return;

    // ====================================================================
    // 🍕 分支 A：如果僵尸正处于【啃咬状态】
    // ====================================================================
    if (state == Eating) {
        // 1. 每帧去大舞台里搜一下：我面前还有没有紧贴着的活植物？
        QList<QGraphicsItem*> items = this->collidingItems();
        Plant* targetPlant = nullptr;

        for (QGraphicsItem* item : items) {
            Plant* p = dynamic_cast<Plant*>(item);
            // 必须是植物，且必须和我处于同一行
            if (p && p->getRow() == this->row) {
                targetPlant = p;
                break;
            }
        }

        // 2. 如果植物还在，疯狂输出！
        if (targetPlant) {
            // 因为 move() 30ms 执行一次，太快了！我们用计数器强行控制“啃咬攻速”
            // 30ms * 16 ≈ 500ms (也就是每半秒僵尸啃一口)
            static int eatCounter = 0;
            eatCounter++;
            if (eatCounter >= 16) {
                eatCounter = 0;

                // ⚠️ 重点：调用植物的扣血函数（原版僵尸每口伤害通常是 20）
                // 请确保你的 Plant 基类或父类里已经写好了类似 takeDamage(int) 的函数
                targetPlant->takeDamage(20);
                qDebug() << "【战斗系统】僵尸正在嘎吱嘎吱咬植物！";

                // =========================================================
                // 🎵 【优化】：直接播放预加载的音效！
                // =========================================================
                eatSound->play();
            }
        }
        // 3. 如果运行到这里 targetPlant 为空，说明植物已经被咬烂销毁了！
        else {
            // 擦擦嘴，根据目前的残余血量，恢复对应的行走衣服
            state = (hp <= 90) ? LostArm : Normal;

            zombieMovie->stop();
            zombieMovie->setFileName(state == LostArm ? ":/res/images/Zombie/Zombie_normal/walk2.gif" : ":/res/images/Zombie/Zombie_normal/walk.gif");
            zombieMovie->start();
            qDebug() << "【战斗系统】植物被吃光了，僵尸继续前进！";
        }

        return; // ❗极其重要：处于啃咬状态时，直接 return，不执行下面的向前平移代码！
    }

    // ====================================================================
    // 🚶 分支 B：如果僵尸处于【正常行走状态】
    // ====================================================================
    // 1. 匀速向左平移
    this->moveBy(-speed, 0);

    // 2. 移动完立刻嗅一下：我的脸有没有撞到新植物？
    QList<QGraphicsItem*> items = this->collidingItems();
    for (QGraphicsItem* item : items) {
        Plant* p = dynamic_cast<Plant*>(item);
        if (p && p->getRow() == this->row) {
            // 抓到了！立刻立定，换上啃咬的衣服
            state = Eating;
            zombieMovie->stop();
            zombieMovie->setFileName(":/res/images/Zombie/Zombie_normal/eat.gif"); // 确认你资源库里的名字
            zombieMovie->start();
            qDebug() << "【战斗系统】僵尸遭遇植物！停下开始啃咬。";
            return;
        }
    }

    // 3. 
    // =========================================================
    // 突破防线判定（小车在 X=130，越过 X=100 说明小车已失守且肉身进屋）
    // =========================================================
    if (this->pos().x() < 160) {
        qDebug() << "【战况警报】僵尸突破了防线！";

        // 发射战败信号，将当前突破防线的僵尸指针自身传过去
        emit gameLost(this);

        // 注意：这里不要执行 deleteLater() 了，把它交给主窗口的战败特写来控制
    }
}


void Zombie::takeDamage(int damage, bool isFire)
{
    // 如果已经死了或者被烧成了灰，直接无视
    if (state == Dead || state == Burned) return;

    hp -= damage;

    // =========================================================
    // 💥 【优化：吧唧受击音效直接播放】
    // =========================================================
    splatSound->play();

    // =========================================================
    // 💥 受击闪白特效
    // =========================================================
    QGraphicsColorizeEffect* flashEffect = new QGraphicsColorizeEffect();
    flashEffect->setColor(Qt::white);
    flashEffect->setStrength(0.8);
    this->setGraphicsEffect(flashEffect);

    QTimer::singleShot(100, this, [this]() {
        this->setGraphicsEffect(nullptr); // 设置 nullptr 时，Qt 会自动销毁旧的 effect，不会内存泄漏
        });

    // 🌟 状态 3：烈火焚身
    if (hp <= 0 && isFire) {
        state = Burned;
        speed = 0;
        moveTimer->stop();
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Burn.gif");
        zombieMovie->start();
        QTimer::singleShot(2500, this, [this]() { this->deleteLater(); });
        return;
    }

    // 🌟 状态 1：断手残血状态
    if (hp <= 90 && state == Normal) {
        state = LostArm;
        qDebug() << "【战况系统】僵尸手臂被打断了！";
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/Zombie_normal/walk2.gif");
        zombieMovie->start();
    }
    // 🌟 状态 2：普通物理死亡状态
    else if (hp <= 0) {
        state = Dead;
        qDebug() << "【战况系统】僵尸被普通击杀！";

        speed = 0;
        moveTimer->stop();

        // 1. 身体播放无头倒地动画
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/Zombie_normal/death.gif");
        zombieMovie->start();

        // 💀 身首异处掉头特效
        QLabel* headLabel = new QLabel();
        headLabel->setAttribute(Qt::WA_TranslucentBackground);
        QMovie* headMovie = new QMovie(":/res/images/ZombieHead.gif");
        headLabel->setMovie(headMovie);
        headMovie->start();

        QGraphicsProxyWidget* headProxy = this->scene()->addWidget(headLabel);
        headProxy->setPos(this->pos().x() + 20, this->pos().y() - 10);
        headProxy->setZValue(this->zValue() + 1);

        QTimer::singleShot(2500, this, [this, headProxy]() {
            headProxy->deleteLater();
            this->deleteLater();
            });
    }
}

// =========================================================
// 🛑 【全场冻结】：停止僵尸的所有动作和位移
// =========================================================
void Zombie::pauseBehavior()
{
    // 1. 拔掉移动的电源，让它停在原地
    if (moveTimer && moveTimer->isActive()) {
        moveTimer->stop();
    }

    // 2. 让 GIF 动图停留在当前这一帧，呈现完美的“定格”效果
    if (zombieMovie) {
        zombieMovie->setPaused(true);
    }
}