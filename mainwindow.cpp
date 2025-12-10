#include "mainwindow.h"
#include <QDialog>
#include <algorithm>
#include <random>
#include <chrono>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), firstCard(nullptr), secondCard(nullptr),
    isProcessing(false), matchedPairs(0),
    isPaused(false), isMuted(false), currentPlayer(1), scoreP1(0), scoreP2(0),
    currentLevel(Medium)
{
    this->setWindowTitle("Memory Flip - Ultimate Battle");
    this->resize(1150, 850);

    this->setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #141E30, stop:1 #243B55); }"
        );

    sfxPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    sfxPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);

    // ১. গেম টাইমার (মিনিট:সেকেন্ড)
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateTimer);

    // ২. টার্ন টাইমার (দ্রুত আপডেট হবে স্মুথ বারের জন্য)
    turnTimer = new QTimer(this);
    connect(turnTimer, &QTimer::timeout, this, &MainWindow::updateTurnTimer);

    setupStartMenu();
}

MainWindow::~MainWindow() {}

// ==========================================
// START MENU
// ==========================================
void MainWindow::setupStartMenu() {
    if(this->centralWidget()) delete this->centralWidget();

    QWidget *menuWidget = new QWidget(this);
    this->setCentralWidget(menuWidget);

    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setAlignment(Qt::AlignCenter);
    menuLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("MEMORY FLIP BATTLE", this);
    titleLabel->setStyleSheet("font-size: 50px; font-weight: bold; color: #F1C40F; margin-bottom: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    menuLayout->addWidget(titleLabel);

    QFrame *inputFrame = new QFrame(this);
    inputFrame->setFixedWidth(450);
    inputFrame->setStyleSheet("background-color: rgba(0,0,0,0.3); border-radius: 20px; padding: 20px;");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputFrame);

    QLabel *l1 = new QLabel("Player 1 Name:", this);
    l1->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    p1NameInput = new QLineEdit(this);
    p1NameInput->setPlaceholderText("Enter Name");
    p1NameInput->setStyleSheet("padding: 10px; font-size: 16px; border-radius: 5px; border: none; background: white; color: black;");

    QLabel *l2 = new QLabel("Player 2 Name:", this);
    l2->setStyleSheet("color: white; font-size: 16px; font-weight: bold; margin-top: 10px;");
    p2NameInput = new QLineEdit(this);
    p2NameInput->setPlaceholderText("Enter Name");
    p2NameInput->setStyleSheet("padding: 10px; font-size: 16px; border-radius: 5px; border: none; background: white; color: black;");

    inputLayout->addWidget(l1); inputLayout->addWidget(p1NameInput);
    inputLayout->addWidget(l2); inputLayout->addWidget(p2NameInput);

    QLabel *diffLabel = new QLabel("Select Difficulty:", this);
    diffLabel->setStyleSheet("color: #F1C40F; font-size: 18px; font-weight: bold; margin-top: 20px; margin-bottom: 10px;");
    diffLabel->setAlignment(Qt::AlignCenter);
    inputLayout->addWidget(diffLabel);

    QHBoxLayout *radioLayout = new QHBoxLayout();
    radioEasy = new QRadioButton("Easy", this);
    radioMedium = new QRadioButton("Medium", this);
    radioHard = new QRadioButton("Hard", this);

    QString radioStyle = "QRadioButton { color: white; font-size: 16px; font-weight: bold; } QRadioButton::indicator { width: 20px; height: 20px; border-radius: 10px; border: 2px solid white; } QRadioButton::indicator:checked { background-color: #2ECC71; border: 2px solid #2ECC71; }";
    radioEasy->setStyleSheet(radioStyle); radioMedium->setStyleSheet(radioStyle); radioHard->setStyleSheet(radioStyle);
    radioMedium->setChecked(true); currentLevel = Medium;

    radioLayout->addWidget(radioEasy); radioLayout->addWidget(radioMedium); radioLayout->addWidget(radioHard);
    inputLayout->addLayout(radioLayout);
    menuLayout->addWidget(inputFrame, 0, Qt::AlignCenter);

    QPushButton *startBtn = new QPushButton("START GAME", this);
    startBtn->setFixedSize(250, 60); startBtn->setCursor(Qt::PointingHandCursor);
    startBtn->setStyleSheet("QPushButton { background-color: #2ECC71; color: white; font-size: 22px; font-weight: bold; border-radius: 30px; } QPushButton:hover { background-color: #27AE60; }");
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartGameClicked);
    menuLayout->addWidget(startBtn, 0, Qt::AlignCenter);
}

