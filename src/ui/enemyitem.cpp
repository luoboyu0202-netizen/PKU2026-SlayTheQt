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

    // 绑定信号
    connect(logicEnemy, &Enemy::hpChanged, this, [this](int cur, int max){
        if (cur < m_hp) {
            // 【果汁感核心】：受到伤害，立刻左右抽搐震动！
            QPropertyAnimation* shake = new QPropertyAnimation(this, "x");
            qreal baseX = this->x();
            shake->setDuration(300);
            shake->setKeyValueAt(0, baseX);
            shake->setKeyValueAt(0.2, baseX - 15);
            shake->setKeyValueAt(0.4, baseX + 15);
            shake->setKeyValueAt(0.6, baseX - 15);
            shake->setKeyValueAt(0.8, baseX + 15);
            shake->setKeyValueAt(1, baseX);
            shake->start(QAbstractAnimation::DeleteWhenStopped);
        }
        m_hp = cur; m_maxHp = max; update();
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

    // 1. 找到你绑定 m_engine->enemyIntentUpdated 信号的地方，改成这样：
    connect(m_engine, &BattleEngine::enemyIntentUpdated, this, [this](Enemy* enemy, Intent intent) {
        if (enemy != m_logicEnemy) return; // 别忘了这句认领判定喵！
        m_intentType = intent.type;
        m_intentValue = intent.value;
        m_statusType = intent.statusType;
        m_statusValue = intent.statusValue;

        // ========================================================
        // 🟢【修复案件一】：根据意图类型，精准分拣数值！
        // ========================================================
        switch (m_intentType) {
        case IntentType::Attack:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害。").arg(m_intentValue);
            break;
        case IntentType::Defend:
            m_tooltipText = QStringLiteral("下回合 获得 %1 点格挡。").arg(m_intentValue);
            break;
        case IntentType::Debuff:
            // 🔴 纯 Debuff，层数在 m_intentValue 里！
            m_tooltipText = QStringLiteral("下回合 对你施加 %1 层负面状态。").arg(m_intentValue);
            break;
        case IntentType::Buff:
            // 🔴 纯 Buff，层数在 m_intentValue 里！
            m_tooltipText = QStringLiteral("下回合 强化自己 %1 层。").arg(m_intentValue);
            break;
        case IntentType::AttackAndDebuff:
            // 复合意图：%1 填伤害(m_intentValue)，%2 填状态层数(m_statusValue)
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害，\n并施加 %2 层负面状态！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::DefendAndBuff:
            // 复合意图：%1 填格挡(m_intentValue)，%2 填强化层数(m_statusValue)
            m_tooltipText = QStringLiteral("下回合 获得 %1 点格挡，\n并强化自己 %2 层！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::InsertStatus:
            // m_intentValue 代表塞几张，statusValue (可选)代表塞到哪里 (0=弃牌堆, 1=抽牌堆)
            m_tooltipText = QStringLiteral("下回合 往你的牌库中洗入 %1 张黏液牌！").arg(m_intentValue);
            break;
        case IntentType::Summon:
            m_tooltipText = QStringLiteral("下回合 這個怪物將要召喚一些僕從！");
            break;
            // 🔴【新增】：攻守兼備的提示！
        case IntentType::AttackAndDefend:
            m_tooltipText = QStringLiteral("下回合 对你造成 %1 点伤害，\n并获得 %2 点格挡！")
                                .arg(m_intentValue).arg(m_statusValue);
            break;
        case IntentType::Curse:
            // 可以根据是否带卡牌ID来动态改变文本喵！
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
        default:
            m_tooltipText = "";
            break;
        }

        update(); // 触发重画
    });

    connect(logicEnemy, &Enemy::died, this, [this](){ this->hide(); });

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
    });

    // 💡 额外保险：如果你想让怪物在主角挂上易伤时，意图数字也跟着暴涨，
    // 把怪物也连进刚才 BattleView 里的那个“全局状态广播局”里，让怪物一起 update()！
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
    return QRectF(-250, -400, 500, 500);
}

