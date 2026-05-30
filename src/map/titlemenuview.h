#pragma once
#ifndef TITLEMENUVIEW_H
#define TITLEMENUVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QPixmap>
#include <QPropertyAnimation>

class TitleMenuView : public QWidget {
    Q_OBJECT

public:
    explicit TitleMenuView(QWidget *parent = nullptr);
    ~TitleMenuView();
    // 在 TitleMenuView.h 中添加信号
    signals:
        void startGameRequested(); // 告诉外部：玩家点开始了！

protected:
    // 重写绘图事件，用于自适应绘制背景图片
    void paintEvent(QPaintEvent *event) override;
    // 事件过滤器，用于捕捉鼠标悬停 (Hover) 触发光效
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onStartGameClicked();
    void onExitGameClicked();

private:
    void initUI();
    void initLightEffects();
    void showHoverAnimation(QPushButton* button);
    void hideHoverAnimation(QPushButton* button);

    QVBoxLayout* m_mainLayout;
    QPushButton* m_btnContinue;
    QPushButton* m_btnStart;
    QPushButton* m_btnExit;

    // 存储每个按钮对应的“光效滑块”
    QMap<QPushButton*, QLabel*> m_lightEffects;

    bool m_hasSavedGame; // 存档状态标志
    QPixmap m_bgPixmap;  // 背景图片
};

#endif // TITLEMENUVIEW_H