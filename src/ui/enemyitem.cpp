#include "EnemyItem.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPropertyAnimation>
#include "battleengine.h" // 确保包含大脑的头文件
#include <QToolTip>
#include <QGraphicsSceneHoverEvent> // 别忘了包含这个头文件喵！
#include <QRandomGenerator> // 🔴 记得加上这个！
#include <QHash>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include "enemies/HexaFlameItem.h"// 🔴 加上这句！把六火亡魂的火焰图纸拿过来！
#include <cmath>           // 确保有数学库算坐标喵！

// 🔴 1. 参数列表增加 int spriteYOffset
EnemyItem::EnemyItem(Enemy* logicEnemy, BattleEngine* engine, int spriteYOffset, QGraphicsItem* parent)
    // 🔴 2. 初始化列表里，加上 m_spriteYOffset(spriteYOffset) 喵！
    : QGraphicsObject(parent),
    m_logicEnemy(logicEnemy),
    m_engine(engine),
    m_spriteYOffset(spriteYOffset), // <--- 关键点！
    m_isTargeted(false) {

    // 初始化数据
    m_hp = logicEnemy->getHp();
    m_maxHp = logicEnemy->getMaxHp();
    m_block = logicEnemy->getBlock();

    // 🔴 3. 注意：这里要改成从大脑计算后的初始意图，防止第一回合又变空！
    m_intentType = logicEnemy->getIntentType();
    m_intentValue = logicEnemy->getIntentValue();

    connect(logicEnemy, &Enemy::hpChanged, this, [this](int cur, int max){
        m_hp = cur;
        m_maxHp = max;
        update();
    });

    // 1. 监听怪物的受击震动
    connect(m_logicEnemy, &Fighter::animationTakeDamage, this, &EnemyItem::playHitAnimation);

    // 2. 监听大管家的出招指令
    connect(m_engine, &BattleEngine::enemyActing, this, [this](Enemy* actor) {
        if (actor == m_logicEnemy) {
            playActionAnimation();
        }
    });

    connect(logicEnemy->getStatusManager(), &StatusManager::statusChanged, this, [this]() {
        update(); // 触发 paint() 重绘
    });
    // 如果你有传入 engine，也可以监听玩家的状态改变
    if (m_engine && m_engine->getPlayer()) {
        connect(m_engine->getPlayer()->getStatusManager(), &StatusManager::statusChanged, this, [this]() {
            update();
        });
    }

    connect(logicEnemy, &Enemy::blockChanged, this, [this](int blk){ m_block = blk; update(); });

    // 🔴 1. 极其重要：打开鼠标悬停感应开关！否则后面的事件根本不会触发！
    setAcceptHoverEvents(true);

    connect(m_engine, &BattleEngine::enemyIntentUpdated, this, [this](Enemy* enemy, Intent intent) {
        if (enemy != m_logicEnemy) return;
        m_intentType = intent.type;
        m_intentValue = intent.value;
        m_statusType = intent.statusType;
        m_statusValue = intent.statusValue;

        switch (m_intentType) {
        case IntentType::Attack:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害。").arg(m_intentValue);
            break;
        case IntentType::Defend:
            m_tooltipText = QStringLiteral("下回合 获得 %1 点格挡。").arg(m_intentValue);
            break;
        case IntentType::Debuff:
            m_tooltipText = QStringLiteral("下回合 对你施加 %1 层负面状态。").arg(m_intentValue);
            break;
        case IntentType::Buff:
            m_tooltipText = QStringLiteral("下回合 强化自己 %1 层。").arg(m_intentValue);
            break;
        case IntentType::AttackAndDebuff:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害，\n并施加 %2 层负面状态！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::AttackAndDefend:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害，\n并获得 %2 点格挡！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::DefendAndBuff:
            m_tooltipText = QStringLiteral("下回合 获得 %1 点格挡，\n并强化自己 %2 层！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::InsertStatus:
            m_tooltipText = QStringLiteral("下回合 往你的牌库中洗入 %1 张粘液牌！").arg(m_intentValue);
            break;
        case IntentType::Summon:
            m_tooltipText = QStringLiteral("下回合 这个怪物将要召唤一些仆从！");
            break;
        case IntentType::Curse:
            if (!intent.cardIdToInsert.isEmpty()) {
                m_tooltipText = QStringLiteral("下回合 将 %1 张可怕的诅咒洗入你的牌库！").arg(m_intentValue);
            } else {
                m_tooltipText = QStringLiteral("下回合 将要对你施加强大的负面效果！");
            }
            break;
        case IntentType::GroupBuff:
            m_tooltipText = QStringLiteral("下回合 使所有敌方单位强化 %1 层！").arg(m_intentValue);
            break;
        case IntentType::GroupDefend:
            m_tooltipText = QStringLiteral("下回合 给予所有敌方单位 %1 点格挡！").arg(m_intentValue);
            break;
        case IntentType::AttackAndInsertStatus:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害，\n并将 %2 张状态牌洗入弃牌堆！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        default:
            m_tooltipText = "";
            break;
        }
        update();
    });

    // ========================================================
    // 🔴【新信号】：监听怪物的状态变化！
    // ========================================================
    connect(logicEnemy->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType type, int amount) {

        if (amount > 0) {
            // 情况 A：这是个新状态，或者层数增加了
            if (!m_statusIcons.contains(type)) {
                // 1. 如果以前没有，新建一个图标！
                StatusIconItem* newIcon = new StatusIconItem(type, amount, this);

                // 2. 登记到记账本里！
                m_activeStatusList.append(type); // 保证后来的排在列表末尾
                m_statusIcons.insert(type, newIcon);
            } else {
                // 如果已经有了，直接更新它的数字就行，不需要重排版
                m_statusIcons[type]->setAmount(amount);
            }
        }
        else {
            // 情况 B：层数归零了，彻底清除！
            if (m_statusIcons.contains(type)) {
                StatusIconItem* deadIcon = m_statusIcons.take(type); // 从字典拔出
                m_activeStatusList.removeAll(type); // 从顺序表中剔除
                delete deadIcon; // 物理超度
            }
        }

        // 只要有新增或删除，就呼叫自动排版工人！
        layoutStatusIcons();
    });


    QString path = logicEnemy->getImagePath();

    // ========================================================
    // 🔴【诊断针 B】：加上更强的诊断逻辑喵！
    // ========================================================
    if (!path.isEmpty()) {
        qDebug() << "[Diagnostics - UI] Attempting to load image from:" << path;

        // 尝试加载图片
        bool loadSuccess = m_enemyPixmap.load(path);

        if (loadSuccess) {
            qDebug() << "[Diagnostics - UI] Image loaded SUCCESS!喵！";
        } else {
            // 如果失败了，这里会输出喵！
            qWarning() << "[Diagnostics - UI] Image loaded FAILED! Error for path:" << path;

            // 我们甚至可以检查一下路径里有没有奇怪的空格或字符喵！
            qWarning() << "[Diagnostics - UI] Path length:" << path.length();
        }
    } else {
        qWarning() << "[Diagnostics - UI] Image path is empty喵！";
    }

    // ========================================================
    // 🌊【水波呼吸动画】：让意图图标永远上下浮动！
    // ========================================================
    QPropertyAnimation* floatAnim = new QPropertyAnimation(this, "intentFloatOffset", this);

    // 🔴 魔法一【错开频率】：让每个怪物的呼吸周期在 1800ms 到 2200ms 之间随机变动！
    // 这样它们不仅起步不一样，而且就算碰巧同步了，过一会儿也会再次错开，极其自然！
    int randomDuration = QRandomGenerator::global()->bounded(2200, 2600);
    floatAnim->setDuration(randomDuration);

    // 设定轨迹：0 -> 往上飘 15 像素 -> 回到 0
    floatAnim->setStartValue(0.0);
    floatAnim->setKeyValueAt(0.5, -20.0);
    floatAnim->setEndValue(0.0);

    floatAnim->setLoopCount(-1);
    floatAnim->setEasingCurve(QEasingCurve::InOutSine);

    // 🔴 必须先调用 start，让动画引擎接管！
    floatAnim->start();

    // 🔴 魔法二【错开起步】：在它刚开始跑的一瞬间，强行把进度条往前拨动一个随机时间！
    // 范围是 0 到它自己的周期长度
    int randomPhase = QRandomGenerator::global()->bounded(randomDuration);
    floatAnim->setCurrentTime(randomPhase);

    // ========================================================
    // 🔴【核心修复 5】：怪物的状态和意图双向绑定！
    // ========================================================
    connect(m_logicEnemy->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType type, int amount) {
        Q_UNUSED(type); Q_UNUSED(amount);

        // 只要怪物状态变了（比如吃到了镣铐），立刻触发重绘！
        this->update();

        // ========================================================
        // 🔮 监听点火协议：大管家只要一修改状态，这里立刻点火！
        // ========================================================
        if (type == StatusType::HexaLevel && m_hexaFlames.size() == 6) {
            for (int i = 0; i < 6; ++i) {
                // 如果当前索引小于拥有的层数，则点燃它！
                // 比如 amount = 2，那么 0 号位和 1 号位就会爆燃！
                m_hexaFlames[i]->setIgnited(i < amount);
            }
        }

    });

    // 💡 额外保险：如果你想让怪物在主角挂上易伤时，意图数字也跟着暴涨，
    // 把怪物也连进刚才 BattleView 里的那个“全局状态广播局”里，让怪物一起 update()！
    // 💀 监听死亡宣告
    connect(m_logicEnemy, &Fighter::died, this, [this](Fighter*) {
        playDeathAnimation();
    });

    // ========================================================
    // 🔮【极坐标法阵】：如果当前怪物是六火亡魂，布置六团冥火！
    // ========================================================
    if (m_logicEnemy->getId() == "Hexaghost") {
        int radius = 140; // 火焰环绕的半径，可根据你 Boss 贴图的实际大小微调

        for (int i = 0; i < 6; ++i) {
            HexaFlameItem* flame = new HexaFlameItem(this);

            // 1. 计算角度：从正上方（-90度）开始，顺时针每 60 度一个
            double angleDeg = -120.0 + (i * 60.0);

            // 2. 角度转弧度 (极坐标核心公式)
            double angleRad = angleDeg * M_PI / 180.0;

            // 3. 映射到直角坐标系 (x = r*cosθ, y = r*sinθ)
            double fx = radius * std::cos(angleRad);
            // 减去 m_spriteYOffset 确保火焰阵列和 Boss 肉体在同一个水平高度
            double fy = radius * std::sin(angleRad) - m_spriteYOffset - 100;

            flame->setPos(fx, fy-40);
            // 这里我们把生成的指针存进一个 QList<HexaFlameItem*> m_hexaFlames 里
            // 🔴 记得在 EnemyItem.h 的 private: 里加上 QList<HexaFlameItem*> m_hexaFlames;
            m_hexaFlames.append(flame);
        }
    }
}


