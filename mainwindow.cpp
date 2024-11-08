#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::digitButtonClicked()
{
    // クリックされたボタンを取得
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if(clickedButton){
        QString digit = clickedButton->text();
        QString currentText = ui->textBrowser->toPlainText();

        // 現在のテキストが "0" であり、次に入力されるのが "0" 以外の数字の場合
        if (currentText == "0" && digit != "0") {
            // "0" を新しい数字で置き換える
            currentText = digit;
        }
        // 現在のテキストが "0" であり、次に "0" が入力された場合は何もしない（"0" を維持）
        else if (currentText == "0" && digit == "0") {
            // 何もしない
            return;
        }
        else {
            // その他の場合は数字を追加
            currentText += digit;
        }

        ui->textBrowser->setText(currentText);
    }
}

void MainWindow::on_pushButton_dot_clicked()
{
    QString currentText = ui->textBrowser->toPlainText();

    // 既に小数点が含まれている場合は追加しない
    if (!currentText.contains(".")) {
        if (currentText.isEmpty()) {
            // 表示が空の場合は "0." に設定
            currentText = "0.";
        }
        else if (currentText == "0") {
            // 表示が "0" の場合は "0." に設定
            currentText = "0.";
        }
        else {
            // その他の場合は小数点を追加
            currentText += ".";
        }

        ui->textBrowser->setText(currentText);
    }
}
