#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

// ==========================================
// 📦 引入各个界面图纸 (完美融合了你和队友的心血)
// ==========================================
#include "map/MapManager.h"
#include "map/TitleMenuView.h"
#include "api/BattleLauncher.h"
#include "ui/RewardScreen.h"
#include "ui/TopBar.h"
#include "ui/RelicTray.h"
#include <functional> // 記得引入 functional


class GameWindow : public QWidget {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

public:
    // 在 GameWindow 中新增这个函数
    void refreshTopBarRelics();

    // 🔴 暴露图标在窗口中的全局坐标（供飞行动画定位）
    QPointF deckPileGlobalPos() const;
    QPointF goldIconGlobalPos() const;

    // 全域粒子流星大砲！
    // type: "Relic" (藍色), "Card" (紫色), "Gold" (金色)
    void playGlobalParticleEffect(QPointF startPos, QPointF endPos, const QString& type, std::function<void()> onLanded);

    void enterCampfireEvent();
    void enterMerchantEvent(); // 🔴 新增：进入商店的专属通道！
    void enterChestEvent();
    void enterQuestionMarkEvent(const MapNode& node);
    // 全域流星特效大砲：傳入圖片路徑、起點、終點、以及抵達後要執行的回呼函數
    void playLootMeteor(const QString& imagePath, QPoint startPos, QPoint endPos, std::function<void()> onLanded);

private slots:
    // 🎬【队友新增】：处理开始界面的黑场转场动画
    void handleStartGameTransition();
    // ==========================================
    // 💾 【新增】：处理读档和存档退出的转场
    // ==========================================
    void handleContinueGameTransition(); // 接收 TitleView 的读档开局
    void handleReturnToTitle();          // 接收 TopBar 的保存并退出

    // 接收地图的“开战”请求
    void onMapNodeClicked(const MapNode& node);
    // 接收战斗结束的战报
    void onBattleConcluded(BattleResult result);
    // 接收玩家在战利品界面点击“继续”的信号
    void onRewardProceedRequested();

private:
    // ==========================================
    // 📺 核心界面容器与频道
    // ==========================================
    QStackedWidget* m_stack;         // 我们的“电视机”
    TitleMenuView* m_titleView;      // 🎬 频道 0：开始界面 (队友新增)
    MapManager* m_mapManager;        // 🗺️ 频道 1：大地图

    // ==========================================
    // ⚔️ 战斗与战利品系统
    // ==========================================
    BattleLauncher* m_launcher;      // 当前负责战斗的管家
    BattleView* m_currentBattleView; // 当前的战斗画面
    RewardScreen* m_rewardScreen;    // 🎁 战利品悬浮层 (删除了重复的声明！)

    // ==========================================
    // 👑 全局顶栏与遗物系统 (你的心血)
    // ==========================================
    TopBar* m_topBar;
    QGraphicsView* m_topBarView;
    RelicTray* m_globalRelicTray = nullptr;
    QList<Relic*> m_globalRelics;

    MapNode m_lastClickedNode;       // 记住刚刚点了哪个节点，为了打赢后解锁

    // ==========================================
    // ⬛ 全局统一的黑场转场动画组件 (完美复用)
    // ==========================================
    QWidget* m_curtain;                      // 物理黑幕
    QGraphicsOpacityEffect* m_curtainEffect; // 控制黑幕透明度的魔法
    QPropertyAnimation* m_fadeAnimation;     // 控制渐变时间的引擎

    // 🔴 【新增】：播放打败 Boss 后的终末之诗动画
    void playEndingAnimation();
};