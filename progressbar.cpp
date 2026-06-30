#include "progressbar.h"

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent), currentProgress(0), maxProgress(100)
{
    // ✅ 核心修复：FlagMeter.png 是一张精灵图！
    // 上半截是黑槽底图，下半截是满进度绿条。我们用 copy() 把它劈成两半！
    QPixmap rawMeter(":/res/images/FlagMeter.png");
    int h = rawMeter.height() / 2;
    int w = rawMeter.width();

    bgImg = rawMeter.copy(0, 0, w, h);        // 截取上半截作为底框
    fillImg = rawMeter.copy(0, h, w, h);      // 截取下半截作为绿条

    QPixmap rawParts(":/res/images/FlagMeterParts.png");
    headImg = rawParts.copy(0, 0, 35, 35);    // 截取僵尸头

    this->setFixedSize(w, h);
}

void ProgressBar::setProgress(int current, int max) {
    currentProgress = current;
    maxProgress = max;
    this->update();
}

void ProgressBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    // 1. 画黑槽底图
    painter.drawPixmap(0, 0, bgImg);

    if (maxProgress > 0) {
        if (currentProgress > maxProgress) currentProgress = maxProgress;
        double ratio = (double)currentProgress / maxProgress;

        int fillWidth = fillImg.width() * ratio;

        // 2. 完美嵌入：从绿条的右侧截取，画在底框的右侧
        QRect sourceRect(fillImg.width() - fillWidth, 0, fillWidth, fillImg.height());
        QRect targetRect(bgImg.width() - fillWidth, 0, fillWidth, bgImg.height());

        painter.drawPixmap(targetRect, fillImg, sourceRect);

        // 3. 画僵尸头游标，死死钉在绿条最左侧（且在 Y 轴居中）
        int headX = targetRect.left() - (headImg.width() / 2) + 5;
        int headY = targetRect.center().y() - (headImg.height() / 2);
        painter.drawPixmap(headX, headY, headImg);
    }
}