#include "CardItem.h"
#include "BattleScene.h"
#include "EnemyItem.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup> // 【新增】动画组合拳
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "battleengine.h"
#include <QGraphicsOpacityEffect>
#include <random>    // 🔴 用于生成随机飞散轨迹喵！
#include <QVariantAnimation> // 🔴 用于驱动平民图元的新型动画库
#include <QTextDocument>

// 构造函数初始化
CardItem::CardItem(Card* logicCard, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_logicCard(logicCard),
    m_homeRotation(0.0), m_isHovered(false), m_isDragging(false),
    m_isPlayed(false), m_isPlayable(true), // 默认可打出
    m_currentTargetedEnemy(nullptr) {

    setAcceptHoverEvents(true);

    // 🔴 加载卡牌立绘！
    if (m_logicCard && !m_logicCard->getImagePath().isEmpty()) {
        m_cardPixmap.load(m_logicCard->getImagePath());
    }

}

void CardItem::checkPlayability(int currentEnergy) {
    bool bright = true;
    BattleEngine* engine = BattleEngine::getInstance();

    // ========================================================
    // 👑【至高无上优先级】：如果是时停选牌阶段，所有牌都是潜在祭品！
    // 必须全部无条件亮起，并且允许悬停和交互喵！
    // ========================================================
    if (engine && engine->isSelectingHandCard()) {
        bright = true;
    }
    // 常规战斗阶段
    else if (m_logicCard->isUnplayable()) {
        bright = true; // 绝对不可打出的状态牌（伤口、眩晕）保持常亮
    } else {
        // 普通牌和可打出状态牌（黏液）：严格检查费用是否足够
        bright = (currentEnergy >= m_logicCard->getCost());
    }

    // 刷新视觉与交互闸门
    if (m_isPlayable != bright) {
        m_isPlayable = bright;
        update(); // 动态脱掉或盖上黑布喵！
    }
}

void CardItem::setHomeState(const QPointF& pos, qreal rotation) {
    m_homePos = pos;
    m_homeRotation = rotation;
}

void CardItem::animateToHome() {
    setRotation(m_homeRotation);
    setScale(1.0);

    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(200);
    posAnim->setEndValue(m_homePos);
    posAnim->setEasingCurve(QEasingCurve::OutQuad);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF CardItem::boundingRect() const {
    return QRectF(-120, -150, 240, 300);
}

void CardItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_logicCard == nullptr) {
        qDebug() << "[CRASH-TEST] 🚨 致命错误：CardItem 在尝试 Paint 时，m_logicCard 丢失了！";
        return;
    }

    if (!m_logicCard) return;

    // 假设卡牌的中心点是 (0,0)，宽度 150，高度 210
    // 左上角坐标就是 (-75, -105)
    QRectF cardRect(-75, -105, 150, 210);

    // ========================================================
    // 🎨 1. 画卡牌底板与边框 (根据卡牌类型变色喵！)
    // ========================================================
    QColor bgColor(40, 40, 45); // 默认暗灰底色
    QColor borderColor;
    switch (m_logicCard->getType()) {
    case CardType::Attack: borderColor = QColor(220, 50, 50); break; // 红色边框
    case CardType::Skill:  borderColor = QColor(50, 200, 100); break;// 绿色边框
    case CardType::Power:  borderColor = QColor(50, 150, 250); break;// 蓝色边框
    default: borderColor = Qt::gray;
    }

    // 启用抗锯齿，让圆角更平滑
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setBrush(bgColor);
    painter->setPen(QPen(borderColor, 3)); // 3 像素粗的类型边框
    painter->drawRoundedRect(cardRect, 8, 8); // 8 像素圆角

    // ========================================================
    // 👑 极品视觉：展示模式下的“黄金选中框”！
    // ========================================================
    if (property("ui_selected").toBool()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255, 215, 0), 6, Qt::SolidLine)); // 6像素粗的纯正金边
        painter->drawRoundedRect(cardRect, 8, 8);
    }

    // ========================================================
    // 🎨 2. 画卡牌上半部分的立绘！
    // ========================================================
    // 图片框占据上半部分：宽 130，高 90，稍微往里缩一点留出边距
    QRectF imageRect(-65, -95, 130, 90);
    if (!m_cardPixmap.isNull()) {
        painter->drawPixmap(imageRect.toRect(), m_cardPixmap);
    } else {
        // 没有图片时画个占位符
        painter->setBrush(Qt::darkGray);
        painter->drawRect(imageRect);
    }

    // ========================================================
    // 🟢 绘制卡牌名字（加入升级翠绿光效！）
    // ========================================================

    // 1. 恢复你原本完美的中心横幅区域（宽 150，高 25）
    // 这样就彻底绕开了未定义的 cardW 和 cardH，同时也避免了和立绘重叠喵！
    QRectF nameBannerRect(-75, 0, 150, 25);

    // （可选）如果你想保留名字背后的半透明黑底，可以让字看得更清楚，取消下面两行的注释：
    // painter->setBrush(QColor(20, 20, 20, 200));
    // painter->setPen(Qt::NoPen);
    // painter->drawRect(nameBannerRect);

    // 2. 设置字体
    QFont nameFont("Microsoft YaHei", 12, QFont::Bold);
    painter->setFont(nameFont);

    // 3. 🔴 视觉分流：如果是升级过的牌，给它染上代表强化的绿色！
    if (m_logicCard->isUpgraded()) {
        painter->setPen(QColor(100, 255, 100)); // 亮翠绿色
    } else {
        painter->setPen(Qt::white); // 原本的普通白色
    }

    // 4. 完美居中画出名字（此时如果升级了，名字已经是 "打击+" 了）
    painter->drawText(nameBannerRect, Qt::AlignCenter, m_logicCard->getName());

    // ========================================================
    // 🎨 4. 画下半部分的描述文字！(自带动态数值与自动换行功能喵！)
    // ========================================================
    QRectF descRect(-65, 35, 130, 60);
    painter->setPen(Qt::lightGray);
    painter->setFont(QFont("Microsoft YaHei", 9));

    // 🔮 1. 从大脑单例中拿到主角指针（因为计算伤害需要主角的力量）
    Player* player = nullptr;
    // ========================================================
    // 🔴 绝对物理隔离：如果是纯展示模式，禁止访问任何战斗单例！
    // ========================================================
    if (!m_isDisplayOnly) {
        if (BattleEngine::getInstance()) {
            player = BattleEngine::getInstance()->getPlayer();
        }
    }

    // 🔮 2. 呼叫动态文案生成器！
    // ========================================================
    // 🔴【核心修复】：如果锁定了 UI 怪物，必须取出它的逻辑实体 (getLogicEnemy)！
    // ========================================================
    Fighter* logicTarget = nullptr;
    if (m_currentTargetedEnemy) {
        logicTarget = m_currentTargetedEnemy->getLogicEnemy(); // 剥离出逻辑指针！
    }

    // 🔮 获取动态文案（你已经修好了 logicTarget，非常棒喵！）
    QString dynamicDesc = m_logicCard->getDynamicDescription(player, logicTarget);

    // ========================================================
    // 🔴【核心修复 1】：使用 QTextDocument 渲染 HTML 富文本！
    // ========================================================
    QTextDocument doc;

    // 🔴【新增视效优化】：把默认的页面边距砍到 0！文字瞬间完美填满你的 descRect！
    doc.setDocumentMargin(0);

    doc.setDefaultFont(QFont("Microsoft YaHei", 9));

    QString htmlStr = QString("<div align='center' style='color: white;'>%1</div>").arg(dynamicDesc);
    doc.setHtml(htmlStr);
    doc.setTextWidth(descRect.width()); // 限制宽度，自动换行

    // 开始画富文本！
    painter->save();
    painter->translate(descRect.topLeft()); // 把画笔原点挪到框的左上角
    doc.drawContents(painter);
    painter->restore();
    // 【新增】：如果费用不够，盖上一层半透明黑布，直接变灰！
    // 🔴【终极完美特判】：如果它是天生不可打出的牌（如伤口），给它视觉特权常亮！
    // 但如果它是黏液（isUnplayable为false），且没费用（!m_isPlayable），它就会乖乖变灰！
    if (!m_isPlayable && !m_logicCard->isUnplayable()) {
        painter->setBrush(QColor(0, 0, 0, 150));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(cardRect, 12, 12);
    }

    // ========================================================
    // ✅ 修复：给时停金边也加上免死金牌，并且加上断网隔离！
    // ========================================================
    if (!m_isDisplayOnly) { // 🔴 必须加上这个围墙！
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->isSelectingHandCard()) {
            if (engine->getSelectedCards().contains(m_logicCard)) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(241, 196, 15), 4, Qt::SolidLine));
                painter->drawRoundedRect(cardRect, 8, 8);
            }
        }
    }
    // ========================================================
    // 🎨 5. 画左上角的能量(Cost)球！(3A 级重构版)
    // ========================================================
    // 圆心在卡牌左上角稍微偏外一点点，更有立体感
    if (m_logicCard->getCost() >= 0)
    {
        // 1. 核心视觉欺骗：如果是 X 费牌，强行把文本变成 "X"
        QString costStr;
        if (m_logicCard->isXCost()) {
            costStr = "X";
        } else {
            costStr = QString::number(m_logicCard->getCost());
        }

        QRectF costRect(-90, -120, 32, 32);

        // ========================================================
        // 💎 视觉升级 1：用径向渐变画出“3D 立体玻璃能量球”！
        // ========================================================
        // 让光源从左上角打过来 (costRect.topLeft() + 偏移量)
        QRadialGradient orbGradient(costRect.center(), 16, costRect.topLeft() + QPointF(8, 8));
        orbGradient.setColorAt(0.0, QColor(100, 200, 255)); // 球体高光（浅蓝，受光面）
        orbGradient.setColorAt(0.8, QColor(30, 100, 200));  // 球体本体（深蓝）
        orbGradient.setColorAt(1.0, QColor(10, 50, 100));   // 球体边缘暗部（极其立体的暗角）

        painter->setBrush(orbGradient);
        painter->setPen(QPen(QColor(20, 25, 30), 2)); // 换用深色极细描边代替纯白，打破塑料感
        painter->drawEllipse(costRect);

        // ========================================================
        // 👻 视觉升级 2：极品文本阴影（让数字悬浮在球体之上）
        // ========================================================
        painter->setFont(QFont("Arial", 14, QFont::Bold));
        painter->setPen(QColor(0, 0, 0, 180)); // 半透明纯黑阴影
        // 将绘制位置向右下方偏移 1.5 个像素，画出底层投影
        painter->drawText(costRect.translated(1.5, 1.5), Qt::AlignCenter, costStr);

        // ========================================================
        // 🚦 视觉升级 3：状态变色引擎（异蛇之眼 / 费用不足）
        // ========================================================
        // (注：m_isDisplayOnly 表示这张牌是不是在战利品/奖励界面，如果是展示模式，就不报红)
        if (!m_isPlayable && !m_isDisplayOnly) {
            // 🚨 费用不足：打不出的牌，字体变成极其刺眼的警告红！
            painter->setPen(QColor(255, 80, 80));
        }
        else if (m_logicCard->isCostModified()) {
            // 👁️ 异蛇之眼 / 疯狂 / 被减费：字体变成科技感的荧光绿！
            painter->setPen(QColor(138, 226, 52));
        }
        else {
            // ⚪ 正常情况：纯净的白色
            painter->setPen(Qt::white);
        }

        // 最后画上带有颜色、并且有刚才底层黑影托底的超清晰数字！
        painter->drawText(costRect, Qt::AlignCenter, costStr);
    }

}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);

    if (m_isDisplayOnly) {

        setZValue(200); // 浮到最上层
        // 只需要极其简单粗暴的原地放大！
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
        scaleAnim->setEndValue(1.5); // 原地放大 1.5 倍
        scaleAnim->setDuration(150);
        scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
        return; // 🟢 极其关键：直接 return！阻止后续的手牌位移代码执行！
    }

    if (m_isGhost) {
        event->ignore(); // 把事件抛出去，不处理！
        return;
    }

    if (m_isPlayed || m_isDragging) return; // 【新增】如果打出了，就当死人，不响应悬停！
    // 🔴【拦截】：费用不够的牌不给弹起（包括没费的黏液）。
    // 但天生不可打出的牌（伤口）拥有悬停特权，允许弹起来给玩家阅读说明！
    if (!m_isPlayable && !m_logicCard->isUnplayable()) return;
    // 🔴【无敌金身】：悬浮状态绝对不允许触发缩回原位的代码！
    if (m_isSuspended) return;

    m_isHovered = true;
    setZValue(100);
    setRotation(0.0);

    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(150);
    posAnim->setEndValue(m_homePos + QPointF(0, -40));
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(150);
    scaleAnim->setEndValue(1.25);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);

    if (m_isDisplayOnly) {
        // ========================================================
        // 🌟 核心魔法：如果我被选中了，我就拒绝缩小！保持高傲的放大姿态！
        // ========================================================
        if (property("ui_selected").toBool()) return;

        setZValue(160);
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
        scaleAnim->setEndValue(1.0);
        scaleAnim->setDuration(150);
        scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    if (m_isGhost) {
        event->ignore(); // 把事件抛出去，不处理！
        return;
    }

    BattleEngine* engine = BattleEngine::getInstance();

    // 🔴【无敌金身】：悬浮状态绝对不允许触发缩回原位的代码！
    if (m_isSuspended) return;

    // ========================================================
    // 🔴【铁闸门】：如果在时停结界里，并且本卡牌已经被勾选为祭品了！
    // 绝对不允许任何重置！让它保持悬浮和金边！
    // ========================================================
    if (engine && engine->isSelectingHandCard()) {
        if (engine->getSelectedCards().contains(m_logicCard)) {
            // 直接 return，无视鼠标离开！保持高傲的姿态喵！
            return;
        }
    }

    if (m_isPlayed || m_isDragging) return; // 【新增】如果打出了，就当死人，不响应悬停！
    // 🔴【拦截】：费用不够的牌不给弹起（包括没费的黏液）。
    // 但天生不可打出的牌（伤口）拥有悬停特权，允许弹起来给玩家阅读说明！
    if (!m_isPlayable && !m_logicCard->isUnplayable()) return;

    m_isHovered = false;
    setZValue(10);

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(150);
    scaleAnim->setEndValue(1.0);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

    animateToHome();
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    BattleEngine* engine = BattleEngine::getInstance();

    if (m_isGhost) {
        event->ignore(); // 把事件抛出去，不处理！
        return;
    }

    if (m_isDisplayOnly) {
        event->accept(); // 吞掉点击事件，当无事发生！
        return;
    }

    // ========================================================
    // 👑【绝对优先级 1】：如果大脑处于“时停选牌”模式，接管一切卡牌的点击！
    // 无论是普通牌还是状态牌，现在统统是待宰的祭品，必须一视同仁！
    // ========================================================
    if (engine && engine->isSelectingHandCard()) {
        if (event->button() == Qt::LeftButton) {

            // 1. 问问大脑，我自己是不是已经被勾选了？
            bool alreadySelected = engine->getSelectedCards().contains(m_logicCard);

            // 2. 如果没被勾选，且大脑已经选满了，则点其他牌毫无反应
            if (!alreadySelected && engine->getSelectedCards().size() >= engine->getSelectionLimit()) {
                event->accept();
                return;
            }

            // 3. 通知大脑切换勾选状态（底层列表的增删）
            engine->toggleCardSelection(m_logicCard);

            // 4. 【视觉弹起与缩回】：动态改变自己在舞台上的 Y 坐标！
            if (!alreadySelected) {
                qDebug() << "[UI] 选中了祭品，弹起：" << m_logicCard->getName();
                setPos(pos().x(), pos().y() - 40);
            } else {
                qDebug() << "[UI] 取消了祭品，缩回：" << m_logicCard->getName();
                setPos(pos().x(), pos().y() + 40);
            }

            update(); // 强制重绘，触发金边高亮
        }
        event->accept();
        return; // 🔴 极其重要：直接 return！绝对不走下面的拖拽和普通拦截逻辑！
    }

    // ========================================================
    // 🛡️【优先级 2】：常规实战模式下的拦截门（只有正常打牌时才会走到这里喵！）
    // ========================================================
    if (event->button() == Qt::LeftButton) {

        // 🔴【状态牌防拖拽铁闸门】：放在选牌逻辑后面！
        // 平时正常打牌时，状态牌不许拖拽；但在选祭品时，它已经在上面被拦截并支持点击了！
        if (m_logicCard->isUnplayable()) {
            event->accept();
            return;
        }

        if (!m_isPlayable) return; // 普通牌费用不够的拦截

        m_isDragging = true;
        setZValue(200);
        m_currentTargetedEnemy = nullptr;

        if (m_logicCard->requiresTarget()) {
            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) bScene->startTargeting(this->scenePos());
        } else {
            setRotation(0.0);
        }
        event->accept();
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}

