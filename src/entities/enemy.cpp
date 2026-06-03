#include "Enemy.h"
#include <QDebug>

// ========================================================
// 🧬 构造函数：极致纯净的肉体初始化
// ========================================================
Enemy::Enemy(const QString& name, int maxHp, const QString& imagePath, QObject* parent)
    : Fighter(name, maxHp, parent), m_imagePath(imagePath)
{
    // 🔴 历史包袱全部卸下！
    // 意图相关的变量 (m_currentIntent, m_moveHistory) 已在头文件自动初始化
}

// ========================================================
// 🧠 虚拟大脑中枢：基类的默认实现
// ========================================================
void Enemy::rollNextIntent() {
    // ⚠️ 警告：因为我们砍掉了旧版的死板序列，基类不再负责逻辑推演。
    // 这里只作为 UI 广播的「兜底发射塔」！
    //
    // 💡 最佳实践：
    // 未来的子类（如 JawWorm, Slime）在重写此函数时，
    // 算出新的 m_currentIntent 后，可以在最后一行调用 Enemy::rollNextIntent();
    // 这样就能自动刷新头顶的意图 UI 啦喵！

    if (m_currentIntent.type != IntentType::Unknown) {
        emit intentChanged(m_currentIntent.type, m_currentIntent.value);
    } else {
        qDebug() << "[Enemy-AI] 🚨 警告：" << m_name << " 还没有决定好意图！请检查子类是否正确重写了 AI！";
    }
}

// ========================================================
// 🛡️ 护甲系统：绝对稳固的防御壁垒
// ========================================================
void Enemy::addBlock(int b) {
    if (b > 0) {
        // m_block 继承自 Fighter，这里直接修改
        m_block += b;

        qDebug() << "[Enemy]" << m_name << "获得了" << b << "点格挡！当前总格挡:" << m_block;

        // 🔴 呼叫 UI 舞台，画出盾牌！
        emit blockChanged(m_block);
    }
}

void Enemy::loseBlock() {
    if (m_block > 0) {
        m_block = 0;

        qDebug() << "[Enemy]" << m_name << "的回合开始，失去所有残存格挡。";

        // 🔴 呼叫 UI 舞台，粉碎盾牌！
        emit blockChanged(m_block);
    }
}