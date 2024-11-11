// mainwindow.cpp
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QStack>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , firstNumber(0.0)
    , secondNumber(0.0)
    , currentOperator("")
    , isOperatorClicked(false)
    , isDegree(true) // 初期は度数法
{
    ui->setupUi(this);
    // ディスプレイの初期表示を "0" に設定
    ui->textBrowser->setText("0");

    // 数字ボタン（0-9）をリストに追加
    QList<QPushButton*> digitButtons = {
        ui->pushButton_0, ui->pushButton_1, ui->pushButton_2,
        ui->pushButton_3, ui->pushButton_4, ui->pushButton_5,
        ui->pushButton_6, ui->pushButton_7, ui->pushButton_8,
        ui->pushButton_9
    };

    // 各数字ボタンに対してスロットを接続
    for(auto button : digitButtons){
        connect(button, &QPushButton::clicked, this, &MainWindow::digitButtonClicked);
    }

    // 「00」ボタンに対してスロットを接続
    connect(ui->pushButton_00, &QPushButton::clicked, this, &MainWindow::on_pushButton_00_clicked);

    // 小数点ボタンに対してスロットを接続
    connect(ui->pushButton_dot, &QPushButton::clicked, this, &MainWindow::on_pushButton_dot_clicked);

    // クリアボタンに対してスロットを接続
    connect(ui->pushButton_Clear, &QPushButton::clicked, this, &MainWindow::on_pushButton_Clear_clicked);

    // 演算子ボタンに対してスロットを接続
    connect(ui->pushButton_Add, &QPushButton::clicked, this, &MainWindow::on_pushButton_Add_clicked);
    connect(ui->pushButton_Subtract, &QPushButton::clicked, this, &MainWindow::on_pushButton_Subtract_clicked);
    connect(ui->pushButton_Multiply, &QPushButton::clicked, this, &MainWindow::on_pushButton_Multiply_clicked);
    connect(ui->pushButton_Divide, &QPushButton::clicked, this, &MainWindow::on_pushButton_Divide_clicked);

    // イコールボタンに対してスロットを接続
    connect(ui->pushButton_Equal, &QPushButton::clicked, this, &MainWindow::on_pushButton_Equal_clicked);

    // 括弧ボタンに対してスロットを接続
    //connect(ui->pushButton_LeftParen, &QPushButton::clicked, this, &MainWindow::on_pushButton_LeftParen_clicked);
    //connect(ui->pushButton_RightParen, &QPushButton::clicked, this, &MainWindow::on_pushButton_RightParen_clicked);

    // 平方根ボタンに対してスロットを接続（修正）
    // connect(ui->pushButton_Sqrt, &QPushButton::clicked, this, &MainWindow::on_pushButton_Sqrt_clicked);

    // 三角関数ボタンに対してスロットを接続
    //connect(ui->pushButton_Sin, &QPushButton::clicked, this, &MainWindow::on_pushButton_Sin_clicked);
    //connect(ui->pushButton_Cos, &QPushButton::clicked, this, &MainWindow::on_pushButton_Cos_clicked);
    //connect(ui->pushButton_Tan, &QPushButton::clicked, this, &MainWindow::on_pushButton_Tan_clicked);

    // Deg/Radボタンに対してスロットを接続
    connect(ui->pushButton_DegRad, &QPushButton::clicked, this, &MainWindow::on_pushButton_DegRad_clicked);
    // カンマボタンに対してスロットを接続
    //connect(ui->pushButton_Comma, &QPushButton::clicked, this, &MainWindow::on_pushButton_Comma_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 演算子の優先順位を返す関数
int MainWindow::getPrecedence(const QString &op)
{
    if(op == "+" || op == "-") {
        return 1;
    }
    if(op == "*" || op == "/" || op == "×" || op == "÷") {
        return 2;
    }
    if(op == "^") {
        return 3; // べき乗は高い優先順位
    }
    if(op == "!") {
        return 4; // 階乗はさらに高い優先順位
    }
    return 0;
}

// 演算子の結合性を返す関数（左結合ならtrue）
bool MainWindow::isLeftAssociative(const QString &op)
{
    if(op == "^" || op == "!") {
        return false; // 右結合
    }
    // その他の演算子は左結合
    return true;
}

// 数式をトークンに分割する関数
QList<QString> MainWindow::tokenize(const QString &expression)
{
    QList<QString> tokens;
    QString numberBuffer;
    QString functionBuffer;

    for(int i = 0; i < expression.length(); ++i){
        QChar c = expression[i];

        if(c.isLetter()){
            functionBuffer += c;
            // 特殊な関数名の処理
            if(functionBuffer == "sqrt" || functionBuffer == "sin" || functionBuffer == "cos" ||
                functionBuffer == "tan" || functionBuffer == "exp" || functionBuffer == "log" ||
                functionBuffer == "ln" || functionBuffer == "nPr" || functionBuffer == "nCr"){
                tokens.append(functionBuffer);
                functionBuffer.clear();
            }
        }
        else if(c.isDigit() || c == '.'){
            if(!functionBuffer.isEmpty()){
                tokens.append(functionBuffer);
                functionBuffer.clear();
            }
            numberBuffer += c;
        }
        else{
            if(!functionBuffer.isEmpty()){
                tokens.append(functionBuffer);
                functionBuffer.clear();
            }
            if(!numberBuffer.isEmpty()){
                tokens.append(numberBuffer);
                numberBuffer.clear();
            }

            if(c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')' ||
                c == QChar(0x00D7) || c == QChar(0x00F7) || c == '!' || c == ','){
                tokens.append(QString(c));
            }
            else{
                // その他の文字は無視
            }
        }
    }

    if(!functionBuffer.isEmpty()){
        tokens.append(functionBuffer);
    }
    if(!numberBuffer.isEmpty()){
        tokens.append(numberBuffer);
    }

    // トークン化された結果を表示
    qDebug() << "Tokens:" << tokens;

    return tokens;
}

// Shunting YardアルゴリズムでトークンをRPNに変換する関数
QList<QString> MainWindow::shuntingYard(const QList<QString> &tokens)
{
    QList<QString> outputQueue;
    QStack<QString> operatorStack;

    for(const QString &token : tokens){
        bool isNumber;
        token.toDouble(&isNumber);
        if(isNumber){
            outputQueue.append(token);
        }
        else if(token == "sqrt" || token == "sin" || token == "cos" || token == "tan" ||
                 token == "exp" || token == "log" || token == "ln" ||
                 token == "nPr" || token == "nCr"){
            operatorStack.push(token);
        }
        else if(token == ","){
            while(!operatorStack.isEmpty() && operatorStack.top() != "("){
                outputQueue.append(operatorStack.pop());
            }
            if(operatorStack.isEmpty()){
                qDebug() << "Error: Mismatched parentheses or misplaced comma";
                outputQueue.clear();
                return outputQueue;
            }
            // カンマの場合、'(' はポップしない
        }
        else if(token == "+" || token == "-" || token == "*" || token == "/" || token == "^" ||
                 token == "×" || token == "÷" || token == "!"){
            while(!operatorStack.isEmpty()){
                QString topOp = operatorStack.top();
                if((getPrecedence(topOp) > getPrecedence(token)) ||
                    (getPrecedence(topOp) == getPrecedence(token) && isLeftAssociative(topOp))){
                    outputQueue.append(operatorStack.pop());
                }
                else{
                    break;
                }
            }
            operatorStack.push(token);
        }
        else if(token == "("){
            operatorStack.push(token);
        }
        else if(token == ")"){
            while(!operatorStack.isEmpty() && operatorStack.top() != "("){
                outputQueue.append(operatorStack.pop());
            }
            if(!operatorStack.isEmpty() && operatorStack.top() == "("){
                operatorStack.pop(); // '(' を取り除く
                if(!operatorStack.isEmpty() && (operatorStack.top() == "sqrt" || operatorStack.top() == "sin" ||
                                                 operatorStack.top() == "cos" || operatorStack.top() == "tan" ||
                                                 operatorStack.top() == "exp" || operatorStack.top() == "log" ||
                                                 operatorStack.top() == "ln" || operatorStack.top() == "nPr" ||
                                                 operatorStack.top() == "nCr")){
                    outputQueue.append(operatorStack.pop());
                }
            }
            else{
                // 括弧のバランスが取れていない場合
                qDebug() << "Error: Mismatched parentheses";
                outputQueue.clear();
                return outputQueue;
            }
        }
        else{
            // その他のトークンは無視
        }
    }

    while(!operatorStack.isEmpty()){
        QString topOp = operatorStack.pop();
        if(topOp == "(" || topOp == ")"){
            // 括弧のバランスが取れていない場合
            qDebug() << "Error: Mismatched parentheses in operator stack";
            outputQueue.clear();
            return outputQueue;
        }
        outputQueue.append(topOp);
    }

    return outputQueue;
}

// RPNを評価する関数
double MainWindow::evaluateRPN(const QList<QString> &rpnTokens, bool &success)
{
    QStack<double> evalStack;
    success = true;

    qDebug() << "Evaluating RPN Tokens:" << rpnTokens; // RPNトークンの表示

    for(const QString &token : rpnTokens){
        qDebug() << "Processing token:" << token << "Length:" << token.length();
        for(int i = 0; i < token.length(); ++i){
            qDebug() << "Character at position" << i << ":" << token.at(i) << "Code:" << token.at(i).unicode();
        }

        bool isNumber;
        double num = token.toDouble(&isNumber);
        if(isNumber){
            evalStack.push(num);
        }
        else if(token == "+" || token == "-" || token == "*" || token == "/" ||
                 token == "×" || token == "÷"){
            if(evalStack.size() < 2){
                success = false;
                qDebug() << "Error: Insufficient values in stack for operator" << token;
                return 0.0;
            }
            double right = evalStack.pop();
            double left = evalStack.pop();
            double result = 0.0;

            if(token == "+"){
                result = left + right;
            }
            else if(token == "-"){
                result = left - right;
            }
            else if(token == "*" || token == "×"){
                result = left * right;
            }
            else if(token == "/" || token == "÷"){
                if(right == 0){
                    success = false;
                    qDebug() << "Error: Division by zero";
                    return 0.0;
                }
                result = left / right;
            }

            evalStack.push(result);
        }
        else if(token == "^"){
            if(evalStack.size() < 2){
                success = false;
                qDebug() << "Error: Insufficient values in stack for operator '^'";
                return 0.0;
            }
            double exponent = evalStack.pop();
            double base = evalStack.pop();
            double result = std::pow(base, exponent);
            evalStack.push(result);
        }
        else if(token == "!"){
            if(evalStack.isEmpty()){
                success = false;
                qDebug() << "Error: Insufficient values in stack for operator '!'";
                return 0.0;
            }
            double operand = evalStack.pop();
            if(operand < 0 || operand != std::floor(operand)){
                success = false;
                qDebug() << "Error: Factorial of non-integer or negative number";
                return 0.0;
            }
            unsigned long long result = factorial(static_cast<int>(operand));
            evalStack.push(static_cast<double>(result));
        }
        else if(token == "nPr" || token == "nCr"){
            if(evalStack.size() < 2){
                success = false;
                qDebug() << "Error: Insufficient values in stack for function" << token;
                return 0.0;
            }
            double r = evalStack.pop();
            double n = evalStack.pop();

            if(n < 0 || r < 0 || n != std::floor(n) || r != std::floor(r)){
                success = false;
                qDebug() << "Error: n and r must be non-negative integers for function" << token;
                return 0.0;
            }
            if(n < r){
                success = false;
                qDebug() << "Error: n must be greater than or equal to r for function" << token;
                return 0.0;
            }

            unsigned long long result = 0;
            if(token == "nPr"){
                result = permutation(static_cast<int>(n), static_cast<int>(r));
            }
            else if(token == "nCr"){
                result = combination(static_cast<int>(n), static_cast<int>(r));
            }

            evalStack.push(static_cast<double>(result));
        }
        else if(token == "sqrt" || token == "sin" || token == "cos" || token == "tan" ||
                 token == "exp" || token == "log" || token == "ln"){
            if(evalStack.isEmpty()){
                success = false;
                qDebug() << "Error: Insufficient values in stack for function" << token;
                return 0.0;
            }
            double operand = evalStack.pop();
            double result = 0.0;

            if(token == "sqrt"){
                if(operand < 0){
                    success = false;
                    qDebug() << "Error: Square root of negative number";
                    return 0.0;
                }
                result = std::sqrt(operand);
            }
            else if(token == "sin"){
                if(isDegree){
                    operand = operand * M_PI / 180.0;
                }
                result = std::sin(operand);
            }
            else if(token == "cos"){
                if(isDegree){
                    operand = operand * M_PI / 180.0;
                }
                result = std::cos(operand);
            }
            else if(token == "tan"){
                if(isDegree){
                    operand = operand * M_PI / 180.0;
                }
                result = std::tan(operand);
            }
            else if(token == "exp"){
                result = std::exp(operand);
            }
            else if(token == "log"){
                if(operand <= 0){
                    success = false;
                    qDebug() << "Error: Logarithm of non-positive number";
                    return 0.0;
                }
                result = std::log10(operand);
            }
            else if(token == "ln"){
                if(operand <= 0){
                    success = false;
                    qDebug() << "Error: Natural logarithm of non-positive number";
                    return 0.0;
                }
                result = std::log(operand);
            }

            evalStack.push(result);
        }
        else{
            success = false;
            qDebug() << "Error: Unknown token" << token;
            return 0.0;
        }

        qDebug() << "Current Stack:" << evalStack; // スタックの状態を表示
    }

    if(evalStack.size() != 1){
        success = false;
        qDebug() << "Error: Stack size not equal to 1 after evaluation";
        return 0.0;
    }

    return evalStack.pop();
}

// 階乗、順列、組み合わせの関数
unsigned long long MainWindow::factorial(int n)
{
    if(n < 0){
        return 0;
    }
    unsigned long long result = 1;
    for(int i = 1; i <= n; ++i){
        result *= i;
        // オーバーフローを防ぐためにチェック
        if(result == 0){
            break;
        }
    }
    return result;
}

unsigned long long MainWindow::permutation(int n, int r)
{
    if(n < r || n < 0 || r < 0){
        return 0;
    }
    return factorial(n) / factorial(n - r);
}

unsigned long long MainWindow::combination(int n, int r)
{
    if(n < r || n < 0 || r < 0){
        return 0;
    }
    return factorial(n) / (factorial(r) * factorial(n - r));
}

// 最後の文字が演算子かどうかをチェック
bool MainWindow::isLastCharOperator()
{
    QString text = ui->textBrowser->toPlainText();
    if(text.isEmpty()) return false;
    QChar lastChar = text.at(text.length() - 1);
    return (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/' ||
            lastChar == QChar(0x00D7) || lastChar == QChar(0x00F7) || lastChar == '^');
}

// 数字入力に関しての共通の処理
void MainWindow::digitButtonClicked()
{
    // クリックされたボタンを取得
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if(clickedButton){
        QString digit = clickedButton->text();
        QString currentText = ui->textBrowser->toPlainText();

        // 演算子がクリックされた後の場合
        if(isOperatorClicked){
            // 演算子が最後に表示されているか確認
            if(isLastCharOperator()){
                // 演算子の後に数字を追加
                currentText += digit;
            }
            else{
                // 通常の数字追加
                currentText += digit;
            }
            isOperatorClicked = false;
        }
        else{
            // 現在の表示が "0" の場合、"0" を新しい数字で置き換える
            if (currentText == "0") {
                if (digit != "0") {
                    currentText = digit;
                }
                // もし "0" が押された場合は何もしない（"0" を維持）
                else {
                    return;
                }
            }
            else {
                currentText += digit;
            }
        }

        ui->textBrowser->setText(currentText);
    }
}

// 小数点ボタンの処理
void MainWindow::on_pushButton_dot_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 演算子がクリックされた後の場合
    if(isOperatorClicked){
        // ディスプレイを "0." に置き換える
        if(isLastCharOperator()){
            currentText += "0.";
        }
        else{
            currentText += ".";
        }
        isOperatorClicked = false;
        ui->textBrowser->setText(currentText);
        return;
    }

    // 既に小数点が含まれている場合は追加しない
    if (currentOperator.isEmpty()) {
        if (currentText.contains(".")) {
            return;
        }
    }
    else{
        QString secondNumberStr = currentText.mid(currentText.lastIndexOf(currentOperator) + 1);
        if(secondNumberStr.contains(".")){
            return;
        }
    }

    // 表示が演算子で終わっている場合は "0." を追加
    if(isLastCharOperator()){
        ui->textBrowser->setText(currentText + "0.");
    }
    // 表示が "0" の場合は "0." に設定
    else if (currentText == "0") {
        ui->textBrowser->setText("0.");
    }
    else {
        ui->textBrowser->setText(currentText + ".");
    }
}