// ========================================================
// 🎨 2. 具备“宽度自适应”的重绘管线
// ========================================================
void EnemyItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ========================================================
    // 🔴 【全新动态自适应核心算法】
    // ========================================================
    int baseHeight = 200;
    // 引入怪物的专属缩放倍率！
    int enemyH = static_cast<int>(baseHeight * m_logicEnemy->getScaleFactor());
    int enemyW = enemyH; // 默认正方形

    if (!m_enemyPixmap.isNull()) {
        qreal aspectRatio = static_cast<qreal>(m_enemyPixmap.width()) / m_enemyPixmap.height();
        enemyW = static_cast<int>(enemyH * aspectRatio);
    }

    // ========================================================
    // 🟢 A. 绘制自适应/拉伸后的怪物肉体
    // ========================================================
    if (!m_enemyPixmap.isNull()) {
        painter->drawPixmap(-enemyW / 2, -enemyH + m_spriteYOffset, enemyW, enemyH, m_enemyPixmap);
    } else {
        painter->setBrush(Qt::red);
        painter->drawRect(-enemyW / 2, -enemyH + m_spriteYOffset, enemyW, enemyH);
    }

    // ========================================================
    // 🔮【核心重构：即时推演前置！】
    // 我们必须先算出怪物到底能打多少血，才能决定给它画多大的刀！
    // ========================================================
    int currentDisplayValue = m_intentValue;

    if (m_intentType == IntentType::Attack ||
        m_intentType == IntentType::AttackAndDebuff ||
        m_intentType == IntentType::AttackAndDefend) {

        if (m_engine && m_engine->getPlayer()) {
            currentDisplayValue = StatusManager::calculateDamage(
                m_logicEnemy,
                m_engine->getPlayer(),
                m_logicEnemy->getCurrentIntent().value
                );
        }
    }

    // ========================================================
    // 🟢 B. 复合意图系统：根据真实伤害动态发牌！
    // ========================================================
    QPixmap intentIcon;
    switch (m_intentType) {
    case IntentType::Attack:
        // ⚔️【压迫感引擎启动】：根据 currentDisplayValue 动态加载不同尺寸的刀！
        // （你可以根据你自己的游戏数值平衡来修改这些阈值）
        if (currentDisplayValue <= 8) {
            intentIcon = QPixmap(":/resources/images/intents/attack_1.png"); // 小匕首
        } else if (currentDisplayValue <= 16) {
            intentIcon = QPixmap(":/resources/images/intents/attack_2.png"); // 普通剑
        } else if (currentDisplayValue <= 24) {
            intentIcon = QPixmap(":/resources/images/intents/attack_3.png"); // 大剑
        } else if (currentDisplayValue <= 32) {
            intentIcon = QPixmap(":/resources/images/intents/attack_4.png"); // 发光大剑
        } else {
            intentIcon = QPixmap(":/resources/images/intents/attack_5.png"); // 致命巨刃！
        }
        break;

    // 💡 提示：如果你的“攻击+异常”复合图标也有不同大小的素材，也可以用同样的 if-else 逻辑！
    // 如果没有，就保持用单张复合图标。
    case IntentType::AttackAndDebuff: intentIcon = QPixmap(":/resources/images/intents/attack_debuff.png"); break;
    case IntentType::AttackAndDefend: intentIcon = QPixmap(":/resources/images/intents/attack_defend.png"); break;
    case IntentType::Defend:          intentIcon = QPixmap(":/resources/images/intents/defend.png"); break;
    case IntentType::Debuff:          intentIcon = QPixmap(":/resources/images/intents/debuff.png"); break;
    case IntentType::Buff:            intentIcon = QPixmap(":/resources/images/intents/buff.png"); break;
    case IntentType::InsertStatus:    intentIcon = QPixmap(":/resources/images/intents/curse.png"); break;
    case IntentType::Summon:          intentIcon = QPixmap(":/resources/images/intents/unknown.png"); break;
    case IntentType::DefendAndBuff:   intentIcon = QPixmap(":/resources/images/intents/defend_buff.png"); break;
    case IntentType::Curse:           intentIcon = QPixmap(":/resources/images/intents/curse.png"); break;
    case IntentType::GroupBuff:           intentIcon = QPixmap(":/resources/images/intents/buff.png"); break;
    case IntentType::GroupDefend:           intentIcon = QPixmap(":/resources/images/intents/defend.png"); break;
    default: break;
    }
    // ========================================================
    // 🔴 完美合成：微调高度，匹配缩小的图标
    // ========================================================
    // 因为图标变小了，我们把基础 Y 坐标往下沉一点点 (-280 改成 -260)，防止图标飘得太高
    int intentY = -enemyH - 60 + m_spriteYOffset + m_intentFloatOffset;

    // 🔴【核心修改 1】：图标尺寸从 96 缩小到 76！
    int iconSize = 76;

    if (!intentIcon.isNull()) {
        // 画出正在晃动的图标喵！
        painter->drawPixmap(-(iconSize / 2), intentY, iconSize, iconSize, intentIcon);

        // ========================================================
        // 🔴 智能排版：调整数字的大小和位置！
        // ========================================================
        if (m_intentType == IntentType::Attack || /*m_intentType == IntentType::Defend ||*/
            m_intentType == IntentType::AttackAndDebuff || /*m_intentType == IntentType::DefendAndBuff ||*/
            m_intentType == IntentType::AttackAndDefend/*||m_intentType == IntentType::GroupDefend || m_intentType == IntentType::GroupBuff*/ ) {

            painter->setPen(Qt::white);
            // 字体稍微改小一点（比如从 16 变成 14），配合变小的图标更协调
            QFont intentFont("Arial", 14, QFont::Bold);
            painter->setFont(intentFont);

            // 文本坐标会自动根据 iconSize 缩小而向中心靠拢！
            int textX = (iconSize / 2) + 5;
            int textY = intentY + (iconSize / 2) + 6;

            // 1. 直接画出我们刚刚在最前面算好的 currentDisplayValue！
            QString baseStr = QString::number(currentDisplayValue);
            painter->drawText(textX, textY, baseStr);

            // 2. 如果是多段攻击，进行动态间距计算
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

    // 背景（暗红）
    painter->setBrush(QColor(80, 20, 20));
    painter->setPen(Qt::NoPen);
    painter->drawRect(barX, barY, barW, barH);

    // 当前血条（亮红）
    if (maxHp > 0) {
        qreal hpRatio = static_cast<qreal>(hp) / maxHp;
        int currentHpW = static_cast<int>(barW * hpRatio);
        painter->setBrush(QColor(220, 20, 60));
        painter->drawRect(barX, barY, currentHpW, barH);
    }

    // 血量文字
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

    // ========================================================
    // 🟢 D. 怪物名字系统
    // ========================================================
    int nameY = barY + barH + 8;
    painter->setPen(Qt::white);
    painter->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
    painter->drawText(-150, nameY, 300, 25, Qt::AlignCenter, m_logicEnemy->getName());
}


// ========================================================
// 🖱️ 悬停进入事件：直接交给 Move 去做精准判定
// ========================================================
void EnemyItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    hoverMoveEvent(event);
}

