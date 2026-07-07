#include "cherrybomb.h"
#include "zombie.h"
#include <QGraphicsScene>
#include <QSoundEffect>
#include <QUrl>
#include <QTimer>
#include <QGraphicsPixmapItem>

CherryBomb::CherryBomb(int r, int c, QGraphicsItem* parent)
    : Plant(r, c, parent)
{
    hp = 300;
    maxHp = 300;
    yOffset = 40;

    // 载入变大变红的起爆动画
    plantMovie->setFileName(":/res/images/CherryBomb.gif");
    plantMovie->start();

    // 💡 【核心设计】：利用 QTimer 单次触发机制 (SingleShot)，实现非阻塞式的 1.2 秒延时起爆
    QTimer::singleShot(600, this, &CherryBomb::act);
}

CherryBomb::~CherryBomb() {}

void CherryBomb::act() {
    // 🛡️ 【内存安全防御】：防止在 1.2 秒内植物已被僵尸吃掉，提前销毁导致的野指针崩溃
    if (!this->scene()) return;

    int placeX = this->x();
    int placeY = this->y();

    // 🎵 1. 播放爆炸音效 (动态分配，播放完利用 Lambda 表达式回调自动销毁，防内存泄漏)
    QSoundEffect* boomSound = new QSoundEffect();
    boomSound->setSource(QUrl("qrc:/res/sound/explosion.wav"));
    boomSound->setVolume(1.0f);
    boomSound->play();
    connect(boomSound, &QSoundEffect::playingChanged, [boomSound]() {
        if (!boomSound->isPlaying()) boomSound->deleteLater();
        });

    // 🔥 2. 生成爆炸特效残影
    QGraphicsPixmapItem* boomGif = new QGraphicsPixmapItem(QPixmap(":/res/images/Boom.gif").scaled(150, 150));
    boomGif->setPos(placeX - 50, placeY - 50);
    boomGif->setZValue(999);
    this->scene()->addItem(boomGif);

    // 延时 1 秒后自动清理特效残影图元
    QTimer::singleShot(1000, [boomGif]() {
        if (boomGif->scene()) boomGif->scene()->removeItem(boomGif);
        delete boomGif;
        });

    // 💥 3. 【算法高光】：九宫格范围索敌与秒杀
    // 获取当前场景内所有图元，遍历寻找僵尸
    QList<QGraphicsItem*> items = this->scene()->items();
    for (auto item : items) {
        // RTTI 运行时类型识别：安全判断该图元是否为僵尸对象
        if (Zombie* z = dynamic_cast<Zombie*>(item)) {
            // 利用绝对距离 (abs) 实现简单的 3x3 范围判定（边界定为 160 像素）
            if (abs(z->x() - placeX) < 160 && abs(z->y() - placeY) < 160) {
                z->takeDamage(1800, true); // 触发带火焰属性的超高伤害（灰烬秒杀）
            }
        }
    }

    // 4. 起爆完成，清理自身对象
    this->die();
}