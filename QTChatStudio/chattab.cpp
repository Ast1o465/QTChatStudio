#include "chattab.h"
#include "ui_chattab.h"
#include "aichat.h"
#include <QDebug>

chatTab::chatTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::chatTab)
    , m_chatEngine(nullptr)
{
    ui->setupUi(this);
}

chatTab::~chatTab()
{
    delete ui;
}

void chatTab::setAiEngine(aichat *engine)
{
    m_chatEngine = engine;
}

void chatTab::setModels(const QStringList &models)
{
    ui->cb_models->clear();
    ui->cb_models->addItems(models);
}

void chatTab::setupConnections()
{
    // Setup connections similar to those in MainWindow
    connect(ui->btn_send, &QPushButton::clicked, this, &chatTab::onSendButtonClicked);
    connect(ui->le_input, &QLineEdit::returnPressed, this, &chatTab::onSendButtonClicked);
    
    // Connections to AI engine are set up dynamically when sending a message
}

void chatTab::onSendButtonClicked()
{
    QString userMessage = ui->le_input->text().trimmed();
    if (userMessage.isEmpty()) return;
    
    addMessageToChat("You", userMessage);
    ui->le_input->clear();
    
    if (m_chatEngine) {
        QString selectedModel = ui->cb_models->currentText();
        
        // Create temporary connections for receiving a response
        connect(m_chatEngine, &aichat::responseReady, this, &chatTab::onAiResponseReceived, Qt::UniqueConnection);
        connect(m_chatEngine, &aichat::errorOccurred, this, &chatTab::onAiError, Qt::UniqueConnection);
        
        // Send the message
        m_chatEngine->sendMessage(userMessage, selectedModel);
        
        ui->btn_send->setEnabled(false);
        ui->btn_send->setText("Sending...");
    } else {
        addMessageToChat("System", "Error: AI engine is not available");
    }
}

void chatTab::onAiResponseReceived(const QString &response)
{
    addMessageToChat("AI", response);
    ui->btn_send->setEnabled(true);
    ui->btn_send->setText("Send");
    
    // Disconnect temporary connections
    disconnect(m_chatEngine, &aichat::responseReady, this, &chatTab::onAiResponseReceived);
    disconnect(m_chatEngine, &aichat::errorOccurred, this, &chatTab::onAiError);
}

void chatTab::onAiError(const QString &error)
{
    addMessageToChat("System", QString("Error: %1").arg(error));
    ui->btn_send->setEnabled(true);
    ui->btn_send->setText("Send");
    
    // Disconnect temporary connections
    disconnect(m_chatEngine, &aichat::responseReady, this, &chatTab::onAiResponseReceived);
    disconnect(m_chatEngine, &aichat::errorOccurred, this, &chatTab::onAiError);
}

void chatTab::addMessageToChat(const QString &sender, const QString &message)
{
    QString formattedMessage = QString("<b>%1:</b> %2<br>").arg(sender, message);
    ui->te_chat->append(formattedMessage);
}

QString chatTab::getChatText() const
{
    if (ui && ui->te_chat) {
        return ui->te_chat->toPlainText();
    }
    return QString();
}

void chatTab::setChatHtml(const QString &html)
{
    if (ui && ui->te_chat) {
        ui->te_chat->clear();
        ui->te_chat->setHtml(html);
    }
}
