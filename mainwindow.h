#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTimer>
#include <QVector>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFrame>
#include <QProgressBar> // নতুন যুক্ত হয়েছে
#include <QDialog>
#include "card.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum DifficultyLevel { Easy, Medium, Hard };

private slots:
    void onCardClicked();
    void checkMatch();
    void updateTimer();         // গেমের মোট সময়ের টাইমার
    void updateTurnTimer();     // নতুন: টার্ন টাইমার (প্লেয়ারের জন্য)
    void onStartGameClicked();
    void restartGame();
    void togglePause();
    void toggleSound();
    void goToMenu();

private:
    void setupStartMenu();
    void setupGame();
    void showGameOverScreen(QString winnerName);
    void playSound(QString soundName);
    void switchTurn();
    void updatePlayerUI();
    void resetTurnTimer(); // টার্ন টাইমার রিসেট করার ফাংশন

    QWidget *mainContainer;

    QHBoxLayout *mainLayout;
    QVBoxLayout *centerLayout;
    QGridLayout *cardGridLayout;
    QWidget *gridContainer;

    QLabel *timerLabel;
    QLabel *pauseOverlay;

    // Player 1 UI
    QFrame *p1Panel;
    QLabel *p1NameLabel;
    QLabel *p1ScoreLabel;
    QProgressBar *p1TurnBar; // নতুন: প্লেয়ার ১ এর টাইম বার

    // Player 2 UI
    QFrame *p2Panel;
    QLabel *p2NameLabel;
    QLabel *p2ScoreLabel;
    QProgressBar *p2TurnBar; // নতুন: প্লেয়ার ২ এর টাইম বার

    QPushButton *restartButton;
    QPushButton *pauseButton;
    QPushButton *soundButton;

    QLineEdit *p1NameInput;
    QLineEdit *p2NameInput;

    QRadioButton *radioEasy;
    QRadioButton *radioMedium;
    QRadioButton *radioHard;
    QButtonGroup *difficultyGroup;
    DifficultyLevel currentLevel;

    QString player1Name;
    QString player2Name;

    QVector<Card*> cards;
    Card *firstCard;
    Card *secondCard;

    bool isProcessing;
    bool isPaused;
    bool isMuted;

    QTimer *gameTimer;      // মেইন গেম টাইমার
    QTimer *turnTimer;      // নতুন: টার্ন টাইমার
    int secondsElapsed;
    int turnTimeRemaining;  // টার্নের বাকি সময় (মিলিসেকেন্ড বা কাউন্ট)
    const int TURN_DURATION = 100; // ১০ সেকেন্ড (বার ১০০ ভাগে ভাগ হবে)

    QMediaPlayer *sfxPlayer;
    QAudioOutput *audioOutput;

    int matchedPairs;
    int totalPairs;

    int currentPlayer;
    int scoreP1;
    int scoreP2;
};

#endif // MAINWINDOW_H