// "00"ボタンの処理
void MainWindow::on_pushButton_00_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 演算子がクリックされた後の場合
    if(isOperatorClicked){
        // 演算子が最後に表示されているか確認
        if(isLastCharOperator()){
            currentText += "00";
        }
        else{
            currentText += "00";
        }
        isOperatorClicked = false;
        ui->textBrowser->setText(currentText);
        return;
    }

    // 現在の表示が "0" の場合、"0" のままにする
    if (currentText == "0") {
        return;
    }
    else {
        currentText += "00";
    }

    ui->textBrowser->setText(currentText);
}

// クリアボタンの処理
void MainWindow::on_pushButton_Clear_clicked()
{
    // ディスプレイを "0" にリセット
    ui->textBrowser->setText("0");

    // 内部変数をリセット
    firstNumber = 0.0;
    secondNumber = 0.0;
    currentOperator = "";
    isOperatorClicked = false;
}

// 足し算ボタンの処理
void MainWindow::on_pushButton_Add_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 現在の表示が "Error" でないことを確認
    if(currentText != "Error"){
        // 演算子がクリックされた後の場合、最後の演算子を置き換える
        if(isOperatorClicked){
            if(isLastCharOperator()){
                currentText.chop(1); // 演算子を削除
                currentText += "+";    // ディスプレイに「+」を追加
                ui->textBrowser->setText(currentText);
                currentOperator = "+"; // 内部ロジックでは「+」を使用
            }
            return;
        }

        // ディスプレイが "0" の場合は置き換える
        if(currentText == "0"){
            ui->textBrowser->setText("0+");
        }
        else{
            ui->textBrowser->setText(currentText + "+");
        }

        // 現在の演算子を "+" に設定
        currentOperator = "+";

        // 演算子がクリックされたことをフラグで示す
        isOperatorClicked = true;
    }
}

