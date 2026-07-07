#include "peabullet.h"
#include "zombie.h"   
#include <QTimer>
#include <QPainter>
#include <QList>
#include <QSoundEffect>
#include <QUrl>

PeaBullet::PeaBullet(int r, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r)
{
    bulletImage = QPixmap(":/res/images/Pea.png");

    QTimer* flyTimer = new QTimer(this);
    connect(flyTimer, &QTimer::timeout, [this]() {
        // 1. 每 30 毫秒往前飞 15 个像素（帧驱动）
        this->moveBy(15, 0);

        // 2. 🛡️ 【内存回收机制】：飞出屏幕外后，利用 deleteLater 安全释放内存，避免堆空间被无限撑爆
        if (this->pos().x() > 1050) {
            this->deleteLater();
            return;
        }

        // ==========================================================
        // 💥 碰撞检测核心逻辑
        // ==========================================================
        // 遍历当前与子弹发生包围盒重叠的所有对象
        QList<QGraphicsItem*> items = this->collidingItems();
        for (QGraphicsItem* item : items) {
            // 类型试探：如果是僵尸
            Zombie* zombie = dynamic_cast<Zombie*>(item);

            // 行道判定：只有同一行的目标才会造成伤害
            if (zombie && zombie->getRow() == this->row) {
                zombie->takeDamage(20);  //-20blood

                // 🎵 播放动态分配的受击音效
                QSoundEffect* splatSound = new QSoundEffect();
                splatSound->setSource(QUrl("qrc:/res/sound/PeaHit.wav"));
                splatSound->setVolume(0.6f);
                splatSound->play();

                QObject::connect(splatSound, &QSoundEffect::playingChanged, [splatSound]() {
                    if (!splatSound->isPlaying()) {
                        splatSound->deleteLater();
                    }
                    });

                // 击中目标后，子弹使命结束，销毁自身
                this->deleteLater();
                return;
            }
        }
        });
    flyTimer->start(30);
}

PeaBullet::~PeaBullet() {}

// 物理碰撞边界锁定为 24x24 像素的豌豆本体
QRectF PeaBullet::boundingRect() const {
    return QRectF(0, 0, 24, 24);
}

void PeaBullet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->drawPixmap(0, 0, 24, 24, bulletImage);
}