void MainWindow::onStartGameClicked() {
    player1Name = p1NameInput->text().isEmpty() ? "Player 1" : p1NameInput->text();
    player2Name = p2NameInput->text().isEmpty() ? "Player 2" : p2NameInput->text();
    if(radioEasy->isChecked()) currentLevel = Easy;
    else if(radioHard->isChecked()) currentLevel = Hard;
    else currentLevel = Medium;
    setupGame();
}

// ==========================================
// GAME SETUP
// ==========================================
void MainWindow::setupGame() {
    if(this->centralWidget()) delete this->centralWidget();

    secondsElapsed = 0; matchedPairs = 0;
    scoreP1 = 0; scoreP2 = 0;
    currentPlayer = 1;
    firstCard = nullptr; secondCard = nullptr;
    isProcessing = false; isPaused = false;

    int colCount = 4;
    if (currentLevel == Easy) { totalPairs = 6; colCount = 4; }
    else if (currentLevel == Medium) { totalPairs = 8; colCount = 4; }
    else { totalPairs = 12; colCount = 6; }

    mainContainer = new QWidget(this);
    this->setCentralWidget(mainContainer);
    mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // --- Player 1 Panel ---
    p1Panel = new QFrame(this); p1Panel->setFixedWidth(240);
    QVBoxLayout *p1Layout = new QVBoxLayout(p1Panel);
    p1NameLabel = new QLabel(player1Name, p1Panel);
    p1NameLabel->setAlignment(Qt::AlignCenter);
    p1NameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    p1ScoreLabel = new QLabel("0", p1Panel);
    p1ScoreLabel->setAlignment(Qt::AlignCenter);
    p1ScoreLabel->setStyleSheet("font-size: 50px; font-weight: bold; color: #3498DB;");

    // P1 Progress Bar
    p1TurnBar = new QProgressBar(p1Panel);
    p1TurnBar->setRange(0, TURN_DURATION);
    p1TurnBar->setValue(TURN_DURATION);
    p1TurnBar->setTextVisible(false);
    p1TurnBar->setFixedHeight(10);
    p1TurnBar->setStyleSheet("QProgressBar { background-color: #333; border-radius: 5px; } QProgressBar::chunk { background-color: #2ECC71; border-radius: 5px; }");

    p1Layout->addWidget(p1NameLabel);
    p1Layout->addWidget(p1TurnBar); // বারের অবস্থান
    p1Layout->addWidget(p1ScoreLabel);
    p1Layout->addStretch();
    mainLayout->addWidget(p1Panel);

    // --- Center ---
    centerLayout = new QVBoxLayout();
    QHBoxLayout *headerLayout = new QHBoxLayout();
    soundButton = new QPushButton("🔊", this); soundButton->setFixedSize(40,40);
    soundButton->setStyleSheet("background-color: rgba(255,255,255,0.1); border-radius: 20px; color: white; font-size: 18px;");
    connect(soundButton, &QPushButton::clicked, this, &MainWindow::toggleSound);
    headerLayout->addWidget(soundButton);

    pauseButton = new QPushButton("⏸", this); pauseButton->setFixedSize(40,40);
    pauseButton->setStyleSheet("background-color: rgba(255,255,255,0.1); border-radius: 20px; color: white; font-size: 18px;");
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::togglePause);
    headerLayout->addWidget(pauseButton);

    headerLayout->addStretch();
    timerLabel = new QLabel("Time: 00:00", this);
    timerLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #F1C40F;");
    headerLayout->addWidget(timerLabel);
    centerLayout->addLayout(headerLayout);

    gridContainer = new QWidget(this);
    cardGridLayout = new QGridLayout(gridContainer);
    cardGridLayout->setSpacing(10);

    QVector<int> cardIds;
    for(int i=1; i<=totalPairs; i++) { cardIds.append(i); cardIds.append(i); }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(cardIds.begin(), cardIds.end(), std::default_random_engine(seed));

    for(int i=0; i<cardIds.size(); i++) {
        Card *newCard = new Card(this);
        int id = cardIds[i];
        int imageId = (id > 8) ? (id % 8) + 1 : id;
        QString imgPath = QString(":/assets/img%1.png").arg(imageId);
        newCard->setupCard(id, imgPath);
        connect(newCard, &Card::clicked, this, &MainWindow::onCardClicked);
        cardGridLayout->addWidget(newCard, i / colCount, i % colCount);
    }
    centerLayout->addWidget(gridContainer);

    pauseOverlay = new QLabel("GAME PAUSED", this);
    pauseOverlay->setAlignment(Qt::AlignCenter);
    pauseOverlay->setStyleSheet("font-size: 40px; font-weight: bold; color: white; background-color: rgba(0,0,0,0.6);");
    pauseOverlay->setVisible(false);

    QHBoxLayout *footerLayout = new QHBoxLayout();
    QPushButton *menuBtn = new QPushButton("Main Menu", this);
    menuBtn->setFixedSize(140, 45); menuBtn->setStyleSheet("background-color: #555; color: white; border-radius: 22px; font-weight: bold;");
    connect(menuBtn, &QPushButton::clicked, this, &MainWindow::goToMenu);
    restartButton = new QPushButton("Restart", this);
    restartButton->setFixedSize(160, 45); restartButton->setStyleSheet("background-color: #E74C3C; color: white; border-radius: 22px; font-weight: bold;");
    connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);
    footerLayout->addStretch(); footerLayout->addWidget(menuBtn); footerLayout->addWidget(restartButton); footerLayout->addStretch();
    centerLayout->addLayout(footerLayout);
    mainLayout->addLayout(centerLayout, 1);

    // --- Player 2 Panel ---
    p2Panel = new QFrame(this); p2Panel->setFixedWidth(240);
    QVBoxLayout *p2Layout = new QVBoxLayout(p2Panel);
    p2NameLabel = new QLabel(player2Name, p2Panel);
    p2NameLabel->setAlignment(Qt::AlignCenter);
    p2NameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    p2ScoreLabel = new QLabel("0", p2Panel);
    p2ScoreLabel->setAlignment(Qt::AlignCenter);
    p2ScoreLabel->setStyleSheet("font-size: 50px; font-weight: bold; color: #E74C3C;");

    // P2 Progress Bar
    p2TurnBar = new QProgressBar(p2Panel);
    p2TurnBar->setRange(0, TURN_DURATION);
    p2TurnBar->setValue(TURN_DURATION);
    p2TurnBar->setTextVisible(false);
    p2TurnBar->setFixedHeight(10);
    p2TurnBar->setStyleSheet("QProgressBar { background-color: #333; border-radius: 5px; } QProgressBar::chunk { background-color: #2ECC71; border-radius: 5px; }");

    p2Layout->addWidget(p2NameLabel);
    p2Layout->addWidget(p2TurnBar);
    p2Layout->addWidget(p2ScoreLabel);
    p2Layout->addStretch();
    mainLayout->addWidget(p2Panel);

    updatePlayerUI();
    resetTurnTimer(); // টাইমার সেট করা
    gameTimer->start(1000);
}