// 引き算ボタンの処理
void MainWindow::on_pushButton_Subtract_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 現在の表示が "Error" でないことを確認
    if(currentText != "Error"){
        // 演算子がクリックされた後の場合、最後の演算子を置き換える
        if(isOperatorClicked){
            if(isLastCharOperator()){
                currentText.chop(1); // 演算子を削除
                currentText += "-";    // ディスプレイに「-」を追加
                ui->textBrowser->setText(currentText);
                currentOperator = "-"; // 内部ロジックでは「-」を使用
            }
            return;
        }

        // ディスプレイが "0" の場合は置き換える
        if(currentText == "0"){
            ui->textBrowser->setText("0-");
        }
        else{
            ui->textBrowser->setText(currentText + "-");
        }

        // 現在の演算子を "-" に設定
        currentOperator = "-";

        // 演算子がクリックされたことをフラグで示す
        isOperatorClicked = true;
    }
}

// 掛け算ボタンの処理
void MainWindow::on_pushButton_Multiply_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 現在の表示が "Error" でないことを確認
    if(currentText != "Error"){
        // 演算子がクリックされた後の場合、最後の演算子を置き換える
        if(isOperatorClicked){
            if(isLastCharOperator()){
                currentText.chop(1); // 演算子を削除
                currentText += QChar(0x00D7);   // ディスプレイに「×」を追加
                ui->textBrowser->setText(currentText);
                currentOperator = "*"; // 内部ロジックでは「*」を使用
            }
            return;
        }

        // ディスプレイが "0" の場合は置き換える
        if(currentText == "0"){
            ui->textBrowser->setText("0×");
        }
        else{
            ui->textBrowser->setText(currentText + QChar(0x00D7));
        }

        // 現在の演算子を "*" に設定
        currentOperator = "*";

        // 演算子がクリックされたことをフラグで示す
        isOperatorClicked = true;
    }
}

