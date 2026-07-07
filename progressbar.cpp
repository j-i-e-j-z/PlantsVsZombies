#include "progressbar.h"

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent), currentProgress(0), maxProgress(100)
{
    // ✂️ 【图形学操作】：利用精灵图裁切技术，复用单张纹理图片降低 IO 开销
    QPixmap rawMeter(":/res/images/FlagMeter.png");
    int h = rawMeter.height() / 2;
    int w = rawMeter.width();

    bgImg = rawMeter.copy(0, 0, w, h);        // 上半部切作黑底框
    fillImg = rawMeter.copy(0, h, w, h);      // 下半部切作绿色填充条

    QPixmap rawParts(":/res/images/FlagMeterParts.png");
    headImg = rawParts.copy(0, 0, 35, 35);    // 提取僵尸头游标

    this->setFixedSize(w, h);
}

void ProgressBar::setProgress(int current, int max) {
    currentProgress = current;
    maxProgress = max;
    this->update(); // 触发重绘
}

// 核心自绘逻辑
void ProgressBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    // 1. 画背景底槽
    painter.drawPixmap(0, 0, bgImg);

    if (maxProgress > 0) {
        if (currentProgress > maxProgress) currentProgress = maxProgress;
        double ratio = (double)currentProgress / maxProgress;

        int fillWidth = fillImg.width() * ratio;

        // 2. 【渲染映射】：计算绿条的裁剪与绘制矩形 (逆向填充，从右至左)
        QRect sourceRect(fillImg.width() - fillWidth, 0, fillWidth, fillImg.height());
        QRect targetRect(bgImg.width() - fillWidth, 0, fillWidth, bgImg.height());

        painter.drawPixmap(targetRect, fillImg, sourceRect);

        // 3. 计算僵尸头指示器的跟随坐标（居中对齐）
        int headX = targetRect.left() - (headImg.width() / 2) + 5;
        int headY = targetRect.center().y() - (headImg.height() / 2);
        painter.drawPixmap(headX, headY, headImg);
    }
}