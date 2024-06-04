#include "minesweeper.h"
#include "./ui_minesweeper.h"
#include <QRandomGenerator>
#include <QMessageBox>

Minesweeper::Minesweeper(QWidget *parent) // Initialize variables, mine count and grid size
    : QMainWindow(parent)
    , ui(new Ui::Minesweeper)
    , rows(20), cols(20), mineCount(10)
    , hintActive(false)
{
    ui->setupUi(this);
    setupUI();
    initializeGame();
}

Minesweeper::~Minesweeper() {
    delete ui;
}

void Minesweeper::setupUI() { // Initialize the game, ui parts, some specific properties

    QWidget *centralWidget = new QWidget(this);
    gridLayout = new QGridLayout(centralWidget);
    grid.resize(rows, QVector<QPushButton*>(cols));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            QPushButton *button = new QPushButton(this);
            button->setStyleSheet("background-image: url(:/assets/empty.png)");
            button->setFixedSize(15, 15);
            grid[row][col] = button;
            gridLayout->addWidget(button, row, col);
            connect(button, &QPushButton::clicked, this, &Minesweeper::handleCellClick);

            button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(button, &QPushButton::customContextMenuRequested, this, &Minesweeper::handleRightClick);
        }
    }

    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);



    QPushButton *restartButton = new QPushButton("Restart", this); // UI Restart Button
    connect(restartButton, &QPushButton::clicked, this, &Minesweeper::restartGame);
    gridLayout->addWidget(restartButton, rows, 0, 1, cols / 2);

    hintButton = new QPushButton("Hint", this); // UI for Hint button
    connect(hintButton, &QPushButton::clicked, this, &Minesweeper::giveHint);
    gridLayout->addWidget(hintButton, rows, cols / 2, 1, cols / 2);

    scoreLabel = new QLabel("Score: 0", this); // Initialize score label
    gridLayout->addWidget(scoreLabel, rows + 1, 0, 1, cols);

    setCentralWidget(centralWidget);
}



void Minesweeper::initializeGame() { // Set the score 0, everything is 0, set the mine field
    scoreLabel->setText("Score: ");
    mineField = QVector<QVector<int>>(rows, QVector<int>(cols, 0));
    hintActive = false;
    revealedCellsCount = 0;
    int minesPlaced = 0;
    while (minesPlaced < mineCount) {
        int row = QRandomGenerator::global()->bounded(rows);
        int col = QRandomGenerator::global()->bounded(cols);
        if (!isMine(row, col)) {
            mineField[row][col] = -1;
            ++minesPlaced;
        }
    }

    for (int row = 0; row < rows; ++row) { // Set all tiles' image to empty
        for (int col = 0; col < cols; ++col) {
            grid[row][col]->setStyleSheet("background-image: url(:/assets/empty.png)");
            grid[row][col]->setEnabled(true);
        }
    }
}



void Minesweeper::handleCellClick() { // All about left clicking a tile
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    int row = -1, col = -1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c] == clickedButton) {
                row = r;
                col = c;
                break;
            }
        }
    }
    if (row != -1 && col != -1) {
        revealCell(row, col);
        if (hintActive && row == hintRow && col == hintCol) {
            hintActive = false;
        }
    }
}

void Minesweeper::handleRightClick() { // Flag functionality
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    if (clickedButton->styleSheet() == "background-image: url(:/assets/flag.png)") {
        clickedButton->setStyleSheet("background-image: url(:/assets/empty.png)");
    } else {
        clickedButton->setStyleSheet("background-image: url(:/assets/flag.png)");
    }
}

void Minesweeper::restartGame() { // Restart button functionality
    initializeGame();
}

void Minesweeper::giveHint() { // All about hint
    if (hintActive) return;

    QVector<QPair<int, int>> revealedCells;
    // Collect revealed cells
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (!grid[row][col]->isEnabled()) {
                revealedCells.append(qMakePair(row, col));
            }
        }
    }

    if (revealedCells.isEmpty()) {
        // No revealed cells, nothing to hint
        return;
    }

    // Shuffle the revealed cells to give a random hint
    std::random_shuffle(revealedCells.begin(), revealedCells.end());

    // Iterate through revealed cells to find adjacent safe cell for hint
    for (const auto& cell : revealedCells) {
        int row = cell.first;
        int col = cell.second;

        for (int r = row - 1; r <= row + 1; ++r) {
            for (int c = col - 1; c <= col + 1; ++c) {
                if (r >= 0 && r < rows && c >= 0 && c < cols && !(r == row && c == col)) {
                    if (grid[r][c]->isEnabled() && !isMine(r, c)) {
                        // Found an adjacent safe cell, set it as hint
                        grid[r][c]->setStyleSheet("background-image: url(:/assets/hint.png)");
                        hintActive = true;
                        hintRow = r;
                        hintCol = c;
                        return;
                    }
                }
            }
        }
    }
}

void Minesweeper::revealCell(int row, int col) { // After left-click
    if (isMine(row, col)) { // If clicked cell contains a mine
        revealAllMines();
        QMessageBox::information(this, "Game Over", "You clicked on a mine!");
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                grid[r][c]->setEnabled(false);
            }
        }

        return;
    }
    // Count adjacent mines to show a number on tiles
    int adjacentMines = countAdjacentMines(row, col);
    if (adjacentMines == 0) {
        grid[row][col]->setStyleSheet("background-image: url(:/assets/0.png)");
    } else {
        grid[row][col]->setStyleSheet("background-image: url(:/assets/" + QString::number(adjacentMines) + ".png)");
    }
    grid[row][col]->setEnabled(false); // Make the revealed cell unclickable

    if (adjacentMines == 0) {
        revealAdjacentCells(row, col);
    }

    ++revealedCellsCount;
    scoreLabel->setText("Score: " + QString::number(revealedCellsCount));

    checkWinCondition();
}



void Minesweeper::revealAdjacentCells(int row, int col) { // Recursive algorithm to reveal adjacent cells continuosly
    for (int r = row - 1; r <= row + 1; ++r) {
        for (int c = col - 1; c <= col + 1; ++c) {
            if (r >= 0 && r < rows && c >= 0 && c < cols && !(r == row && c == col)) {
                if (grid[r][c]->isEnabled() && grid[r][c]->text().isEmpty()) {
                    revealCell(r, c);
                }
            }
        }
    }
}

int Minesweeper::countAdjacentMines(int row, int col) { //Function name :)
    int count = 0;
    for (int r = row - 1; r <= row + 1; ++r) {
        for (int c = col - 1; c <= col + 1; ++c) {
            if (isMine(r, c)) {
                ++count;
            }
        }
    }
    return count;
}

bool Minesweeper::isMine(int row, int col) { //  Check if is mine
    return row >= 0 && row < rows && col >= 0 && col < cols && mineField[row][col] == -1;
}

void Minesweeper::revealAllMines() { // Game is won or lost, reveal all mines
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (isMine(r, c)) {
                grid[r][c]->setStyleSheet("background-image: url(:/assets/mine.png)");
            }
            grid[r][c]->setEnabled(false);
        }
    }
}

void Minesweeper::checkWinCondition() { // Check win condition
    if (revealedCellsCount == (rows * cols - mineCount)) {
        QMessageBox::information(this, "Congratulations", "You won the game!");
        revealAllMines();
    }
}
