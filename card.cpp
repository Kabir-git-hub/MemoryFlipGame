#include "card.h"
#include <QSequentialAnimationGroup>

Card::Card(QWidget *parent)
    : QPushButton(parent), m_isFlipped(false), m_isMatched(false)
{
    this->setFixedSize(110, 110);
    this->setCursor(Qt::PointingHandCursor);

    animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(150);
    connect(animation, &QPropertyAnimation::finished, this, &Card::onAnimationFinished);

    updateAppearance();
}

void Card::setupCard(int id, QString imagePath) {
    m_id = id;
    m_faceImage = imagePath;
}

// === ১. মাউস কার্ডের ওপর আসলে (HOVER ENTER) ===
void Card::enterEvent(QEnterEvent *event) {
    // যদি কার্ড উল্টানো থাকে বা ম্যাচ হয়ে থাকে, তাহলে কিছু হবে না
    if(m_isMatched || m_isFlipped) {
        QPushButton::enterEvent(event);
        return;
    }

    // হোভার স্টাইল: উজ্জ্বল বর্ডার (Gold Color)
    this->setStyleSheet(
        "QPushButton {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FF512F, stop:1 #DD2476);"
        "   border: 4px solid #F1C40F;" /* হলুদ গ্লো বর্ডার */
        "   border-radius: 12px;"
        "}"
        );

    QPushButton::enterEvent(event);
}

// === ২. মাউস কার্ড থেকে সরে গেলে (HOVER LEAVE) ===
void Card::leaveEvent(QEvent *event) {
    // যদি কার্ড উল্টানো বা ম্যাচ হয়ে থাকে, কিছু হবে না
    if(m_isMatched || m_isFlipped) {
        QPushButton::leaveEvent(event);
        return;
    }

    // আগের স্টাইলে ফিরে যাওয়া
    updateAppearance();

    QPushButton::leaveEvent(event);
}

void Card::flip() {
    if (m_isMatched || m_isFlipped) return;

    isAnimatingForward = true;
    m_isFlipped = true;

    QRect startRect = this->geometry();
    QRect endRect = QRect(startRect.x() + 55, startRect.y(), 0, 110);

    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    animation->start();
}

void Card::shake() {
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);

    // লাল বর্ডার (ভুল হলে)
    this->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; border: 3px solid #E74C3C; border-radius: 12px; }"
        );

    for(int i=0; i<3; i++){
        QPropertyAnimation *animRight = new QPropertyAnimation(this, "pos");
        animRight->setDuration(50);
        animRight->setStartValue(pos());
        animRight->setEndValue(QPoint(x() + 5, y()));

        QPropertyAnimation *animLeft = new QPropertyAnimation(this, "pos");
        animLeft->setDuration(50);
        animLeft->setStartValue(QPoint(x() + 5, y()));
        animLeft->setEndValue(QPoint(x() - 5, y()));

        group->addAnimation(animRight);
        group->addAnimation(animLeft);
    }

    QPropertyAnimation *animBack = new QPropertyAnimation(this, "pos");
    animBack->setDuration(50);
    animBack->setStartValue(QPoint(x() - 5, y()));
    animBack->setEndValue(pos());
    group->addAnimation(animBack);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void Card::zoomEffect() {
    // সবুজ বর্ডার (ম্যাচ হলে)
    this->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; border: 3px solid #2ECC71; border-radius: 12px; }"
        );

    QPropertyAnimation *zoomAnim = new QPropertyAnimation(this, "geometry");
    zoomAnim->setDuration(300);
    QRect start = geometry();
    zoomAnim->setStartValue(start);
    zoomAnim->setEndValue(QRect(start.x()-10, start.y()-10, start.width()+20, start.height()+20));
    zoomAnim->setEasingCurve(QEasingCurve::OutBounce);

    connect(zoomAnim, &QPropertyAnimation::finished, this, [this](){
        this->setMatched();
    });

    zoomAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Card::reset() {
    if(m_isMatched) return;

    isAnimatingForward = true;
    m_isFlipped = false;

    QRect startRect = this->geometry();
    QRect endRect = QRect(startRect.x() + 55, startRect.y(), 0, 110);

    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    animation->start();
}

void Card::onAnimationFinished() {
    if (isAnimatingForward) {
        updateAppearance();
        isAnimatingForward = false;

        QRect currentRect = this->geometry();
        QRect finalRect = QRect(currentRect.x() - 55, currentRect.y(), 110, 110);

        animation->setStartValue(currentRect);
        animation->setEndValue(finalRect);
        animation->start();
    }
}

void Card::setMatched() {
    m_isMatched = true;
    this->setEnabled(false);
    this->setVisible(false);
}

void Card::updateAppearance() {
    if (m_isFlipped) {
        // ফেস আপ (ছবি)
        this->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; border: 2px solid #fff; border-radius: 12px; }"
            );
        if (!m_faceImage.isEmpty()) {
            this->setIcon(QIcon(m_faceImage));
            this->setIconSize(QSize(100, 100));
            this->setText("");
        }
    } else {
        // ফেস ডাউন (স্বাভাবিক অবস্থা)
        this->setIcon(QIcon());
        this->setStyleSheet(
            "QPushButton {"
            "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FF512F, stop:1 #DD2476);"
            "   border: 2px solid #FFFFFF;"
            "   border-radius: 12px;"
            "}"
            );
    }
}
