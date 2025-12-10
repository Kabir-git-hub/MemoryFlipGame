#ifndef CARD_H
#define CARD_H
#include <QPushButton>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QEnterEvent>
#include <QEvent>

class Card : public QPushButton
{
    Q_OBJECT

public:
    explicit Card(QWidget *parent = nullptr);
    void setupCard(int id, QString imagePath);
    void flip();
    void reset();

    // এনিমেশন ফাংশন
    void shake();
    void zoomEffect();
    void setMatched();

    int getId() const { return m_id; }
    bool isFlipped() const { return m_isFlipped; }
    bool isMatched() const { return m_isMatched; }

protected:
    // === নতুন মাউস হোভার ফাংশন ===
    void enterEvent(QEnterEvent *event) override; // মাউস কার্ডের ওপর আসলে
    void leaveEvent(QEvent *event) override;      // মাউস সরে গেলে

private slots:
    void onAnimationFinished();

private:
    int m_id;
    QString m_faceImage;
    bool m_isFlipped;
    bool m_isMatched;

    QPropertyAnimation *animation;
    bool isAnimatingForward;

    void updateAppearance();
};

#endif // CARD_H