// =========================================================================
// UI Polish 核心实现喵：拖拽过程锁定提示 + 打出流光动画
// =========================================================================

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    // 🔴【无敌金身】：悬浮期间禁止拖拽！
    if (m_isSuspended) {
        event->accept(); // 吞掉点击
        return;
    }

    if (m_isDisplayOnly) {
        event->accept(); // 吞掉点击事件，当无事发生！
        return;
    }

    if (m_isGhost) {
        event->ignore(); // 把事件抛出去，不处理！
        return;
    }

    if (m_isDragging) {
        if (m_logicCard->requiresTarget()) {
            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) bScene->updateTargeting(event->scenePos());

            // 🔴 【原版核心】：拖拽过程中实时穿透检测锁定提示喵！
            QList<QGraphicsItem*> hitItems = scene()->items(event->scenePos());
            EnemyItem* foundEnemy = nullptr;

            for (QGraphicsItem* item : hitItems) {
                EnemyItem* e = dynamic_cast<EnemyItem*>(item);
                if (e && !e->getLogicEnemy()->isDead()) {
                    foundEnemy = e;
                    break;
                }
            }

            // 锁定状态机流转
            if (foundEnemy != m_currentTargetedEnemy) {
                // 1. 如果以前指着某个怪，现在指没了，取消以前那个怪的目标高亮喵
                if (m_currentTargetedEnemy) {
                    m_currentTargetedEnemy->m_isTargeted = false;
                    m_currentTargetedEnemy->update(); // 刷新旧怪物，熄灭高亮
                }

                // 2. 将目标锁定到新找到的怪身上，并让它当场发光喵！
                m_currentTargetedEnemy = foundEnemy;
                if (m_currentTargetedEnemy) {
                    m_currentTargetedEnemy->m_isTargeted = true;
                    m_currentTargetedEnemy->update(); // 刷新新怪物，点亮高亮

                    qDebug() << "[UI Vibe] >> TARGET LOCKED ON:" << m_currentTargetedEnemy->getLogicEnemy()->getName();
                }

                // ========================================================
                // 🔴【灵魂缝合】：目标变了（可能从无到有，或换了只怪），强制卡牌自刷新！
                // 这会立刻触发本卡牌的 paint()，调用 getDynamicDescription()。
                // 伤害数字瞬间暴涨或回落，果汁感和策略反馈拉满喵！
                // ========================================================
                update();
            }

        } else {
            setPos(event->scenePos());

            // 🔴 群体卡牌虽然不拉箭头，但鼠标摸到怪物时，也要让卡牌文字动态算入该怪物的易伤！
            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) {
                EnemyItem* currentHitItem = nullptr;
                QList<QGraphicsItem*> colliders = bScene->items(event->scenePos());

                for (QGraphicsItem* item : colliders) {
                    EnemyItem* eItem = dynamic_cast<EnemyItem*>(item);
                    if (eItem && !eItem->getLogicEnemy()->isDead()) {
                        currentHitItem = eItem;
                        break;
                    }
                }

                // 如果鼠标指针滑入/滑出了某只怪，立刻刷新卡牌描述！
                if (m_currentTargetedEnemy != currentHitItem) {
                    m_currentTargetedEnemy = currentHitItem;
                    update(); // 🔴 核心：触发群体牌伤害数字动态暴涨变绿！
                }
            }
        }
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {

    if (m_isDisplayOnly) {
        event->accept(); // 吞掉点击事件，当无事发生！
        return;
    }

    if (m_isGhost) {
        event->ignore(); // 把事件抛出去，不处理！
        return;
    }

    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;

        BattleScene* bScene = dynamic_cast<BattleScene*>(scene());

        // ========================================================
        // 🔴【多怪无缝雷达 + 动态数值同步】：
        // 进行最后一次绝对精准的碰撞检测，并同步校准目标
        // ========================================================
        if (m_logicCard->requiresTarget() && bScene) {
            // 顺手呼叫场景雷达更新最后一帧的终点，让大箭头完美收网喵！
            bScene->updateTargeting(event->scenePos());

            QList<QGraphicsItem*> colliders = bScene->items(event->scenePos());
            for (QGraphicsItem* item : colliders) {
                EnemyItem* eItem = dynamic_cast<EnemyItem*>(item);
                // 只要压住了一只活着的怪物，立刻强行校准当前目标！
                if (eItem && !eItem->getLogicEnemy()->isDead()) {
                    m_currentTargetedEnemy = eItem;
                    break;
                }
            }
        }

        // 彻底取消怪物的锁定显示高亮（防止怪物带着高亮死亡） —— 🟢 完美继承你的原生代码！
        if (m_currentTargetedEnemy) {
            m_currentTargetedEnemy->m_isTargeted = false;
            m_currentTargetedEnemy->update();
        }

        // ========================================================
        // ⚔️ 分流判定区
        // ========================================================
        if (m_logicCard->requiresTarget()) {
            if (bScene) bScene->stopTargeting();

            // 如果鼠标抬起时，鼠标底下正指着那个我们标记的目标，则视为打出成功
            if (m_currentTargetedEnemy && !m_currentTargetedEnemy->getLogicEnemy()->isDead()) {
                // 🔴 动态打印具体怪物的真实姓名
                qDebug() << "[UI] Polish Play Success! Targeted card executed on:" << m_currentTargetedEnemy->getLogicEnemy()->getName();

                // 【核心】：向上级管家和引擎汇报！
                emit cardPlayedRequest(m_logicCard, m_currentTargetedEnemy->getLogicEnemy());

                // 预留你的出牌与消耗过渡动画
                // animatePlayAndExhaust();
                // emit cardVisualDestroyed(this);

            } else {
                qDebug() << "[UI] Missed target during polish test. Snapping back.";
                animateToHome(); // 弹回手牌
            }

            // 🔴【点睛之笔】：指针清空后，立刻强制卡牌自重绘！
            // 这样卡牌在缩回手牌或者消失时，动态描述文本会瞬间弹回初始的基础数值，消灭一切视觉Bug！
            m_currentTargetedEnemy = nullptr;
            update();

        } else {
            // 🟢 区域打出逻辑（比如防御牌、能力牌）完全保持原样，纯正的血统！
            if (event->scenePos().y() < 750.0) {
                qDebug() << "[UI] Polish Area Play Success!";

                emit cardPlayedRequest(m_logicCard, nullptr);
                // animatePlayAndExhaust();
                // emit cardVisualDestroyed(this);
            } else {
                qDebug() << "[UI] Drag too short. Polish abort.";
                animateToHome();
            }

            // 🔴【点睛之笔】：区域卡牌结算完毕后，也顺手清空并强制重绘刷新
            m_currentTargetedEnemy = nullptr;
            update();
        }

        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}