EnemyItem::~EnemyItem() {
    qDebug() << "[🚨 探针一] 警报！EnemyItem 被彻底销毁了！名字:" << (m_logicEnemy ? m_logicEnemy->getName() : "未知");
}

// ========================================================
// 🔴【自动排版工人】：永远把图标按先来后到从左到右摆放！
// ========================================================
void EnemyItem::layoutStatusIcons() {
    // 让状态栏和我们拉长后的血条左侧（-75）完美对齐
    int startX = -75;

    // 🔴 核心微调：名字现在渲染在 Y=41 附近，高度 25。
    // 我们把状态图标的起点定在 Y = 74，能产生非常规整的排版间距！
    int startY = 74;

    for (int i = 0; i < m_activeStatusList.size(); ++i) {
        StatusType currentType = m_activeStatusList[i];
        StatusIconItem* icon = m_statusIcons[currentType];

        // 每个图标间隔 36 像素平铺
        icon->setPos(startX + (i * 36), startY);
    }
}

// ========================================================
// 📐 1. 扩容物理边界（为了防止更宽的怪物被裁剪，横向加大空间）
// ========================================================
QRectF EnemyItem::boundingRect() const {
    // 🔴 画布整体扩大！往上拉伸到 -400，宽度给 500！
    return QRectF(-250, -500, 600, 600);
}

