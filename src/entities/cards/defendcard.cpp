#include "DefendCard.h"
#include "../entities/Player.h"
#include "../entities/Fighter.h"

DefendCard::DefendCard(QObject* parent)
    : Card("defend", QStringLiteral("防御"), 1, false, parent) {
}

void DefendCard::play(Player* source, Fighter* target) {
    Q_UNUSED(target); // 消除未使用参数 'target' 的警告喵！

    if (source) {
        // 基础格挡为 5
        source->addBlock(5);
    }
}
