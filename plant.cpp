#include "plant.h"
#include <QGraphicsColorizeEffect>
#include <QSoundEffect> 
#include <QUrl>
#include <QDebug>

Plant::Plant(int r, int c, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), col(c), hp(300), maxHp(300)
{
    // 🎵 统一播放种植音效
    QSoundEffect* placeSound = new QSoundEffect();
    placeSound->setSource(QUrl("qrc:/res/sound/Place.wav"));
    placeSound->setVolume(0.8f);
    placeSound->play();
    connect(placeSound, &QSoundEffect::playingChanged, [placeSound]() {
        if (!placeSound->isPlaying()) placeSound->deleteLater();
        });

    actionTimer = new QTimer(this);

    // ✅【终极修复】：在父类构造函数中真正分配内存，豌豆再也不会崩溃了！
    plantMovie = new QMovie(this);
    connect(plantMovie, &QMovie::frameChanged, [this]() {
        this->update();
        });
}

Plant::~Plant()
{
}

// 统一指定植物的大小边界 (规范为 70x90 瘦长网格，基于中心点对齐)
QRectF Plant::boundingRect() const
{
    return QRectF(-35, -45, 70, 90);
}

void Plant::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (plantMovie && plantMovie->isValid()) {
        QPixmap pix = plantMovie->currentPixmap();

        // =========================================================
        // 🎯 【精准控位】：彻底解决植物出格、高矮不齐的物理锚点算法
        // =========================================================
        int imgW = pix.width();
        int imgH = pix.height();

        // 1. X 轴：往左偏移图片宽度的一半，让植物的横向视觉中心死死锁在格子正中
        int drawX = -imgW / 2;

        // 2. Y 轴：由于每行格子的高度是固定的，我们让植物的“脚底”对准格子的相对底部
       
        int drawY = -imgH + yOffset;

        // 3. 完美的各就各位
        painter->drawPixmap(drawX, drawY, pix);


        // (可选测试线：取消注释可以在游戏里看到格子的中心原点，方便你微调上面的 40)
        // painter->setPen(Qt::blue);
        // painter->drawEllipse(QPoint(0,0), 3, 3); 
    }
}

void Plant::takeDamage(int damage)
{
    hp -= damage;

    // 受击红光闪烁
    QGraphicsColorizeEffect* hurtEffect = new QGraphicsColorizeEffect();
    hurtEffect->setColor(Qt::red);
    hurtEffect->setStrength(0.5);
    this->setGraphicsEffect(hurtEffect);

    QTimer::singleShot(100, this, [this]() {
        this->setGraphicsEffect(nullptr);
        });

    if (hp <= 0) {
        qDebug() << "【战场系统】一株植物被吃掉了！";
        die();
    }
}

void Plant::die()
{
    if (actionTimer->isActive()) {
        actionTimer->stop();
    }
    this->hide();
    this->deleteLater();
}