// ========================================================
// 🎨 2. 具备“宽度自适应”与“极速缓存”的重绘管线
// ========================================================
void EnemyItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ========================================================
    // 🚀【性能核弹】：静态图片缓存池！
    // 利用 static 关键字，图片只会在第一次遇到时读取磁盘，
    // 之后哪怕每秒执行 60 次 paint，都是直接从内存中极速提取，0 性能消耗！
    // ========================================================
    static QHash<QString, QPixmap> iconCache;
    auto getCachedIcon = [](const QString& path) -> QPixmap {
        if (!iconCache.contains(path)) {
            iconCache.insert(path, QPixmap(path));
        }
        return iconCache.value(path);
    };

    // ========================================================
    // 🔴 【全新动态自适应核心算法】
    // ========================================================
    int baseHeight = 200;
    int enemyH = static_cast<int>(baseHeight * m_logicEnemy->getScaleFactor());
    int enemyW = enemyH;

    if (!m_enemyPixmap.isNull()) {
        qreal aspectRatio = static_cast<qreal>(m_enemyPixmap.width()) / m_enemyPixmap.height();
        enemyW = static_cast<int>(enemyH * aspectRatio);
    }

    // 🟢 A. 绘制自适应/拉伸后的怪物肉体
    if (!m_enemyPixmap.isNull()) {
        painter->drawPixmap(-enemyW / 2, -enemyH + m_spriteYOffset, enemyW, enemyH, m_enemyPixmap);
    } else {
        painter->setBrush(Qt::red);
        painter->drawRect(-enemyW / 2, -enemyH + m_spriteYOffset, enemyW, enemyH);
    }

    // ========================================================
    // 🔮【核心重构：即时推演前置！】
    // ========================================================
    int currentDisplayValue = m_intentValue;

    if (m_intentType == IntentType::Attack ||
        m_intentType == IntentType::AttackAndDebuff ||
        m_intentType == IntentType::AttackAndDefend||
        m_intentType ==IntentType::AttackAndInsertStatus) { // 👈 补上了 AttackAndBuff

        if (m_engine && m_engine->getPlayer()) {
            currentDisplayValue = StatusManager::calculateDamage(
                m_logicEnemy,
                m_engine->getPlayer(),
                m_logicEnemy->getCurrentIntent().value
                );
        }
    }

    // ========================================================
    // 🟢 B. 复合意图系统：利用极速缓存获取动态图标！
    // ========================================================
    QPixmap intentIcon;
    switch (m_intentType) {
    case IntentType::Attack:
        if (currentDisplayValue <= 5) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_1.png");
        } else if (currentDisplayValue <= 10) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_2.png");
        } else if (currentDisplayValue <= 15) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_3.png");
        } else if (currentDisplayValue <= 20) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_4.png");
        } else if (currentDisplayValue <= 25) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_5.png");
        } else if (currentDisplayValue <= 30) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_6.png");
        }
        break;

    case IntentType::AttackAndDebuff:
        if (currentDisplayValue <= 5) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_1.png");
        } else if (currentDisplayValue <= 10) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_2.png");
        } else if (currentDisplayValue <= 15) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_3.png");
        } else if (currentDisplayValue <= 20) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_4.png");
        } else if (currentDisplayValue <= 25) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_5.png");
        }
        break;

    case IntentType::AttackAndInsertStatus:
        if (currentDisplayValue <= 5) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_1.png");
        } else if (currentDisplayValue <= 10) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_2.png");
        } else if (currentDisplayValue <= 15) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_3.png");
        } else if (currentDisplayValue <= 20) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_4.png");
        } else if (currentDisplayValue <= 25) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_debuff_5.png");
        }
        break;

    case IntentType::AttackAndDefend:
        if (currentDisplayValue <= 5) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_defend_1.png");
        } else if (currentDisplayValue <= 10) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_defend_2.png");
        } else if (currentDisplayValue <= 15) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_defend_3.png");
        } else if (currentDisplayValue <= 20) {
            intentIcon = getCachedIcon(":/resources/images/intents/attack_defend_4.png");
        }
        break;
        
    case IntentType::Defend:          intentIcon = getCachedIcon(":/resources/images/intents/defend.png"); break;
    case IntentType::Debuff:          intentIcon = getCachedIcon(":/resources/images/intents/debuff.png"); break;
    case IntentType::Buff:            intentIcon = getCachedIcon(":/resources/images/intents/buff.png"); break;
    case IntentType::InsertStatus:    intentIcon = getCachedIcon(":/resources/images/intents/curse.png"); break;
    case IntentType::Summon:          intentIcon = getCachedIcon(":/resources/images/intents/unknown.png"); break; // 修复了原本指向 unknown 的问题喵
    case IntentType::DefendAndBuff:   intentIcon = getCachedIcon(":/resources/images/intents/defend_buff.png"); break;
    case IntentType::Curse:           intentIcon = getCachedIcon(":/resources/images/intents/curse.png"); break;
    case IntentType::GroupBuff:       intentIcon = getCachedIcon(":/resources/images/intents/buff.png"); break;
    case IntentType::GroupDefend:     intentIcon = getCachedIcon(":/resources/images/intents/defend.png"); break;
    default: break;
    }

    // ========================================================
    // 🔴 完美合成：微调高度，匹配缩小的图标
    // ========================================================
    int intentY = -enemyH - 60 + m_spriteYOffset + m_intentFloatOffset;
    int iconSize = 76;

    if (!intentIcon.isNull()) {
        painter->drawPixmap(-(iconSize / 2), intentY, iconSize, iconSize, intentIcon);

        if (m_intentType == IntentType::Attack ||
            m_intentType == IntentType::AttackAndDebuff ||
            m_intentType == IntentType::AttackAndDefend||
            m_intentType ==IntentType::AttackAndInsertStatus            ) { // 👈 补上了 AttackAndBuff

            painter->setPen(Qt::white);
            QFont intentFont("Arial", 14, QFont::Bold);
            painter->setFont(intentFont);

            int textX = (iconSize / 2) + 5;
            int textY = intentY + (iconSize / 2) + 6;

            QString baseStr = QString::number(currentDisplayValue);
            painter->drawText(textX, textY, baseStr);

            if (m_logicEnemy->getCurrentIntent().multiHitCount > 1) {
                QFontMetrics metrics(intentFont);
                int baseTextWidth = metrics.horizontalAdvance(baseStr);

                painter->setFont(QFont("Arial", 16, QFont::Bold));
                painter->drawText(textX + baseTextWidth + 2, textY,
                                  QString("x%1").arg(m_logicEnemy->getCurrentIntent().multiHitCount));
            }
        }
    }

    // ========================================================
    // 🟢 C. 绘制血条系统（🛑 绝对不加偏移量！）
    // ========================================================
    int hp = m_logicEnemy->getHp();
    int maxHp = m_logicEnemy->getMaxHp();
    int block = m_logicEnemy->getBlock();

    int barW = 150;
    int barH = 18;
    int barX = -barW / 2;
    int barY = 15;

    painter->setBrush(QColor(80, 20, 20));
    painter->setPen(Qt::NoPen);
    painter->drawRect(barX, barY, barW, barH);

    if (maxHp > 0) {
        qreal hpRatio = static_cast<qreal>(hp) / maxHp;
        int currentHpW = static_cast<int>(barW * hpRatio);
        painter->setBrush(QColor(220, 20, 60));
        painter->drawRect(barX, barY, currentHpW, barH);
    }

    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(barX, barY, barW, barH, Qt::AlignCenter, QString("%1/%2").arg(hp).arg(maxHp));

    if (block > 0) {
        int shieldSize = 24;
        int shieldX = barX - (shieldSize / 2) - 5;
        int shieldY = barY + (barH - shieldSize) / 2;

        painter->setBrush(QColor(41, 128, 185));
        painter->setPen(QPen(Qt::white, 1.5));
        painter->drawRoundedRect(QRectF(shieldX, shieldY, shieldSize, shieldSize), 4, 4);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 11, QFont::Bold));
        painter->drawText(QRectF(shieldX, shieldY + 1, shieldSize, shieldSize), Qt::AlignCenter, QString::number(block));
    }

    int nameY = barY + barH + 8;
    painter->setPen(Qt::white);
    painter->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
    painter->drawText(-150, nameY, 300, 25, Qt::AlignCenter, m_logicEnemy->getName());
}

