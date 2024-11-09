// mainwindow.cpp
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 最後の文字が演算子かどうかをチェック
bool MainWindow::isLastCharOperator()
{
    QString text = ui->textBrowser->toPlainText();
    if(text.isEmpty()) return false;
    QChar lastChar = text.at(text.length() - 1);
    return (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/' ||
            lastChar == QChar(0x00D7) || lastChar == QChar(0x00F7));   // '×' をQCharで追加 // '÷' をQCharで追加
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

// イコールボタンの処理
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

    // 演算子を統一（× を * に、÷ を / に）
    expression.replace(QChar(0x00D7), "*");
    expression.replace(QChar(0x00F7), "/");

    // トークン化（数字と演算子を分ける）
    QStringList tokens;
    QString numberBuffer = "";

    for(int i = 0; i < expression.length(); ++i){
        QChar c = expression[i];
        if(c.isDigit() || c == '.'){
            numberBuffer += c;
        }
        else {
            if(!numberBuffer.isEmpty()){
                tokens.append(numberBuffer);
                numberBuffer = "";
            }
            tokens.append(QString(c));
        }
    }
    if(!numberBuffer.isEmpty()){
        tokens.append(numberBuffer);
    }

    // 計算の実行（左から右へ順に計算）
    if(tokens.isEmpty()){
        return;
    }

    bool ok;
    double result = tokens[0].toDouble(&ok);
    if(!ok){
        ui->textBrowser->setText("Error");
        return;
    }

    for(int i = 1; i < tokens.size(); i += 2){
        if(i + 1 >= tokens.size()){
            break;
        }
        QString op = tokens[i];
        double num = tokens[i + 1].toDouble(&ok);
        if(!ok){
            ui->textBrowser->setText("Error");
            return;
        }

        if(op == "+"){
            result += num;
        }
        else if(op == "-"){
            result -= num;
        }
        else if(op == "*"){
            result *= num;
        }
        else if(op == "/"){
            if(num == 0){
                ui->textBrowser->setText("Error");
                return;
            }
            result /= num;
        }
        else{
            // 不明な演算子
            ui->textBrowser->setText("Error");
            return;
        }
    }

    // 結果を表示（小数点以下15桁まで）
    ui->textBrowser->setText(QString::number(result, 'g', 15));

    // 状態をリセット
    currentOperator = "";
    isOperatorClicked = false;
}
