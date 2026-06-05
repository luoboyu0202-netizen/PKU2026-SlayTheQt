#include <QGraphicsObject>
#include <QPainter>
#include <QRadialGradient>
#include <QTimer>
#include <QRandomGenerator>
#include <QList>

// ========================================================
// ⚛️ 物理粒子结构体：定义每一个火星的灵魂
// ========================================================
struct FlameParticle {
    qreal x, y;          // 当前坐标
    qreal vx, vy;        // X和Y轴的移动速度
    int life;            // 剩余寿命
    int maxLife;         // 初始最大寿命
    qreal startRadius;   // 初始大小
};

class HexaFlameItem : public QGraphicsObject {
    Q_OBJECT

private:
    bool m_isIgnited = false;
    QList<FlameParticle> m_particles; // 粒子池
    QTimer* m_emitterTimer;           // 引擎点火器

public:
    explicit HexaFlameItem(QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent) {

        // 🔴 粒子物理引擎：每 30 毫秒刷新一次物理运算 (约 33 FPS，保证流畅且不吃满 CPU)
        m_emitterTimer = new QTimer(this);
        connect(m_emitterTimer, &QTimer::timeout, this, &HexaFlameItem::updateParticles);
    }

    void setIgnited(bool ignited) {
        if (m_isIgnited == ignited) return;
        m_isIgnited = ignited;

        if (m_isIgnited) {
            this->show();
            m_emitterTimer->start(30); // 点火！引擎开始喷射粒子！

            // 爆燃瞬间：一次性喷射 20 个爆炸粒子！
            for(int i=0; i<20; ++i) spawnParticle(true);
        } else {
            m_emitterTimer->stop(); // 熄火！
            m_particles.clear();
            this->hide();
        }
    }

    QRectF boundingRect() const override {
        // 留出足够大的空间供粒子向上飘散
        return QRectF(-40, -80, 80, 100);
    }

    // ========================================================
    // ⚙️ 核心算法：物理规则更新与发射器
    // ========================================================
    void updateParticles() {
        if (!m_isIgnited) return;

        // 1. 喷射新粒子 (每帧稳定产生 2~3 个新火星)
        int spawnCount = QRandomGenerator::global()->bounded(2, 4);
        for (int i = 0; i < spawnCount; ++i) {
            spawnParticle(false);
        }

        // 2. 物理演算：更新所有粒子的位置与寿命
        for (int i = m_particles.size() - 1; i >= 0; --i) {
            FlameParticle& p = m_particles[i];

            p.x += p.vx;
            p.y += p.vy;

            // 热空气上升加速，火焰越往上飘移越快，并且会有微小的左右扰动(模仿风)
            p.vy -= 0.1;
            p.x += (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;

            p.life--;

            // 粒子死亡，移出内存池
            if (p.life <= 0) {
                m_particles.removeAt(i);
            }
        }

        // 3. 呼叫显卡重新渲染这一帧！
        update();
    }

    void spawnParticle(bool isBurst) {
        FlameParticle p;
        // 初始位置在底部核心处，稍微有一点随机偏移
        p.x = (QRandomGenerator::global()->generateDouble() - 0.5) * 15;
        p.y = 10 + (QRandomGenerator::global()->generateDouble() * 5);

        if (isBurst) {
            // 爆燃时的粒子：速度极快，向四周炸开
            p.vx = (QRandomGenerator::global()->generateDouble() - 0.5) * 6;
            p.vy = (QRandomGenerator::global()->generateDouble() - 0.5) * 6;
            p.maxLife = QRandomGenerator::global()->bounded(10, 20);
            p.startRadius = QRandomGenerator::global()->bounded(15, 25);
        } else {
            // 正常燃烧时的粒子：主要向上飘 (Y速度为负数)
            p.vx = (QRandomGenerator::global()->generateDouble() - 0.5) * 2;
            p.vy = -QRandomGenerator::global()->generateDouble() * 3 - 1;
            p.maxLife = QRandomGenerator::global()->bounded(20, 35);
            p.startRadius = QRandomGenerator::global()->bounded(12, 18);
        }
        p.life = p.maxLife;
        m_particles.append(p);
    }

    // ========================================================
    // 🎨 渲染管线：滤色混合与颜色插值
    // ========================================================
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        if (!m_isIgnited || m_particles.isEmpty()) return;

        painter->setRenderHint(QPainter::Antialiasing);

        // 🟢【灵魂滤镜】：Screen（滤色）模式！
        // 这一步是化腐朽为神奇的关键！当多个半透明的粒子重叠时，
        // 它们的颜色会像光一样叠加，中心最密集的地方会自然变成刺眼的白光！
        painter->setCompositionMode(QPainter::CompositionMode_Screen);
        painter->setPen(Qt::NoPen);

        for (const FlameParticle& p : m_particles) {
            // 生命占比 (1.0 = 刚出生, 0.0 = 即将死亡)
            qreal lifeRatio = static_cast<qreal>(p.life) / p.maxLife;

            // 粒子越老，尺寸越小，产生火焰向上收缩的视觉错觉
            qreal currentRadius = p.startRadius * lifeRatio;
            if (currentRadius <= 0) continue;

            // 动态颜色插值演算：
            // 核心（刚出生）：刺眼的高温白绿色
            // 中段：极度纯正的幽绿
            // 末端：暗淡的黑绿色并逐渐透明
            int r, g, b, a;
            if (lifeRatio > 0.6) {
                // 白绿 -> 纯绿
                qreal t = (lifeRatio - 0.6) / 0.4; // 0~1
                r = static_cast<int>(50 + 205 * t);  // 50 -> 255
                g = static_cast<int>(200 + 55 * t);  // 200 -> 255
                b = static_cast<int>(50 + 150 * t);  // 50 -> 200
                a = 255;
            } else {
                // 纯绿 -> 暗透明
                qreal t = lifeRatio / 0.6; // 0~1
                r = static_cast<int>(50 * t);
                g = static_cast<int>(200 * t);
                b = static_cast<int>(50 * t);
                a = static_cast<int>(255 * t);
            }

            // 画出具有柔和边缘的单颗粒子
            QRadialGradient grad(p.x, p.y, currentRadius);
            grad.setColorAt(0.0, QColor(r, g, b, a));
            grad.setColorAt(1.0, Qt::transparent);

            painter->setBrush(grad);
            painter->drawEllipse(QPointF(p.x, p.y), currentRadius, currentRadius);
        }
    }
};
