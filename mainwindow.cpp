// mainwindow.cpp
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QStack>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , firstNumber(0.0)
    , secondNumber(0.0)
    , currentOperator("")
    , isOperatorClicked(false)
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
    // 自動接続を利用する場合は以下のコメントを外す
    // connect(ui->pushButton_LeftParen, &QPushButton::clicked, this, &MainWindow::on_pushButton_LeftParen_clicked);
    // connect(ui->pushButton_RightParen, &QPushButton::clicked, this, &MainWindow::on_pushButton_RightParen_clicked);

    // 平方根ボタンに対してスロットを接続（修正）
    //connect(ui->pushButton_Sqrt, &QPushButton::clicked, this, &MainWindow::on_pushButton_Sqrt_clicked);
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
    return 0;
}

// 演算子の結合性を返す関数（左結合ならtrue）
bool MainWindow::isLeftAssociative(const QString &op)
{
    // 現在の演算子はすべて左結合
    return true;
}

// 数式をトークンに分割する関数
QList<QString> MainWindow::tokenize(const QString &expression)
{
    QList<QString> tokens;
    QString numberBuffer;
    QString functionBuffer; // 追加

    for(int i = 0; i < expression.length(); ++i){
        QChar c = expression[i];

        if(c.isLetter()){
            functionBuffer += c;
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

            if(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' ||
                c == QChar(0x00D7) || c == QChar(0x00F7)){
                tokens.append(QString(c));
            }
            // その他の文字は無視
        }
    }

    if(!functionBuffer.isEmpty()){
        tokens.append(functionBuffer);
    }
    if(!numberBuffer.isEmpty()){
        tokens.append(numberBuffer);
    }

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
        else if(token == "sqrt"){
            operatorStack.push(token);
        }
        else if(token == "+" || token == "-" || token == "*" || token == "/" || token == "×" || token == "÷"){
            while(!operatorStack.isEmpty()){
                QString topOp = operatorStack.top();
                if((getPrecedence(topOp) > getPrecedence(token)) ||
                    (getPrecedence(topOp) == getPrecedence(token) && isLeftAssociative(topOp)) ||
                    (topOp == "sqrt")){ // 追加
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
                if(!operatorStack.isEmpty() && operatorStack.top() == "sqrt"){
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

    for(const QString &token : rpnTokens){
        bool isNumber;
        double num = token.toDouble(&isNumber);
        if(isNumber){
            evalStack.push(num);
        }
        else if(token == "sqrt"){
            if(evalStack.isEmpty()){
                success = false;
                qDebug() << "Error: Insufficient values in stack for function sqrt";
                return 0.0;
            }
            double operand = evalStack.pop();
            if(operand < 0){
                success = false;
                qDebug() << "Error: Square root of negative number";
                return 0.0;
            }
            double result = std::sqrt(operand);
            evalStack.push(result);
        }
        else{
            if(evalStack.size() < 2){
                success = false;
                qDebug() << "Error: Insufficient values in stack for operation" << token;
                return 0.0;
            }
            double right = evalStack.pop();
            double left = evalStack.pop();
            double result = 0.0;

            if(token == "+" ){
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
            else{
                success = false;
                qDebug() << "Error: Unknown operator" << token;
                return 0.0;
            }

            evalStack.push(result);
        }
    }

    if(evalStack.size() != 1){
        success = false;
        qDebug() << "Error: Stack size not equal to 1 after evaluation";
        return 0.0;
    }

    return evalStack.pop();
}

// 最後の文字が演算子かどうかをチェック
bool MainWindow::isLastCharOperator()
{
    QString text = ui->textBrowser->toPlainText();
    if(text.isEmpty()) return false;
    QChar lastChar = text.at(text.length() - 1);
    return (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/' ||
            lastChar == QChar(0x00D7) || lastChar == QChar(0x00F7));   // '×' と '÷' をQCharで追加
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
        // もし演算子がクリックされた後でなければ、左括弧の前に演算子を追加する
        if(isOperatorClicked == false && !currentText.isEmpty()){
            // 最後の文字が数字または閉じ括弧である場合、掛け算を暗黙的に追加
            QChar lastChar = currentText.at(currentText.length() - 1);
            if(lastChar.isDigit() || lastChar == ')'){
                currentText += QChar(0x00D7); // '×' を追加
                currentText += "(";
            }
            else{
                currentText += "(";
            }
        }
        else{
            currentText += "(";
        }
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

// 平方根ボタンの処理（修正後）
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
