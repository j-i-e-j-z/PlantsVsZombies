#include "plant.h"
#include <QGraphicsColorizeEffect>
#include <QSoundEffect> 
#include <QUrl>
#include <QDebug>

Plant::Plant(int r, int c, QGraphicsItem* parent)
    : QGraphicsObject(parent), row(r), col(c), hp(300), maxHp(300)
{
    // 初始化种下植物的音效
    QSoundEffect* placeSound = new QSoundEffect();
    placeSound->setSource(QUrl("qrc:/res/sound/plant.wav"));
    placeSound->setVolume(0.8f);
    placeSound->play();
    connect(placeSound, &QSoundEffect::playingChanged, [placeSound]() {
        if (!placeSound->isPlaying()) placeSound->deleteLater();
        });

    // 为派生类预备的动作定时器引擎 (如射击、产阳光)
    actionTimer = new QTimer(this);

    plantMovie = new QMovie(this);
    // 将动画帧的推进与当前图元的重绘绑定
    connect(plantMovie, &QMovie::frameChanged, [this]() {
        this->update();
        });
}

Plant::~Plant() {}

// 定义植物标准的物理碰撞与刷新包围盒
QRectF Plant::boundingRect() const {
    return QRectF(-35, -45, 70, 90);
}

void Plant::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (plantMovie && plantMovie->isValid()) {
        QPixmap pix = plantMovie->currentPixmap();
        // 底边居中对齐算法，通过 yOffset 动态适应不同植物的身高差异
        int drawX = -pix.width() / 2;
        int drawY = -pix.height() + yOffset;
        painter->drawPixmap(drawX, drawY, pix);
    }
}

// 多态基础：通用的受击扣血逻辑
void Plant::takeDamage(int damage) {
    hp -= damage;

    // 🎨 【渲染特效】：每次受击动态挂载一个红色的色彩滤镜
    QGraphicsColorizeEffect* hurtEffect = new QGraphicsColorizeEffect();
    hurtEffect->setColor(Qt::red);
    hurtEffect->setStrength(0.5);
    this->setGraphicsEffect(hurtEffect);

    // 0.1秒后自动卸载滤镜，形成“闪红”效果
    QTimer::singleShot(100, this, [this]() {
        this->setGraphicsEffect(nullptr);
        });

    if (hp <= 0) die();
}

void Plant::die() {
    // 死亡时必须先停掉所有的业务定时器，防止死后开火或产阳光
    if (actionTimer && actionTimer->isActive()) actionTimer->stop();
    this->hide();
    this->deleteLater(); // 委托 Qt 事件循环安全销毁该对象内存
}

// ⏸️ 暂停控制接口：阻断定时器与 GIF 动画，实现完美的时空冻结
void Plant::pauseBehavior() {
    if (actionTimer && actionTimer->isActive()) actionTimer->stop();
    if (plantMovie && plantMovie->state() == QMovie::Running) plantMovie->setPaused(true);
}

void Plant::resumeBehavior() {
    if (actionTimer && !actionTimer->isActive()) actionTimer->start();
    if (plantMovie && plantMovie->state() == QMovie::Paused) plantMovie->setPaused(false);
}