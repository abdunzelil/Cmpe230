#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QVector>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class Minesweeper; }
QT_END_NAMESPACE

class Minesweeper : public QMainWindow {
    Q_OBJECT

public:
    Minesweeper(QWidget *parent = nullptr);
    ~Minesweeper();

private slots:
    void handleCellClick();
    void handleRightClick();
    void restartGame();
    void giveHint();

private:
    void setupUI();
    void initializeGame();
    void revealCell(int row, int col);
    void revealAdjacentCells(int row, int col);
    int countAdjacentMines(int row, int col);
    bool isMine(int row, int col);
    void disableHint();
    void revealAllMines();
    void checkWinCondition();

    Ui::Minesweeper *ui;
    QGridLayout *gridLayout;
    QVector<QVector<QPushButton*>> grid;
    QVector<QVector<int>> mineField;
    int rows;
    int cols;
    int mineCount;
    QPushButton *hintButton;
    bool hintActive;
    int hintRow;
    int hintCol;
    QLabel *scoreLabel;
    int revealedCellsCount;
};

#endif // MINESWEEPER_H