// 割り算ボタンの処理
void MainWindow::on_pushButton_Divide_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 現在の表示が "Error" でないことを確認
    if(currentText != "Error"){
        // 演算子がクリックされた後の場合、最後の演算子を置き換える
        if(isOperatorClicked){
            if(isLastCharOperator()){
                currentText.chop(1); // 演算子を削除
                currentText += QChar(0x00F7);    // ディスプレイに「÷」を追加
                ui->textBrowser->setText(currentText);
                currentOperator = "/"; // 内部ロジックでは「/」を使用
            }
            return;
        }

        // ディスプレイが "0" の場合は置き換える
        if(currentText == "0"){
            ui->textBrowser->setText("0÷");
        }
        else{
            ui->textBrowser->setText(currentText + QChar(0x00F7));
        }

        // 現在の演算子を "/" に設定
        currentOperator = "/";

        // 演算子がクリックされたことをフラグで示す
        isOperatorClicked = true;
    }
}

// 括弧ボタンの処理
void MainWindow::on_pushButton_LeftParen_clicked()
{
    qDebug() << "Left Parenthesis Button Clicked"; // デバッグメッセージ
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "(" に置き換える
        currentText = "(";
    }
    else {
        // もし演算子がクリックされた後でなければ、左括弧の前に掛け算を追加する
        if(isOperatorClicked == false && !currentText.isEmpty()){
            // 最後の文字が数字または閉じ括弧である場合、掛け算を暗黙的に追加
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }
        currentText += "(";
    }

    ui->textBrowser->setText(currentText);
    isOperatorClicked = false;
}

