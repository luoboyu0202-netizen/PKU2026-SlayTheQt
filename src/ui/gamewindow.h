#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "map/MapManager.h"
#include "api/BattleLauncher.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "ui/RewardScreen.h" // 🔴 引入战利品界面

class GameWindow : public QWidget {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);

private slots:
    // 接收地图的“开战”请求
    void onBattleRequested(const MapNode& node);
    // 接收战斗结束的战报
    void onBattleConcluded(BattleResult result);

    // 🔴【新增】：接收玩家在战利品界面点击“继续”的信号
    void onRewardProceedRequested();

private:
    QStackedWidget* m_stack;    // 📺 我们的“电视机”
    MapManager* m_mapManager;   // 🗺️ 频道 0：大地图
    BattleLauncher* m_launcher; // ⚔️ 当前负责战斗的管家
    BattleView* m_currentBattleView; // 当前的战斗画面

    // 🔴【新增】：固定持有一个战利品频道实例
    RewardScreen* m_rewardScreen;

    MapNode m_lastClickedNode;  // 记住刚刚点了哪个节点，为了打赢后解锁
    QWidget* m_curtain;                      // ⬛ 我们的物理黑幕
    QGraphicsOpacityEffect* m_curtainEffect; // 👻 控制黑幕透明度的魔法
    QPropertyAnimation* m_fadeAnimation;     // ⏱️ 控制渐变时间的引擎
};
