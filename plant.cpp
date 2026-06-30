#include "plant.h"
#include <QGraphicsColorizeEffect>
#include <QSoundEffect> 
#include <QUrl>
#include <QDebug>

Plant::Plant(int r, int c, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), col(c), hp(300), maxHp(300)
{
    QSoundEffect* placeSound = new QSoundEffect();
    placeSound->setSource(QUrl("qrc:/res/sound/plant.wav")); // 使用经典种地音效
    placeSound->setVolume(0.8f);
    placeSound->play();
    connect(placeSound, &QSoundEffect::playingChanged, [placeSound]() {
        if (!placeSound->isPlaying()) placeSound->deleteLater();
        });

    actionTimer = new QTimer(this);

    plantMovie = new QMovie(this);
    connect(plantMovie, &QMovie::frameChanged, [this]() {
        this->update();
        });
}

Plant::~Plant() {}

QRectF Plant::boundingRect() const {
    return QRectF(-35, -45, 70, 90);
}

void Plant::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (plantMovie && plantMovie->isValid()) {
        QPixmap pix = plantMovie->currentPixmap();
        int imgW = pix.width();
        int imgH = pix.height();
        int drawX = -imgW / 2;
        int drawY = -imgH + yOffset;
        painter->drawPixmap(drawX, drawY, pix);
    }
}

void Plant::takeDamage(int damage) {
    hp -= damage;
    QGraphicsColorizeEffect* hurtEffect = new QGraphicsColorizeEffect();
    hurtEffect->setColor(Qt::red);
    hurtEffect->setStrength(0.5);
    this->setGraphicsEffect(hurtEffect);

    QTimer::singleShot(100, this, [this]() {
        this->setGraphicsEffect(nullptr);
        });

    if (hp <= 0) die();
}

void Plant::die() {
    if (actionTimer && actionTimer->isActive()) actionTimer->stop();
    this->hide();
    this->deleteLater();
}

// ====================================================================
// ✅ 新增：原生时空冻结响应，暂停时再也不会动了！
// ====================================================================
void Plant::pauseBehavior() {
    if (actionTimer && actionTimer->isActive()) actionTimer->stop();
    if (plantMovie && plantMovie->state() == QMovie::Running) plantMovie->setPaused(true);
}

void Plant::resumeBehavior() {
    if (actionTimer && !actionTimer->isActive()) actionTimer->start();
    if (plantMovie && plantMovie->state() == QMovie::Paused) plantMovie->setPaused(false);
}