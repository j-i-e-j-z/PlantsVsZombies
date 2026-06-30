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
    if (type == ConeheadZombie) {
        armorHp = 370;
        currentFolder = "Zombie_conehead";
    }
    else if (type == BucketheadZombie) {
        armorHp = 1100;
        currentFolder = "Zombie_buckethead";
    }
    else {
        armorHp = 0;
        currentFolder = "Zombie_normal";
    }

    zombieMovie = new QMovie(":/res/images/Zombie/" + currentFolder + "/walk.gif");
    zombieMovie->start();

    connect(zombieMovie, &QMovie::frameChanged, this, [this]() {
        this->update();
        });

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

QRectF Zombie::boundingRect() const
{
    return QRectF(-100, -50, 200, 250);
}

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

void Zombie::move()
{
    if (state == Dead || state == Burned) return;

    if (state == Eating) {
        QList<QGraphicsItem*> items = this->collidingItems();
        Plant* targetPlant = nullptr;

        for (QGraphicsItem* item : items) {
            Plant* p = dynamic_cast<Plant*>(item);
            if (p && p->getRow() == this->row) {
                targetPlant = p;
                break;
            }
        }

        if (targetPlant) {
            static int eatCounter = 0;
            eatCounter++;
            if (eatCounter >= 16) {
                eatCounter = 0;
                targetPlant->takeDamage(20);
                eatSound->play();
            }
        }
        else {
            state = (hp <= 90) ? LostArm : Normal;
            QString armStr = (state == LostArm) ? "2" : "";
            zombieMovie->stop();
            zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/walk" + armStr + ".gif");
            zombieMovie->start();
        }
        return;
    }

    this->moveBy(-speed, 0);

    QList<QGraphicsItem*> items = this->collidingItems();
    for (QGraphicsItem* item : items) {
        Plant* p = dynamic_cast<Plant*>(item);
        if (p && p->getRow() == this->row) {
            state = Eating;
            zombieMovie->stop();
            zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/eat.gif");
            zombieMovie->start();
            return;
        }
    }

    if (this->pos().x() < 160) {
        emit gameLost(this);
    }
}

void Zombie::takeDamage(int damage, bool isFire)
{
    if (state == Dead || state == Burned) return;

    if (armorHp > 0) {
        armorHp -= damage;

        if (type == ConeheadZombie) plasticHitSound->play();
        else if (type == BucketheadZombie) shieldHitSound->play();

        if (armorHp <= 0) {
            damage = -armorHp;
            armorHp = 0;
            currentFolder = "Zombie_normal";

            zombieMovie->stop();
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
            damage = 0;
        }
    }
    else {
        splatSound->play();
    }

    if (damage > 0) {
        hp -= damage;
    }

    QGraphicsColorizeEffect* flashEffect = new QGraphicsColorizeEffect();
    flashEffect->setColor(Qt::white);
    flashEffect->setStrength(0.8);
    this->setGraphicsEffect(flashEffect);
    QTimer::singleShot(100, this, [this]() { this->setGraphicsEffect(nullptr); });

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

    if (hp <= 90 && state == Normal) {
        state = LostArm;
        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/walk2.gif");
        zombieMovie->start();
    }
    else if (hp <= 0) {
        state = Dead;
        speed = 0;
        moveTimer->stop();

        zombieMovie->stop();
        zombieMovie->setFileName(":/res/images/Zombie/" + currentFolder + "/death.gif");
        zombieMovie->start();

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

void Zombie::pauseBehavior()
{
    if (moveTimer && moveTimer->isActive()) {
        moveTimer->stop();
    }
    if (zombieMovie) {
        zombieMovie->setPaused(true);
    }
}

// ✅ 新增：恢复僵尸行为
void Zombie::resumeBehavior()
{
    if (state == Dead || state == Burned) return;

    if (moveTimer && !moveTimer->isActive()) {
        moveTimer->start(30);
    }
    if (zombieMovie && zombieMovie->state() == QMovie::Paused) {
        zombieMovie->setPaused(false);
    }
}