// ==========================================
// TURN TIMER LOGIC (NEW)
// ==========================================
void MainWindow::resetTurnTimer() {
    turnTimeRemaining = TURN_DURATION; // ১০০ পয়েন্ট (ধরলাম ১০ সেকেন্ড, প্রতি ১০০ms এ ১ কমবে)

    // সব বার ফুল করা
    p1TurnBar->setValue(TURN_DURATION);
    p2TurnBar->setValue(TURN_DURATION);

    // বার কালার সবুজ করা
    QString greenStyle = "QProgressBar { background-color: #333; border-radius: 5px; } QProgressBar::chunk { background-color: #2ECC71; border-radius: 5px; }";
    p1TurnBar->setStyleSheet(greenStyle);
    p2TurnBar->setStyleSheet(greenStyle);

    // টাইমার চালু (প্রতি ১০০ মিলিসেকেন্ডে কল হবে)
    turnTimer->start(100);
}

void MainWindow::updateTurnTimer() {
    if(isProcessing) return; // কার্ড ম্যাচ চেক করার সময় টাইমার কমবে না

    turnTimeRemaining--;

    // বর্তমান প্লেয়ারের বার আপডেট করা
    QProgressBar *currentBar = (currentPlayer == 1) ? p1TurnBar : p2TurnBar;
    currentBar->setValue(turnTimeRemaining);

    // কালার চেঞ্জ লজিক (৩০% এর কম হলে লাল, ৬০% হলে হলুদ)
    if(turnTimeRemaining < 30) {
        currentBar->setStyleSheet("QProgressBar { background-color: #333; border-radius: 5px; } QProgressBar::chunk { background-color: #E74C3C; border-radius: 5px; }"); // লাল
    } else if(turnTimeRemaining < 60) {
        currentBar->setStyleSheet("QProgressBar { background-color: #333; border-radius: 5px; } QProgressBar::chunk { background-color: #F1C40F; border-radius: 5px; }"); // হলুদ
    }

    // সময় শেষ হলে
    if(turnTimeRemaining <= 0) {
        playSound("wrong.wav"); // টাইম আপ সাউন্ড

        // যদি একটি কার্ড উল্টানো থাকে, সেটা নামিয়ে দাও
        if(firstCard) {
            firstCard->reset();
            firstCard = nullptr;
        }

        // টার্ন সুইচ করা
        switchTurn();
    }
}