void MainWindow::on_pushButton_RightParen_clicked()
{
    qDebug() << "Right Parenthesis Button Clicked"; // デバッグメッセージ
    QString currentText = ui->textBrowser->toPlainText();

    // 右括弧を追加する前に、括弧のバランスを確認
    int openParens = currentText.count('(');
    int closeParens = currentText.count(')');
    if(openParens > closeParens){
        currentText += ")";
        ui->textBrowser->setText(currentText);
    }
    // バランスが取れていない場合は追加しない
}

// イコールボタンの処理の修正
void MainWindow::on_pushButton_Equal_clicked()
{
    QString expression = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(expression == "Error"){
        return;
    }

    // 演算子が最後に表示されている場合は計算しない
    if(isLastCharOperator()){
        return;
    }

    // '×' を '*' に、'÷' を '/' に置き換え
    expression.replace(QChar(0x00D7), "*");
    expression.replace(QChar(0x00F7), "/");

    // Shunting Yardアルゴリズムで数式をトークン化
    QList<QString> tokens = tokenize(expression);
    if(tokens.isEmpty()){
        ui->textBrowser->setText("Error");
        return;
    }

    // Shunting YardアルゴリズムでRPNに変換
    QList<QString> rpn = shuntingYard(tokens);
    if(rpn.isEmpty()){
        ui->textBrowser->setText("Error");
        return;
    }

    // RPNを評価
    bool success;
    double result = evaluateRPN(rpn, success);
    if(!success){
        ui->textBrowser->setText("Error");
        return;
    }

    // 結果を表示（小数点以下15桁まで）
    ui->textBrowser->setText(QString::number(result, 'g', 15));

    // 状態をリセット
    currentOperator = "";
    isOperatorClicked = false;
}