// ========================================================
// 🖱️ 悬停移动事件：动态追踪那个上下浮动的图标！
// ========================================================
void EnemyItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    if (m_tooltipText.isEmpty() || m_intentType == IntentType::Unknown) {
        QToolTip::hideText();
        return;
    }

    // 1. 重新推演意图图标当前的精准位置！
    int iconSize = 76;

    // 🔴 保持和 paint 里一模一样的动态高度推演逻辑！
    int baseHeight = 200;
    int enemyH = static_cast<int>(baseHeight * m_logicEnemy->getScaleFactor());
    int intentY = -enemyH - 60 + m_spriteYOffset + m_intentFloatOffset;

    // 2. 构造图标的物理碰撞箱 (Hitbox)
    QRectF intentRect(-(iconSize / 2), intentY, iconSize, iconSize);
    // 💡 喵娘的“手感优化”魔法：把碰撞箱稍微向外扩张 10 像素！
    // 这样玩家的鼠标就算稍微偏了一点点，也能顺利触发，不容易因为图标浮动而疯狂闪烁
    intentRect.adjust(-10, -10, 10, 10);

    // 3. 精准制导判定：鼠标此刻到底在不在图标框内？
    if (intentRect.contains(event->pos())) {
        // 在图标范围内：显示提示框！
        QToolTip::showText(event->screenPos(), m_tooltipText);
    } else {
        // 移出了图标范围（虽然还在怪物身体上）：立刻隐藏提示框！
        QToolTip::hideText();
    }
}

// ========================================================
// 🖱️ 悬停离开事件：彻底离开怪物区域
// ========================================================
void EnemyItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    QToolTip::hideText();
}