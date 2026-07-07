#include "zombie.h"
#include "plant.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>  
#include <QGraphicsScene>           
#include <QGraphicsProxyWidget>     
#include <QLabel>                   
#include <QSoundEffect>
#include <QUrl>
#include <QTimer>

Zombie::Zombie(int r, ZombieType type, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), type(type), hp(270), maxHp(270), speed(0.3), state(Normal)
{
    // 工厂模式思想：根据传入的枚举类型，初始化不同的防具血量与资源路径
    if (type == ConeheadZombie) {
        armorHp = 370;
        currentFolder = "Zombie_conehead";
    }
    else if (type == BucketheadZombie) {
        armorHp = 1100;
        currentFolder = "Zombie_buckethead";
    }
    else {
        armorHp = 0; // 普通僵尸无防具
        currentFolder = "Zombie_normal";
    }

    zombieMovie = new QMovie(":/res/images/Zombie/" + currentFolder + "/walk.gif");
    zombieMovie->start();

    // 帧同步刷新机制
    connect(zombieMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

    // 独立初始化各种音效对象（啃咬、被击中、防具被击中）
    eatSound = new QSoundEffect(this);
    eatSound->setSource(QUrl("qrc:/res/sound/chomp.wav"));
    eatSound->setVolume(0.5f);

    splatSound = new QSoundEffect(this);
    splatSound->setSource(QUrl("qrc:/res/sound/splat.wav"));
    splatSound->setVolume(0.7f);

    shieldHitSound = new QSoundEffect(this);
    shieldHitSound->setSource(QUrl("qrc:/res/sound/shieldhit.wav"));
    shieldHitSound->setVolume(0.7f);

    plasticHitSound = new QSoundEffect(this);
    plasticHitSound->setSource(QUrl("qrc:/res/sound/plastichit.wav"));
    plasticHitSound->setVolume(0.7f);

    // 僵尸的“心脏”：驱动位移与索敌的定时器
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &Zombie::move);
    moveTimer->start(30);
}

Zombie::~Zombie()
{
    // 安全的资源释放，防止内存泄漏
    if (zombieMovie) {
        zombieMovie->stop();
        delete zombieMovie;
    }
}

// 渲染包围盒，告诉 Qt 引擎这个图元的刷新范围
QRectF Zombie::boundingRect() const
{
    return QRectF(-100, -50, 200, 250);
}

// 【碰撞检测优化】：精确定义碰撞形状（AABB 碰撞框）
// 剔除贴图边缘的空白透明区域，使植物和子弹的判定更符合直觉
QPainterPath Zombie::shape() const
{
    QPainterPath path;
    path.addRect(-20, 0, 40, 130);
    return path;
}

void Zombie::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (zombieMovie && zombieMovie->isValid()) {
        QPixmap pix = zombieMovie->currentPixmap();
        QPixmap scaledPix = pix.scaledToHeight(140, Qt::SmoothTransformation);
        painter->drawPixmap(-scaledPix.width() / 2, 0, scaledPix);
    }
}

// 核心状态机轮转：移动、索敌与啃咬逻辑
void Zombie::move()
{
    // 死亡或被灰烬秒杀时，剥夺行动权
    if (state == Dead || state == Burned) return;

    if (state == Eating) {
        // 1. 啃咬状态下，获取当前重叠的所有实体
        QList<QGraphicsItem*> items = this->collidingItems();
        Plant* targetPlant = nullptr;

        for (QGraphicsItem* item : items) {
            // RTTI 运行时类型识别：安全地向下转型判断是否为植物
            Plant* p = dynamic_cast<Plant*>(item);
            if (p && p->getRow() == this->row) { // 必须在同一行
                targetPlant = p;
                break;
            }
        }

        if (targetPlant) {
            // 计时器降频处理：每 16 个 ticks (约0.5秒) 造成一次实际伤害
            static int eatCounter = 0;
            eatCounter++;
            if (eatCounter >= 16) {
                eatCounter = 0;
                targetPlant->takeDamage(20);
                eatSound->play();
            }
        }
        else {
            // 植物被吃掉后，状态机切换回移动状态 (判断是否断手)
            state = (hp <= 90) ? LostArm : Normal;
            QString armStr = (state == LostArm) ? "2" : "";
            zombieMovie->stop();
            zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/walk" + armStr + ".gif");
            zombieMovie->start();
        }
        return;
    }

    // 执行位移
    this->moveBy(-speed, 0);

    // 2. 移动状态下进行索敌碰撞检测
    QList<QGraphicsItem*> items = this->collidingItems();
    for (QGraphicsItem* item : items) {
        Plant* p = dynamic_cast<Plant*>(item);
        if (p && p->getRow() == this->row) {
            // 碰到植物，状态机切换到“进食”
            state = Eating;
            zombieMovie->stop();
            zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/eat.gif");
            zombieMovie->start();
            return;
        }
    }

    // 3. 破门判定：当 X 坐标越过底线，触发游戏失败信号
    if (this->pos().x() < 160) {
        emit gameLost(this);
    }
}

