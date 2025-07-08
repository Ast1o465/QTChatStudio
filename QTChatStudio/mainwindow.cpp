#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    m_chatEngine = new aichat(this);
    setupConnections();
    
    // Set default models
    ui->cb_models->addItems({"llama3.2", "DeepSeek-R1", "codellama", "gemma3:1b"});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->btn_send, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(m_chatEngine, &aichat::responseReady, this, &MainWindow::onAiResponseReceived);
    connect(m_chatEngine, &aichat::errorOccurred, this, &MainWindow::onAiError);
    
    // Send message on Enter key press
    connect(ui->le_input, &QLineEdit::returnPressed, this, &MainWindow::onSendButtonClicked);
}

void MainWindow::onSendButtonClicked()
{
    QString userMessage = ui->le_input->text().trimmed();
    if (userMessage.isEmpty()) return;
    
    addMessageToChat("You", userMessage);
    ui->le_input->clear();
    
    QString selectedModel = ui->cb_models->currentText();
    m_chatEngine->sendMessage(userMessage, selectedModel);
    
    ui->btn_send->setEnabled(false);
    ui->btn_send->setText("Sending...");
}

void MainWindow::onAiResponseReceived(const QString &response)
{
    addMessageToChat("AI", response);
    ui->btn_send->setEnabled(true);
    ui->btn_send->setText("Send");
}

void MainWindow::onAiError(const QString &error)
{
    addMessageToChat("System", QString("Error: %1").arg(error));
    ui->btn_send->setEnabled(true);
    ui->btn_send->setText("Send");
}

void MainWindow::addMessageToChat(const QString &sender, const QString &message)
{
    QString formattedMessage = QString("<b>%1:</b> %2<br>").arg(sender, message);
    ui->te_chat->append(formattedMessage);
}
