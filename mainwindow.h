// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextBrowser>
#include <QStack>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
enum Operator {
    NONE,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    POWER,
    FACTORIAL ,
    COMBINATION,
    PERMUTATION
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void digitButtonClicked();               // 数字ボタンクリック時の共通スロット
    void on_pushButton_dot_clicked();        // 小数点ボタンクリック時のスロット
    void on_pushButton_00_clicked();         // "00" ボタンクリック時のスロット
    void on_pushButton_Clear_clicked();       // クリアボタンクリック時のスロット
    void on_pushButton_Add_clicked();         // 足し算ボタンクリック時のスロット
    void on_pushButton_Subtract_clicked();    // 引き算ボタンクリック時のスロット
    void on_pushButton_Multiply_clicked();    // 乗算ボタンクリック時のスロット
    void on_pushButton_Divide_clicked();      // 除算ボタンクリック時のスロット
    void on_pushButton_Equal_clicked();       // イコールボタンクリック時のスロット
    void on_pushButton_LeftParen_clicked();   // 括弧ボタン用のスロット
    void on_pushButton_RightParen_clicked();  // 括弧ボタン用のスロット
    void on_pushButton_Sqrt_clicked();         //平方根用のスロット
    //三角関数用のスロット
    void on_pushButton_Sin_clicked();
    void on_pushButton_Cos_clicked();
    void on_pushButton_Tan_clicked();

    void on_pushButton_DegRad_clicked();        //度数法とラジアン切り替え用のスロット
    void on_pushButton_Power_clicked();         //指数関数用のスロット
    void on_pushButton_Exp_clicked();         // expボタンクリック時のスロット
    void on_pushButton_Log_clicked();         // logボタンクリック時のスロット
    void on_pushButton_Ln_clicked();          // lnボタンクリック時のスロット
    void on_pushButton_Factorial_clicked(); // 階乗ボタンクリック時のスロット
    void on_pushButton_Combination_clicked();    // 組み合わせボタンクリック時のスロット
    void on_pushButton_Permutation_clicked();    // 順列ボタンクリック時のスロット


private:
    Ui::MainWindow *ui;


    double firstNumber;                         // 最初の数値
    double secondNumber;                        // 演算子の後の数値
    QString currentOperator;                    // 現在の演算子
    bool isOperatorClicked;                     // 演算子がクリックされたかどうか
    bool isLastCharOperator();                  // 最後の文字が演算子かどうかをチェックする関数
    int getPrecedence(const QString &op);
    bool isLeftAssociative(const QString &op);
    QList<QString> tokenize(const QString &expression);
    QList<QString> shuntingYard(const QList<QString> &tokens);
    double evaluateRPN(const QList<QString> &rpnTokens, bool &success);
    bool isDegree; // true: 度数法, false: ラジアン
    unsigned long long factorial(int n);           // 階乗を計算する関数
    unsigned long long permutation(int n, int r);  // 順列 nPr を計算する関数
    unsigned long long combination(int n, int r);  // 組み合わせ nCr を計算する関数
};

#endif // MAINWINDOW_H
