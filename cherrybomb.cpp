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

    // 💡 樱桃炸弹种下后 1.2 秒自动起爆
    QTimer::singleShot(1200, this, &CherryBomb::act);
}

CherryBomb::~CherryBomb() {}

void CherryBomb::act() {
    // 确保对象还没被提前销毁（比如在起爆前被秒杀了）
    if (!this->scene()) return;

    int placeX = this->x();
    int placeY = this->y();

    // 🎵 1. 播放爆炸音效
    QSoundEffect* boomSound = new QSoundEffect();
    boomSound->setSource(QUrl("qrc:/res/sound/explosion.wav"));
    boomSound->setVolume(1.0f);
    boomSound->play();
    connect(boomSound, &QSoundEffect::playingChanged, [boomSound]() {
        if (!boomSound->isPlaying()) boomSound->deleteLater();
        });

    // 🔥 2. 生成爆炸特效残影
    QGraphicsPixmapItem* boomGif = new QGraphicsPixmapItem(QPixmap(":/res/images/Burn.gif").scaled(150, 150));
    boomGif->setPos(placeX - 50, placeY - 50);
    boomGif->setZValue(999);
    this->scene()->addItem(boomGif);

    // 1秒后自动清理特效残影
    QTimer::singleShot(1000, [boomGif]() {
        if (boomGif->scene()) boomGif->scene()->removeItem(boomGif);
        delete boomGif;
        });

    // 💥 3. 九宫格范围索敌与秒杀
    QList<QGraphicsItem*> items = this->scene()->items();
    for (auto item : items) {
        if (Zombie* z = dynamic_cast<Zombie*>(item)) {
            // 判断是否在 3x3 九宫格内 (绝对距离小于 160 像素)
            if (abs(z->x() - placeX) < 160 && abs(z->y() - placeY) < 160) {
                z->takeDamage(1800, true); // 触发灰烬秒杀判定
            }
        }
    }

    // 4. 清理自身
    this->die();
}