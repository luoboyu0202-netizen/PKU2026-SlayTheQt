#include "BashCard.h"
#include "../relics/RelicManager.h"
#include "../StatusManager.h"
#include "../Fighter.h"

void BashCard::play(Player* source, Fighter* target, RelicManager* relics) {
    if (target) {
        int baseDamage = 8; // 痛击的基础伤害是 8

        // 1. 走状态管道：计算力量、虚弱、易伤等修饰
        int finalDamage = StatusManager::calculateDamage(source, target, baseDamage);

        // 2. 走遗物管道：计算钢笔尖等翻倍效果
        if (relics) {
            finalDamage = relics->modifyAttackDamage(finalDamage);
        }

        // 3. 狠狠砸下去！
        target->takeDamage(finalDamage);

        // =======================================================
        // 🔴【痛击的灵魂】：给目标挂上 2 层易伤！
        // =======================================================
        if (target->getStatusManager()) {
            target->getStatusManager()->applyStatus(StatusType::Vulnerable, 2);
        }
    }
}
