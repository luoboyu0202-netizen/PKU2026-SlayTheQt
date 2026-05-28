#pragma once
#include <QObject>
#include "BattleAPI.h"
#include "ui/BattleView.h"
#include "../logic/BattleEngine.h"

class BattleLauncher : public QObject {
    Q_OBJECT
public:
    explicit BattleLauncher(QObject* parent = nullptr);
    ~BattleLauncher();

    // 🚀【唯一暴露的启动接口】：外部只要调这个，画面就切进战斗！
    BattleView* launch(const BattleContext& context);

signals:
    // 📩【承诺的输出】：战斗结束时，将结果报告给外部系统！
    void battleConcluded(BattleResult result);

private:
    BattleView* m_view=nullptr;     // 舞台
    BattleEngine* m_engine=nullptr; // 大脑
};
