#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QLabel>
#include "cards/Card.h" // 引入你的卡牌实体类喵！

class CardDraftOverlay : public QWidget {
    Q_OBJECT
public:
    explicit CardDraftOverlay(QWidget* parent = nullptr);

    // 🔴 核心启动接口：传入刚才工厂摇出的 3 个卡牌 ID
    void showDraft(const QList<QString>& cardIds);

signals:
    void cardSelected(QString cardId);
    void returnRequested(); // 🟢 改名为请求返回信号

private slots:
    void onSkipClicked();
    void onCardClicked(QString cardId);

private:
    // 用来承载真实卡牌实体的“微型舞台”
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

    // 标题与跳过按钮
    QLabel* m_titleLabel;
    QPushButton* m_skipButton;

    // 记录这三张展示出来的实体牌，方便清理内存
    QList<Card*> m_draftCards;
};
