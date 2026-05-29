#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include "api/BattleAPI.h"
#include <QParallelAnimationGroup> // 🔴【新增】：并行播放特效
#include "CardDraftOverlay.h"

// ==========================================
// 🎟️ 组件：单个战利品条目按钮 (高度复用！)
// ==========================================
class RewardItemButton : public QPushButton {
    Q_OBJECT
public:
    enum RewardType { Gold, Relic, Card };

    // 构造函数
    explicit RewardItemButton(RewardType type, const QString& text, QWidget* parent = nullptr);

    RewardType getType() const { return m_type; }

    // 灵活的数据挂载点
    int goldAmount = 0;
    QString relicId = "";

private:
    RewardType m_type;
};

class RewardScreen : public QWidget {
    Q_OBJECT
public:
    explicit RewardScreen(QWidget *parent = nullptr);
    void loadRewards(const BattleResult& result);

    // 🔴【新增】：华丽登场的方法！
    void dropDown();

signals:
    void proceedRequested();

    // 🔴【新增】：幽灵粒子安全抵达停机坪的信号！
    void relicFlightFinished(QString relicId);
    void goldFlightFinished(int amount);
    void deckUpdated();

private slots:
    void onRewardItemClicked();
    void onProceedClicked();

private:
    QWidget* m_boardWidget;       // ⬛ 核心组件：那块砸下来的黑板！
    QVBoxLayout* m_listLayout;
    QPushButton* m_proceedButton;
    // 在 private 区域加入：
    CardDraftOverlay* m_draftOverlay;
    RewardItemButton* m_pendingCardButton; // 记录是哪个按钮触发了抽卡

    QPropertyAnimation* m_dropAnimation; // ⏱️ 控制“砸下+抖动”的引擎
    void animateAndRemoveItem(RewardItemButton* btn);


};