#include "TitleMenuView.h"
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QDebug>
#include <QEasingCurve>

TitleMenuView::TitleMenuView(QWidget *parent)
    : QWidget(parent), m_hasSavedGame(false) // 备注：目前无存档功能，默认为 false
{
    // 1. 加载背景图片
    // 🔴 已经为你更新为新的纯英文图片名称！
    // 请确保这张图片已经添加到了你的 Qt 资源文件 (.qrc) 的对应目录中
    m_bgPixmap.load(":/resources/images/map_images/TitleMenuView.png");
    if(m_bgPixmap.isNull()){
        qWarning() << "警告: 找不到背景图片，请检查资源路径！";
    }

    // 2. 初始化 UI
    initUI();

    // 3. 初始化光效层
    initLightEffects();
}

TitleMenuView::~TitleMenuView() {
}

void TitleMenuView::initUI() {
    // 布局设置 (左下角)
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_mainLayout->setContentsMargins(100, 0, 0, 100); // 距离左边100，距离底部100
    m_mainLayout->setSpacing(20);

    // 创建按钮
    m_btnContinue = new QPushButton("继续游戏", this);
    m_btnStart = new QPushButton("开始游戏", this);
    m_btnExit = new QPushButton("退出游戏", this);

    m_mainLayout->addWidget(m_btnContinue);
    m_mainLayout->addWidget(m_btnStart);
    m_mainLayout->addWidget(m_btnExit);

    // 设置基础 QSS 样式 (白色，仿宋加粗，透明底)
    QString baseQss = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            font-family: "FangSong";
            font-weight: bold;
            color: white;
            font-size: 32px;
            padding: 10px 15px;
            text-align: left;
        }
    )";
    this->setStyleSheet(baseQss);

    // 存档状态判断逻辑
    if (!m_hasSavedGame) {
        // 无存档：禁用按钮，文字变灰
        m_btnContinue->setEnabled(false);
        m_btnContinue->setStyleSheet("QPushButton { color: gray; font-family: 'FangSong'; font-weight: bold; }");
    } else {
        // 有存档才安装事件监听器（才会有光效）
        m_btnContinue->installEventFilter(this);
    }

    // 给开始和退出按钮安装事件监听器
    m_btnStart->installEventFilter(this);
    m_btnExit->installEventFilter(this);

    // 绑定信号槽
    connect(m_btnStart, &QPushButton::clicked, this, &TitleMenuView::onStartGameClicked);
    connect(m_btnExit, &QPushButton::clicked, this, &TitleMenuView::onExitGameClicked);
}

void TitleMenuView::initLightEffects() {
    // 光斑样式：中心淡黄，边缘全透明
    QString effectQss = R"(
        QLabel {
            background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(255, 255, 0, 150), stop:1 rgba(255, 255, 0, 0));
            border-radius: 20px;
        }
    )";

    // 找出需要光效的按钮
    QList<QPushButton*> buttons = {m_btnStart, m_btnExit};
    if (m_hasSavedGame) {
        buttons.append(m_btnContinue);
    }

    // 为每个按钮生成一个隐藏的“光斑” Label
    for (QPushButton* btn : buttons) {
        QLabel* lightLabel = new QLabel(this);
        lightLabel->setFixedSize(120, btn->sizeHint().height() - 10); // 光斑宽度120
        lightLabel->setStyleSheet(effectQss);
        lightLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 让鼠标穿透光斑
        lightLabel->hide();
        m_lightEffects.insert(btn, lightLabel);
    }
}

// 绘制背景图（自适应窗口拉伸）
void TitleMenuView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(this->rect(), m_bgPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

// 拦截鼠标事件
bool TitleMenuView::eventFilter(QObject *watched, QEvent *event) {
    QPushButton* btn = qobject_cast<QPushButton*>(watched);
    if (btn && m_lightEffects.contains(btn)) {
        if (event->type() == QEvent::HoverEnter || event->type() == QEvent::Enter) {
            showHoverAnimation(btn);
        } else if (event->type() == QEvent::HoverLeave || event->type() == QEvent::Leave) {
            hideHoverAnimation(btn);
        }
    }
    return QWidget::eventFilter(watched, event);
}

// 启动光效动画
void TitleMenuView::showHoverAnimation(QPushButton* button) {
    QLabel* label = m_lightEffects[button];
    label->show();
    label->raise(); // 确保光斑在按钮上方显示

    // 计算起点和终点
    QRect startRect(button->x() - label->width(), button->y() + 5, label->width(), label->height());
    QRect endRect(button->x() + button->width(), button->y() + 5, label->width(), label->height());

    // 动态分配动画对象，防止冲突
    QPropertyAnimation* anim = new QPropertyAnimation(label, "geometry", this);
    anim->setDuration(500); // 500毫秒滑过
    anim->setStartValue(startRect);
    anim->setEndValue(endRect);
    anim->setEasingCurve(QEasingCurve::OutCubic); // 缓动曲线

    // 动画结束自动隐藏并销毁动画对象
    connect(anim, &QPropertyAnimation::finished, label, &QLabel::hide);
    connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);

    anim->start();
}

// 鼠标移出，立即隐藏
void TitleMenuView::hideHoverAnimation(QPushButton* button) {
    QLabel* label = m_lightEffects.value(button);
    if(label) {
        label->hide();
    }
}

void TitleMenuView::onStartGameClicked() {
    qDebug() << "[开始界面] 点击了开始游戏！准备切入大地图...";
    emit startGameRequested(); // 🔴 发射开战信号！

    // 假设大地图界面类叫 MapManager，并且你之后要在这里切换过去
    // 这里先保留 Debug 输出，证明按钮生效了
}

void TitleMenuView::onExitGameClicked() {
    qDebug() << "[开始界面] 退出游戏。";
    QApplication::quit();
}