void CardItem::animatePlayAndDiscard() {
    m_isPlayed = true; // 🔴【立刻封印】：开启打出状态！这之后它不会理会任何鼠标事件喵！
    // 工业级 QParallelAnimationGroup：旋转、缩放、飞往弃牌堆的三位一体组合拳！喵喵！
    QParallelAnimationGroup* playGroup = new QParallelAnimationGroup(this);

    // 1. 飞向右下角逻辑上的弃牌堆区域 (假设坐标为 1800, 1000)喵
    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(300);
    posAnim->setEndValue(QPointF(1800, 1000));
    posAnim->setEasingCurve(QEasingCurve::InBack); // 带一点先往后拉，再猛冲的蓄力感喵
    playGroup->addAnimation(posAnim);

    // 2. 缩小动画喵
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setEndValue(0.1); // 缩成一个点喵
    playGroup->addAnimation(scaleAnim);

    // 3. 旋转狂飙动画 (产生流光效果)喵
    QPropertyAnimation* rotateAnim = new QPropertyAnimation(this, "rotation");
    rotateAnim->setDuration(300);
    // 从当前角度狂甩 720 度，看起来就是光速旋转喵！
    rotateAnim->setEndValue(this->rotation() + 720.0);
    playGroup->addAnimation(rotateAnim);

    // 动画放完后，优雅地执行 deleteLater 销毁图元喵
    connect(playGroup, &QParallelAnimationGroup::finished, this, &CardItem::deleteLater);

    // 开启图层混合模式，让旋转产生流光残影（根据 Qt 版本支持，这里仅示意）
    // painter->setCompositionMode(QPainter::CompositionMode_Plus);

    qDebug() << "[UI Animation] Polish play transition started for:" << m_logicCard->getName();
    playGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardItem::animateSuspendInCenter() {
    m_isSuspended = true;

    // 🔴 赋予至高无上的层级！103 压过黑幕(100)、提示字(101)和确认按钮(102)！
    setZValue(103);

    // 使用 Qt 动画引擎让它飞到屏幕中央靠上的位置
    QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(300); // 0.3秒飞过去
    // 假设你的屏幕宽 1920，正中间偏上大概是 x=960-75(卡牌一半宽), y=400
    anim->setEndValue(QPointF(960, 500));
    anim->setEasingCurve(QEasingCurve::OutBack); // 带一点点果汁感的 Q 弹回弹！
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 顺便放大一点点，彰显施法者的尊贵！
    setScale(1.2);
}

// 🔴 真正的燃烧献祭动画！
void CardItem::animateTrueExhaust() {
    m_isPlayed = true; // 封印鼠标

    QGraphicsScene* s = scene();
    if (!s) return;

    qreal baseZ = zValue();
    int particleCount = 40;
    QRectF cardRect = boundingRect();

    // ========================================================
    // 🔥 第一部分：大号余烬球球生成系统！(终极视网膜级全透明版)
    // ========================================================
    for (int i = 0; i < particleCount; ++i) {

        qreal size = randomBetween(10, 18);
        QGraphicsEllipseItem* ash = new QGraphicsEllipseItem(0, 0, size, size);

        QColor emberColor(randomBetween(40, 65), randomBetween(30, 40), randomBetween(30, 40));
        ash->setBrush(QBrush(emberColor));
        ash->setPen(Qt::NoPen);
        ash->setZValue(baseZ + 1);
        s->addItem(ash);

        // 初始位置：卡牌内部随机
        qreal startX = scenePos().x() + randomBetween(cardRect.left(), cardRect.right());
        qreal startY = scenePos().y() + randomBetween(cardRect.top(), cardRect.bottom());
        ash->setPos(startX, startY);

        // 飞散目标
        qreal targetX = startX + randomBetween(-350, 350);
        qreal targetY = startY - randomBetween(400, 700);
        qreal startRot = this->rotation();
        qreal targetRot = startRot + randomBetween(-720, 720);

        QVariantAnimation* anim = new QVariantAnimation(this);

        // 慢速消散
        anim->setDuration(randomBetween(1500, 2500));
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutQuad);

        connect(anim, &QVariantAnimation::valueChanged, [ash, startX, startY, targetX, targetY, startRot, targetRot](const QVariant& value) {
            qreal progress = value.toReal();

            ash->setPos(startX + (targetX - startX) * progress, startY + (targetY - startY) * progress);
            ash->setRotation(startRot + (targetRot - startRot) * progress);

            // 延迟隐去算法：进度前 30% 不透明，后 70% 慢慢淡出
            qreal opacity = 1.0;
            if (progress > 0.3) {
                // 🔴 终极修正：由于 OutQuad 的原因，最后几帧可能因为浮点数精度 hovering。
                // 我们在数学上强制拉一个底喵！
                opacity = qMax(0.0, 1.0 - ((progress - 0.3) / 0.7));
            }

            // ========================================================
            // 🔴【终极铁闸门】：如果动画进度达到 1.0（最后一帧），强制视觉透明度为 0.0！
            // 彻底解决最后 0.01 的淡出不彻底导致 abrupt disappearance 的问题！
            // ========================================================
            if (progress >= 1.0) {
                opacity = 0.0;
            }

            ash->setOpacity(opacity);
        });

        connect(anim, &QVariantAnimation::finished, [ash]() {
            delete ash; // 这里直接 delete，因为已经是视觉上的死人啦喵喵喵！
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // ========================================================
    // 🔥 第二部分：卡牌本体崩解！(全自动无 Bug 视网膜版)
    // ========================================================
    QParallelAnimationGroup* masterGroup = new QParallelAnimationGroup(this);

    // 1. 卡牌本体：稍微向上升腾
    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(1500);
    posAnim->setEndValue(this->pos() + QPointF(0, -120));
    posAnim->setEasingCurve(QEasingCurve::InQuad);
    masterGroup->addAnimation(posAnim);

    // 2. 卡牌本体：透明淡出
    if (this->graphicsEffect()) {
        QGraphicsOpacityEffect* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect());
        if (opacityEffect) {
            QPropertyAnimation* fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
            fadeAnim->setDuration(1500);
            fadeAnim->setStartValue(1.0);
            fadeAnim->setEndValue(0.0);

            // ========================================================
            // 🔴【终极修正】：更改曲线为 Linear。
            // 确保淡出过程线性平滑地直达 0.0，彻底消除 InQuad 曲线末端加速导致视觉突兀喵！
            // ========================================================
            fadeAnim->setEasingCurve(QEasingCurve::Linear);

            masterGroup->addAnimation(fadeAnim);
        }
    }

    // 3. 卡牌本体：稍微缩小
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(1500);
    scaleAnim->setEndValue(0.01);
    masterGroup->addAnimation(scaleAnim);

    // 烧完后骨灰随风飘散（卡牌图元本体销毁）
    connect(masterGroup, &QParallelAnimationGroup::finished, this, &CardItem::deleteLater);

    qDebug() << "[UI Animation] 视网膜级冥灰崩解特效启动喵！！！ ->" << m_logicCard->getName();
    masterGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

// 确保你把下面这个生成器放在了 cpp 文件的最下面喵！
qreal CardItem::randomBetween(qreal low, qreal high) {
    return low + (static_cast<qreal>(std::rand()) / static_cast<qreal>(RAND_MAX)) * (high - low);
}