// 平方根ボタンの処理
void MainWindow::on_pushButton_Sqrt_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "sqrt(" に置き換える
        currentText = "sqrt(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "sqrt(" を追加
        currentText += "sqrt(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

// 三角関数の処理
void MainWindow::on_pushButton_Sin_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "sin(" に置き換える
        currentText = "sin(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "sin(" を追加
        currentText += "sin(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

void MainWindow::on_pushButton_Cos_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "cos(" に置き換える
        currentText = "cos(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "cos(" を追加
        currentText += "cos(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

void MainWindow::on_pushButton_Tan_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "tan(" に置き換える
        currentText = "tan(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "tan(" を追加
        currentText += "tan(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

// Deg/Radボタンの処理
void MainWindow::on_pushButton_DegRad_clicked()
{
    isDegree = !isDegree; // 単位を切り替える

    if(isDegree){
        ui->pushButton_DegRad->setText("Deg");
    }
    else{
        ui->pushButton_DegRad->setText("Rad");
    }
}

//指数関数ボタンの処理
void MainWindow::on_pushButton_Power_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 演算子がクリックされた後の場合、最後の演算子を置き換える
    if(isOperatorClicked){
        if(isLastCharOperator()){
            currentText.chop(1); // 演算子を削除
            currentText += "^";    // ディスプレイに「^」を追加
            ui->textBrowser->setText(currentText);
            currentOperator = "^"; // 内部ロジックでは「^」を使用
        }
        return;
    }

    // ディスプレイが "0" の場合は置き換える
    if(currentText == "0"){
        ui->textBrowser->setText("0^");
    }
    else{
        ui->textBrowser->setText(currentText + "^");
    }

    // 現在の演算子を "^" に設定
    currentOperator = "^";

    // 演算子がクリックされたことをフラグで示す
    isOperatorClicked = true;
}

// expボタンの処理
void MainWindow::on_pushButton_Exp_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "exp(" に置き換える
        currentText = "exp(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "exp(" を追加
        currentText += "exp(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

// logボタンの処理
void MainWindow::on_pushButton_Log_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "log(" に置き換える
        currentText = "log(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "log(" を追加
        currentText += "log(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

// lnボタンの処理
void MainWindow::on_pushButton_Ln_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 特殊ケース: ディスプレイが "0" の場合
    if (currentText == "0") {
        // "0" を "ln(" に置き換える
        currentText = "ln(";
    }
    else {
        // もし直前に数字や閉じ括弧がある場合、掛け算を暗黙的に追加
        if(!currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }

        // "ln(" を追加
        currentText += "ln(";
    }

    ui->textBrowser->setText(currentText);

    isOperatorClicked = false;
}

// 階乗ボタンの処理
void MainWindow::on_pushButton_Factorial_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // 階乗は後置一項演算子なので、直前の数値や閉じ括弧に適用されます
    // そのため、階乗を直接追加します
    // ただし、演算子がクリックされた後の場合は追加しません
    if(isOperatorClicked){
        // 一項演算子なので、演算子がクリックされた後では適用できない
        return;
    }

    // 現在の表示が空でない場合にのみ階乗を追加
    if(!currentText.isEmpty()){
        QChar lastChar = currentText.at(currentText.length() - 1);
        // 数字または閉じ括弧の場合にのみ階乗を追加
        if(lastChar.isDigit() || lastChar == ')'){
            currentText += "!";
            ui->textBrowser->setText(currentText);
            // 階乗は一項演算子なので、演算子がクリックされた状態にはしない
            isOperatorClicked = false;
        }
    }
}

// 順列ボタンの処理
void MainWindow::on_pushButton_Permutation_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // ディスプレイが "0" の場合は "nPr(" に置き換える
    if(currentText == "0"){
        currentText = "nPr(";
    }
    else{
        // もし演算子がクリックされた後でなければ、順列の前に掛け算を追加する
        if(!isOperatorClicked && !currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')' || lastChar == '!'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }
        currentText += "nPr(";
    }

    ui->textBrowser->setText(currentText);
    isOperatorClicked = false;
}

// 組み合わせボタンの処理
void MainWindow::on_pushButton_Combination_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // ディスプレイが "0" の場合は "nCr(" に置き換える
    if(currentText == "0"){
        currentText = "nCr(";
    }
    else{
        // もし演算子がクリックされた後でなければ、組み合わせの前に掛け算を追加する
        if(!isOperatorClicked && !currentText.isEmpty()){
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')' || lastChar == '!'){
                currentText += QChar(0x00D7); // '×' を追加
            }
        }
        currentText += "nCr(";
    }

    ui->textBrowser->setText(currentText);
    isOperatorClicked = false;
}
// カンマボタンの処理
void MainWindow::on_pushButton_Comma_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // "Error" の場合は何もしない
    if(currentText == "Error"){
        return;
    }

    // カンマは関数の引数を区切るために使用されます
    // 演算子がクリックされた後ではなく、関数内でのみ有効
    // したがって、関数が開かれているかどうかを確認する必要があります

    // 現在のテキストにカンマを追加
    currentText += ",";
    ui->textBrowser->setText(currentText);

    // カンマは演算子ではないため、isOperatorClicked フラグは変更しません
}
