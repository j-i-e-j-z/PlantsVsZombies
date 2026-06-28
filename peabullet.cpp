#include "peabullet.h"
#include "zombie.h"   // 引入僵尸头文件，让子弹能认识它
#include <QTimer>
#include <QPainter>
#include <QList>
#include <QSoundEffect>
#include <QUrl>

PeaBullet::PeaBullet(int r, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r)
{
    // 加载豌豆子弹图片，请确保你的资源库里有 Pea.png
    bulletImage = QPixmap(":/res/images/Pea.png");

    QTimer* flyTimer = new QTimer(this);
    connect(flyTimer, &QTimer::timeout, [this]() {
        // 1. 每 30 毫秒往前飞 15 个像素
        this->moveBy(15, 0);

        // 2. 如果飞出屏幕外，自动销毁，释放内存
        if (this->pos().x() > 1050) {
            this->deleteLater();
            return;
        }

        // ==========================================================
        // 💥 【核心大招：碰撞检测】
        // ==========================================================
        // 获取当前和子弹发生碰撞的所有图形实体
        // 获取当前和子弹发生碰撞的所有图形实体
        QList<QGraphicsItem*> items = this->collidingItems();
        for (QGraphicsItem* item : items) {
            Zombie* zombie = dynamic_cast<Zombie*>(item);

            if (zombie && zombie->getRow() == this->row) {
                zombie->takeDamage(20);

                // =========================================================
                // 🎵 【新增：子弹击中僵尸音效】
                // =========================================================
                QSoundEffect* splatSound = new QSoundEffect(); // 这里不要传 this，因为子弹马上要 deleteLater 了
                splatSound->setSource(QUrl("qrc:/res/sound/PeaHit.wav")); // ⚠️ 检查你的击中音效文件名
                splatSound->setVolume(0.6f);
                splatSound->play();

                // 播放完毕后自动回收音效内存
                QObject::connect(splatSound, &QSoundEffect::playingChanged, [splatSound]() {
                    if (!splatSound->isPlaying()) {
                        splatSound->deleteLater();
                    }
                    });

                this->deleteLater();    // 子弹销毁
                return;
            }
        }
        });
    flyTimer->start(30);
}

// 析构函数
PeaBullet::~PeaBullet()
{
}

// 物理碰撞边界 (宽高24左右，契合豌豆的大小)
QRectF PeaBullet::boundingRect() const
{
    return QRectF(0, 0, 24, 24);
}

// 渲染画图逻辑
void PeaBullet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->drawPixmap(0, 0, 24, 24, bulletImage);
}