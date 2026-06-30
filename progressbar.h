#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPainter>

class ProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit ProgressBar(QWidget* parent = nullptr);
    void setProgress(int current, int max);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap bgImg;
    QPixmap fillImg;
    QPixmap headImg;
    int currentProgress;
    int maxProgress;
};