// 伤害结算中心 (支持普通伤害与灰烬秒杀)
void Zombie::takeDamage(int damage, bool isFire)
{
    if (state == Dead || state == Burned) return;

    // 【防具结算逻辑】
    if (armorHp > 0) {
        armorHp -= damage;

        // 根据防具类型播放对应的受击音效
        if (type == ConeheadZombie) plasticHitSound->play();
        else if (type == BucketheadZombie) shieldHitSound->play();

        if (armorHp <= 0) {
            // 防具被击碎，伤害溢出部分算作肉体伤害
            damage = -armorHp;
            armorHp = 0;
            currentFolder = "Zombie_normal"; // 降级为普通僵尸模型

            zombieMovie->stop();
            // 保持当前动作逻辑换皮（无缝切换动画）
            if (state == Eating) {
                zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/eat.gif");
            }
            else {
                QString armStr = (hp <= 90) ? "2" : "";
                zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/walk" + armStr + ".gif");
            }
            zombieMovie->start();
        }
        else {
            damage = 0; // 伤害被防具完全吸收
        }
    }
    else {
        splatSound->play(); // 肉体受击音效
    }

    if (damage > 0) {
        hp -= damage;
    }

    // 受击白光闪烁效果 (利用 Qt 的 GraphicsEffect)
    QGraphicsColorizeEffect* flashEffect = new QGraphicsColorizeEffect();
    flashEffect->setColor(Qt::white);
    flashEffect->setStrength(0.8);
    this->setGraphicsEffect(flashEffect);
    QTimer::singleShot(100, this, [this]() { this->setGraphicsEffect(nullptr); });

    // 【死亡结算机制】
    if (hp <= 0 && isFire) {
        // 1. 被樱桃炸弹等灰烬植物秒杀
        state = Burned;
        speed = 0;
        moveTimer->stop();
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Burn.gif");
        zombieMovie->start();
        // 延时销毁对象，确保动画播完
        QTimer::singleShot(2500, this, [this]() { this->deleteLater(); });
        return;
    }

    if (hp <= 90 && state == Normal) {
        // 2. 生命值极低，触发断手状态
        state = LostArm;
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/walk2.gif");
        zombieMovie->start();
    }
    else if (hp <= 0) {
        // 3. 常规死亡：播放倒地动画与掉头特效
        state = Dead;
        speed = 0;
        moveTimer->stop();

        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/death.gif");
        zombieMovie->start();

        // 动态生成一个掉头图元覆盖在原位置上方
        QLabel* headLabel = new QLabel();
        headLabel->setAttribute(Qt::WA_TranslucentBackground);
        QMovie* headMovie = new QMovie(":/res/images/ZombieHead.gif");
        headLabel->setMovie(headMovie);
        headMovie->start();

        QGraphicsProxyWidget* headProxy = this->scene()->addWidget(headLabel);
        headProxy->setPos(this->pos().x() + 20, this->pos().y() - 10);
        headProxy->setZValue(this->zValue() + 1);

        // 统一在 2.5 秒后安全清理内存
        QTimer::singleShot(2500, this, [this, headProxy]() {
            headProxy->deleteLater();
            this->deleteLater();
            });
    }
}

// 核心控制接口：响应游戏暂停（时空冻结）
void Zombie::pauseBehavior()
{
    if (moveTimer && moveTimer->isActive()) {
        moveTimer->stop();
    }
    if (zombieMovie) {
        zombieMovie->setPaused(true);
    }
}

// 核心控制接口：响应游戏恢复
void Zombie::resumeBehavior()
{
    if (state == Dead || state == Burned) return; // 死亡实体不恢复动作

    if (moveTimer && !moveTimer->isActive()) {
        moveTimer->start(30);
    }
    if (zombieMovie && zombieMovie->state() == QMovie::Paused) {
        zombieMovie->setPaused(false);
    }
}