void MainWindow::updatePlayerUI() {
    QString activeStyle = "QFrame { background-color: rgba(255, 255, 255, 0.2); border: 4px solid #2ECC71; border-radius: 15px; }";
    QString inactiveStyle = "QFrame { background-color: rgba(0, 0, 0, 0.2); border: 2px solid #555; border-radius: 15px; }";

    if(currentPlayer == 1) {
        p1Panel->setStyleSheet(activeStyle);
        p2Panel->setStyleSheet(inactiveStyle);
        p1TurnBar->setVisible(true); // যার টার্ন তার বার দেখাবে
        p2TurnBar->setVisible(false); // অন্যেরটা লুকাবে বা ডিম হবে
    }
    else {
        p1Panel->setStyleSheet(inactiveStyle);
        p2Panel->setStyleSheet(activeStyle);
        p1TurnBar->setVisible(false);
        p2TurnBar->setVisible(true);
    }
}

void MainWindow::switchTurn() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    updatePlayerUI();
    resetTurnTimer(); // নতুন টার্ন, নতুন সময়
}

// ==========================================
// OTHER LOGIC
// ==========================================
void MainWindow::onCardClicked() {
    if (isProcessing || isPaused) return;
    Card *clickedCard = qobject_cast<Card*>(sender());
    if (!clickedCard || clickedCard->isFlipped() || clickedCard->isMatched()) return;

    playSound("flip.wav");
    clickedCard->flip();

    if (!firstCard) {
        firstCard = clickedCard;
    } else {
        secondCard = clickedCard;
        isProcessing = true;
        turnTimer->stop(); // ম্যাচ চেক করার সময় টাইমার থামবে
        QTimer::singleShot(800, this, &MainWindow::checkMatch);
    }
}

void MainWindow::checkMatch() {
    if (firstCard->getId() == secondCard->getId()) {
        playSound("match.wav");
        firstCard->zoomEffect();
        secondCard->zoomEffect();
        matchedPairs++;

        if(currentPlayer == 1) { scoreP1++; p1ScoreLabel->setText(QString::number(scoreP1)); }
        else { scoreP2++; p2ScoreLabel->setText(QString::number(scoreP2)); }

        if (matchedPairs == totalPairs) {
            gameTimer->stop(); turnTimer->stop(); // সব থামাও
            QTimer::singleShot(1000, [this](){
                playSound("win.wav");
                QString winnerMsg;
                if(scoreP1 > scoreP2) winnerMsg = player1Name + " Wins!";
                else if (scoreP2 > scoreP1) winnerMsg = player2Name + " Wins!";
                else winnerMsg = "It's a Tie!";
                showGameOverScreen(winnerMsg);
            });
        } else {
            // মিলে গেলে একই প্লেয়ার আবার সুযোগ পাবে, তাই টাইমার রিসেট
            resetTurnTimer();
            isProcessing = false;
            firstCard = nullptr; secondCard = nullptr;
        }
    } else {
        playSound("wrong.wav");
        firstCard->shake();
        secondCard->shake();
        QTimer::singleShot(500, [this](){
            firstCard->reset(); secondCard->reset();
            firstCard = nullptr; secondCard = nullptr;
            isProcessing = false;
            switchTurn(); // টাইমার রিসেট switchTurn এর ভেতর আছে
        });
        return;
    }
}