// ========================================================
// 🖱️ 悬停事件与精准判定
// ========================================================
void EnemyItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {

    // // 🔴 临时调试：只要鼠标一指上去，它当场表演去世！
    // qDebug() << "[🚨 探针二] 鼠标强制触发死亡动画！";
    // playDeathAnimation();

    hoverMoveEvent(event);
}

void EnemyItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    if (m_tooltipText.isEmpty() || m_intentType == IntentType::Unknown) {
        QToolTip::hideText();
        return;
    }

    int iconSize = 76;
    int baseHeight = 200;
    int enemyH = static_cast<int>(baseHeight * m_logicEnemy->getScaleFactor());
    int intentY = -enemyH - 60 + m_spriteYOffset + m_intentFloatOffset;

    QRectF intentRect(-(iconSize / 2), intentY, iconSize, iconSize);
    intentRect.adjust(-10, -10, 10, 10);

    if (intentRect.contains(event->pos())) {
        QToolTip::showText(event->screenPos(), m_tooltipText);
    } else {
        QToolTip::hideText();
    }
}

void EnemyItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    QToolTip::hideText();
}

// ========================================================
// 💥 动画魔法 1：受击震动 (Hit Shake)
// ========================================================
void EnemyItem::playHitAnimation() {

    if (m_logicEnemy->isDead()) return;

    // 针对当前 Item 的 "pos" 属性创建一个动画
    QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(250); // 极速震动！0.25 秒完成

    QPointF startPos = this->pos();

    // 关键帧：左右快速抽搐！
    anim->setKeyValueAt(0.0, startPos);
    anim->setKeyValueAt(0.2, startPos + QPointF(15, 0));  // 往右偏
    anim->setKeyValueAt(0.4, startPos + QPointF(-15, 0)); // 往左偏
    anim->setKeyValueAt(0.6, startPos + QPointF(10, 0));  // 往右偏小一点
    anim->setKeyValueAt(0.8, startPos + QPointF(-10, 0)); // 往左偏小一点
    anim->setKeyValueAt(1.0, startPos);                   // 完美归位！

    anim->start(QAbstractAnimation::DeleteWhenStopped); // 播完自动销毁动画对象
}

