#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

// 引入你的各个界面图纸
#include "map/MapManager.h"
#include "map/TitleMenuView.h"
#include "api/BattleLauncher.h"
#include "ui/RewardScreen.h"

class GameWindow : public QWidget {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);

private slots:
    // 🔴【新增】：处理开始界面的黑场转场动画
    void handleStartGameTransition();

    // 接收地图的“开战”请求
    void onBattleRequested(const MapNode& node);
    // 接收战斗结束的战报
    void onBattleConcluded(BattleResult result);
    // 接收玩家在战利品界面点击“继续”的信号
    void onRewardProceedRequested();

private:
    // ==========================================
    // 📺 核心界面容器与频道
    // ==========================================
    QStackedWidget* m_stack;         // 我们的“电视机”
    TitleMenuView* m_titleView;      // 🎬 频道 0：开始界面
    MapManager* m_mapManager;        // 🗺️ 频道 1：大地图

    // ==========================================
    // ⚔️ 战斗与战利品系统
    // ==========================================
    BattleLauncher* m_launcher;      // 当前负责战斗的管家
    BattleView* m_currentBattleView; // 当前的战斗画面
    RewardScreen* m_rewardScreen;    // 🎁 战利品悬浮层

    MapNode m_lastClickedNode;       // 记住刚刚点了哪个节点，为了打赢后解锁

    // ==========================================
    // ⬛ 全局统一的黑场转场动画组件 (完美复用)
    // ==========================================
    QWidget* m_curtain;                      // 物理黑幕
    QGraphicsOpacityEffect* m_curtainEffect; // 控制黑幕透明度的特效
    QPropertyAnimation* m_fadeAnimation;     // 控制渐变时间的引擎
};