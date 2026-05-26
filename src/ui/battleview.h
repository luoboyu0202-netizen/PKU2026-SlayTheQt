#pragma once
#include <QGraphicsView>
#include "BattleScene.h"
#include "TopBar.h"
#include "RelicTray.h"
#include "EnemyItem.h"
#include "HandLayoutManager.h"
#include "PileItem.h"
#include "EndTurnButton.h"
#include "EnergyWidget.h"
#include "../logic/BattleEngine.h" // 引入大脑
#include "PlayerItem.h"
#include "ConfirmButton.h" // 记得引入头文件喵！


class BattleView : public QGraphicsView {
    Q_OBJECT
public:
    explicit BattleView(QWidget* parent = nullptr);
    virtual ~BattleView() = default;

    // 🟢【架构大师钦点接口】：一键灌注逻辑大脑，全自动激活全场数据线！
    void bindEngine(BattleEngine* engine);
    void bindRelics(RelicManager* relicManager);

    void playShuffleAnimation(); // 播放弃牌堆洗入抽牌堆的金色方块动画

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initStageInfrastructure(); // 拼装那些一成不变的 UI 骨架

    // ========================================================
    // 📐【全新加入】：舞台怪物绝对槽位排版几何常数
    // ========================================================
    static const int MONSTER_START_X = 920;  // 第一只怪物的 X 坐标起点
    static const int MONSTER_SPACING = 280;   // 每只怪物之间的横向间距
    static const int UNIFIED_BASE_Y  = 680;   // 所有怪物对齐的统一地平线 Y 坐标

    // 🔴 强制所有指针出生时必须是干干净净的 nullptr！
    PlayerItem* m_playerItem = nullptr;
    HandLayoutManager* m_layoutManager = nullptr;

    // 喵娘建议：把其他的 UI 指针也顺手都加上 = nullptr，比如：
    PileItem* m_drawPileUI = nullptr;
    PileItem* m_discardPileUI = nullptr;
    PileItem* m_exhaustPileUI = nullptr;
    BattleScene* m_scene;

    // 🛠️ 场景基础设施（不变元素完全封装）
    TopBar* m_topBar;
    RelicTray* m_relicTray;
    EndTurnButton* m_endTurnBtn;
    EnergyWidget* m_energyBall;

    // 动态怪物组件（因为每场战斗怪物可能不同，由 bindEngine 动态创建）
    QList<EnemyItem*> m_enemyItems;

    BattleEngine* m_engine;

    // 🔴 结界道具：暗色蒙版和提示文字
    QGraphicsRectItem* m_darkOverlay;
    QGraphicsTextItem* m_promptTextItem;

    // 引入原生或者自定义按钮
    ConfirmButton* m_confirmBtn;
    bool m_isConfirmValid = false; // 记录当前按钮是否可点
};