// ========================================================
// 🗡️ 动画魔法 2：出招前扑 (Action Pounce)
// ========================================================
void EnemyItem::playActionAnimation() {
    QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(350); // 配合异步引擎的 600ms 延时，0.35 秒的演出刚刚好！

    QPointF startPos = this->pos();

    // 关键帧：迅速向左冲刺（靠近主角），然后缓慢退回
    anim->setKeyValueAt(0.0, startPos);

    // 🔴 魔法：OutCubic 能让它冲出去瞬间极快，0.3 的时候就冲到了最前面 (-40像素)
    anim->setKeyValueAt(0.3, startPos + QPointF(-40, 0));

    anim->setKeyValueAt(1.0, startPos); // 剩下来的时间缓慢退回原位

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ========================================================
// 💀 动画魔法 3：死亡消散 (Death Fade & Shake) - 终极稳定版
// ========================================================
void EnemyItem::playDeathAnimation() {
    qDebug() << "[🩻 动画诊断] 敌人死亡动画正式启动！名字:" << m_logicEnemy->getName();

    // 解除缓存，防止滤镜失效
    this->setCacheMode(QGraphicsItem::NoCache);

    m_intentType = IntentType::Unknown;
    m_tooltipText = "";
    QToolTip::hideText();
    update(); // 让头顶的大刀立刻消失

    QParallelAnimationGroup* deathGroup = new QParallelAnimationGroup(this);

    // [子动画 A]：极其剧烈的垂死挣扎
    QPropertyAnimation* shake = new QPropertyAnimation(this, "pos");
    shake->setDuration(600);
    QPointF startPos = this->pos();
    shake->setKeyValueAt(0.0, startPos);
    shake->setKeyValueAt(0.2, startPos + QPointF(20, -10));
    shake->setKeyValueAt(0.4, startPos + QPointF(-20, 15));
    shake->setKeyValueAt(0.6, startPos + QPointF(15, -20));
    shake->setKeyValueAt(0.8, startPos + QPointF(-10, 10));
    shake->setKeyValueAt(1.0, startPos);

    // ========================================================
    // 🔴 核心修复 2：使用专属透明度滤镜！绝对不会瞬间失效！
    // ========================================================
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    this->setGraphicsEffect(opacityEffect); // 将滤镜套在怪物身上

    QPropertyAnimation* fade = new QPropertyAnimation(opacityEffect, "opacity");
    fade->setDuration(600);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);

    // 把两个动画塞进组里
    deathGroup->addAnimation(shake);
    deathGroup->addAnimation(fade);

    // 彻底埋葬：动画结束后，安全隐藏并销毁
    connect(deathGroup, &QParallelAnimationGroup::finished, this, [this]() {
        this->hide();
        this->deleteLater();
    });

    // 启动！
    deathGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

// ========================================================
// ✂️ 精准切割：把 600x600 的巨无霸碰撞箱，削减为真实的肉体！
// ========================================================
QPainterPath EnemyItem::shape() const {
    QPainterPath path;

    // 1. 获取怪物真实的缩放肉体尺寸
    int baseHeight = 200;
    int enemyH = static_cast<int>(baseHeight * m_logicEnemy->getScaleFactor());
    int enemyW = enemyH;

    if (!m_enemyPixmap.isNull()) {
        qreal aspectRatio = static_cast<qreal>(m_enemyPixmap.width()) / m_enemyPixmap.height();
        enemyW = static_cast<int>(enemyH * aspectRatio);
    }

    // 🟢 区域 A：真实的怪物肉体图像区域
    path.addRect(-enemyW / 2, -enemyH + m_spriteYOffset, enemyW, enemyH);

    // 🟢 区域 B：底部的血条与护甲区域（让玩家鼠标指到血条也能查看意图）
    int barW = 150;
    int barH = 18;
    int barX = -barW / 2;
    int barY = 15;
    // 稍微给血条区域加一点点击宽容度 (向下延伸包住名字)
    path.addRect(barX - 20, barY - 10, barW + 40, barH + 40);

    // 🟢 区域 C：头顶的意图气泡区域（精准包裹 76x76 的图标）
    int intentY = -enemyH - 60 + m_spriteYOffset + m_intentFloatOffset;
    int iconSize = 76;
    path.addRect(-(iconSize / 2), intentY, iconSize, iconSize);

    return path;
}