void MainWindow::showGameOverScreen(QString winnerName) {
    QDialog gameOverDialog(this);
    gameOverDialog.setWindowTitle("Game Over");
    gameOverDialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    gameOverDialog.setAttribute(Qt::WA_TranslucentBackground);

    gameOverDialog.setStyleSheet(
        "QDialog { background-color: white; border: 4px solid #F1C40F; border-radius: 20px; }"
        "QLabel { color: #2C3E50; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(&gameOverDialog);
    layout->setSpacing(20); layout->setContentsMargins(40, 40, 40, 40);

    QLabel *icon = new QLabel("🏆", &gameOverDialog);
    icon->setStyleSheet("font-size: 80px; border: none; background: transparent;");
    icon->setAlignment(Qt::AlignCenter); layout->addWidget(icon);

    QLabel *title = new QLabel(winnerName, &gameOverDialog);
    title->setStyleSheet("font-size: 30px; font-weight: bold; color: #E67E22; border: none; background: transparent;");
    title->setAlignment(Qt::AlignCenter); layout->addWidget(title);

    QString details = QString("Player 1: %1  |  Player 2: %2\nTime: %3 sec").arg(scoreP1).arg(scoreP2).arg(secondsElapsed);
    QLabel *scoreLabel = new QLabel(details, &gameOverDialog);
    scoreLabel->setStyleSheet("font-size: 18px; color: #555; border: none; background: transparent;");
    scoreLabel->setAlignment(Qt::AlignCenter); layout->addWidget(scoreLabel);

    QPushButton *btn = new QPushButton("Play Again", &gameOverDialog);
    btn->setFixedSize(200, 50); btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; font-size: 18px; font-weight: bold; border-radius: 25px; } QPushButton:hover { background-color: #2ECC71; }");
    connect(btn, &QPushButton::clicked, &gameOverDialog, &QDialog::accept);
    layout->addWidget(btn, 0, Qt::AlignCenter);

    if(gameOverDialog.exec() == QDialog::Accepted) { setupGame(); }
}

void MainWindow::togglePause() {
    isPaused = !isPaused;
    if(isPaused) {
        gameTimer->stop(); turnTimer->stop(); // পজ দিলে টার্ন টাইমারও থামবে
        pauseButton->setText("▶");
        gridContainer->setVisible(false);
        centerLayout->insertWidget(1, pauseOverlay); pauseOverlay->setVisible(true);
    } else {
        gameTimer->start(1000); turnTimer->start(100);
        pauseButton->setText("⏸");
        pauseOverlay->setVisible(false); centerLayout->removeWidget(pauseOverlay);
        gridContainer->setVisible(true);
    }
}

void MainWindow::toggleSound() {
    isMuted = !isMuted;
    if(isMuted) { soundButton->setText("🔇"); audioOutput->setVolume(0); }
    else { soundButton->setText("🔊"); audioOutput->setVolume(1.0); }
}

void MainWindow::updateTimer() {
    secondsElapsed++;
    int m = secondsElapsed / 60; int s = secondsElapsed % 60;
    timerLabel->setText(QString("Time: %1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
}

void MainWindow::playSound(QString soundName) {
    if(isMuted) return;
    sfxPlayer->setSource(QUrl("qrc:/assets/" + soundName));
    sfxPlayer->play();
}

void MainWindow::restartGame() { gameTimer->stop(); turnTimer->stop(); setupGame(); }
void MainWindow::goToMenu() { gameTimer->stop(); turnTimer->stop(); setupStartMenu(); }
