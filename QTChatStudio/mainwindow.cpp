#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include "QDebug"

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
    connect(ui->action_new_chat, &QAction::triggered, this, &MainWindow::onNewChatActionTriggered);
    
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

void MainWindow::onSaveChatButtonClicked()
{
    
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

void MainWindow::onNewChatActionTriggered()
{
    int tabCount = ui->tabWidget->count();
    QString tabName = QString("Chat %1").arg(tabCount + 1);

    // Create a new QTextEdit for the chat
    QTextEdit *newChat = new QTextEdit(this);
    qDebug() << "Creating new chat tab:" << tabName;
    newChat->setReadOnly(true);

    // Add the new tab with the QTextEdit
    ui->tabWidget->addTab(newChat, tabName);
    ui->tabWidget->setCurrentIndex(tabCount);
}

void MainWindow::addMessageToChat(const QString &sender, const QString &message)
{
    QString formattedMessage = QString("<b>%1:</b> %2<br>").arg(sender, message);
    ui->te_chat->append(formattedMessage);
}
