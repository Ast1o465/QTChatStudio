#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_requestFromMainTab(false)
    , m_currentTabIndex(0)
{
    ui->setupUi(this);
    
    m_chatEngine = new aichat(this);
    setupConnections();

    // Set default models
    QTimer::singleShot(100, this, &MainWindow::onUpdateModelsButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(m_chatEngine, &aichat::responseReady, this, &MainWindow::onAiResponseReceived);
    connect(m_chatEngine, &aichat::errorOccurred, this, &MainWindow::onAiError);
    connect(ui->btn_send, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->btn_update_models, &QPushButton::clicked, this, &MainWindow::onUpdateModelsButtonClicked);
    connect(ui->btn_delete_model, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    connect(ui->action_new_chat, &QAction::triggered, this, &MainWindow::onNewChatActionTriggered);
    connect(ui->action_save_chat, &QAction::triggered, this, &MainWindow::onSaveChatActionTriggered);
    connect(ui->action_open_file, &QAction::triggered, this, &MainWindow::onOpenChatActionTriggered);
    
    // Send message on Enter key press
    connect(ui->le_input, &QLineEdit::returnPressed, this, &MainWindow::onSendButtonClicked);
}

void MainWindow::onTabChanged(int index)
{
    m_currentTabIndex = index;
    qDebug() << "Active tab changed to:" << index;
}

void MainWindow::onSendButtonClicked()
{
    QString userMessage = ui->le_input->text().trimmed();
    if (userMessage.isEmpty()) return;
    
    // Set flag that the request is from the main tab
    m_requestFromMainTab = true;
    
    addMessageToChat("You", userMessage);
    ui->le_input->clear();
    
    QString selectedModel = ui->cb_models->currentText();
    m_chatEngine->sendMessage(userMessage, selectedModel);
    
    ui->btn_send->setEnabled(false);
    ui->btn_send->setText("Sending...");
}

void MainWindow::onUpdateModelsButtonClicked()
{
    // Run ollama list command
    QProcess process;
    process.start("ollama", QStringList() << "list");
    
    if (!process.waitForFinished()) {
        qDebug() << "Error running Ollama";
        return;
    }
    
    // Get output and log available models
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    // Prepare model names list
    QStringList modelNames;
    
    // Process each line from output
    for (const QString& line : lines) {
        qDebug() << "Ollama update";
        // qDebug() << "Ollama models:" << line;


        if (line.contains(':')) {
            QString modelName = line.split(':').first().trimmed();
            modelNames << modelName;
        }
    }
    
    // Clear existing models in UI
    ui->lw_models->clear();
    ui->cb_models->clear();
    ui->cb_available_models->clear();

    // Add new models to UI
    ui->lw_models->addItems(lines);
    ui->cb_models->addItems(modelNames);
    ui->cb_available_models->addItems(modelNames);
    
    // Update models in all chat tabs
    for (int i = 1; i < ui->tabWidget->count(); i++) {
        chatTab *tab = qobject_cast<chatTab*>(ui->tabWidget->widget(i));
        if (tab) {
            tab->setModels(modelNames);
        }
    }
}

void MainWindow::onDeleteButtonClicked()
{
    // Get the selected model from the combobox
    QString modelName = ui->cb_available_models->currentText();
    
    if (modelName.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No model selected");
        return;
    }
    
    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Delete Model", 
        QString("Are you sure you want to delete model '%1'?").arg(modelName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Run the deletion command
    QProcess process;
    process.start("ollama", QStringList() << "rm" << modelName);
    
    if (!process.waitForFinished()) {
        QMessageBox::critical(this, "Error", "Error running Ollama");
        return;
    }
    
    // Update the models list after deletion
    QTimer::singleShot(100, this, &MainWindow::onUpdateModelsButtonClicked);
    
    QMessageBox::information(this, "Success", QString("Model '%1' has been deleted").arg(modelName));
}

void MainWindow::onAiResponseReceived(const QString &response)
{
    // Check if the request was sent from the main tab and if it's active
    if (m_requestFromMainTab && m_currentTabIndex == 0) {
        addMessageToChat("AI", response);
        ui->btn_send->setEnabled(true);
        ui->btn_send->setText("Send");
    }
    
    // Reset the flag after processing the response
    m_requestFromMainTab = false;
}

void MainWindow::onAiError(const QString &error)
{
    // Check if the request was sent from the main tab and if it's active
    if (m_requestFromMainTab && m_currentTabIndex == 0) {
        addMessageToChat("System", QString("Error: %1").arg(error));
        ui->btn_send->setEnabled(true);
        ui->btn_send->setText("Send");
    }
    
    // Reset the flag after processing the error
    m_requestFromMainTab = false;
}

void MainWindow::onSaveChatActionTriggered()
{
    qDebug() << "Saving chat...";
    
    // Create directory for chat history
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir + "/chat_history");
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "Created directory:" << dir.absolutePath();
    }
    
    // Generate unique filename with timestamp
    QString filename = dir.absolutePath() + "/chat_" + 
                      QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".txt";
    
    // Get text from active tab
    QString chatText;
    if (m_currentTabIndex == 0) {
        // Get text from main tab
        chatText = ui->te_chat ? ui->te_chat->toPlainText() : "";
    } else {
        // Get text from custom chat tab
        chatTab *activeTab = qobject_cast<chatTab*>(ui->tabWidget->currentWidget());
        chatText = activeTab ? activeTab->getChatText() : "";
    }
    
    // Check if we have content to save
    if (chatText.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No chat content to save");
        return;
    }
    
    // Save chat to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << chatText;
        file.close();
        QMessageBox::information(this, "Success", QString("Chat saved to %1").arg(filename));
    } else {
        QMessageBox::critical(this, "Error", 
                            QString("Could not save chat: %1").arg(file.errorString()));
    }
}

void MainWindow::onNewChatActionTriggered()
{
    int tabCount = ui->tabWidget->count();
    QString tabName = QString("Chat %1").arg(tabCount + 1);
    
    // Create a new chatTab widget
    chatTab *newChatTab = new chatTab(this);
    
    // Pass the chat engine and set up models
    newChatTab->setAiEngine(m_chatEngine);
    
    // Get the list of models from the main combobox
    QStringList models;
    for(int i = 0; i < ui->cb_models->count(); i++) {
        models << ui->cb_models->itemText(i);
    }
    newChatTab->setModels(models);
    
    // Set up connections
    newChatTab->setupConnections();
    
    qDebug() << "Creating new chat tab using chatTab widget:" << tabName;

    // Add the new tab with the chatTab widget
    ui->tabWidget->addTab(newChatTab, tabName);
    ui->tabWidget->setCurrentIndex(tabCount);   

    QMessageBox::information(this, "New Chat", 
                            QString("Creating new chat tab: %1").arg(tabName));
}

void MainWindow::onOpenChatActionTriggered()
{
    // Set up history directory
    QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history";
    QDir().mkpath(historyDir); // Create directory if it doesn't exist
    
    // Get file from user
    QString filePath = QFileDialog::getOpenFileName(this, "Open Chat History", 
                                                  historyDir, "Text Files (*.txt)");
    if (filePath.isEmpty()) return;
    
    // Read file content
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file: " + file.errorString());
        return;
    }
    
    QString chatContent = QTextStream(&file).readAll();
    file.close();
    
    // Display content in appropriate tab
    if (m_currentTabIndex == 0) {
        // Update main tab
        ui->te_chat->clear();
        ui->te_chat->setHtml(chatContent);
    } else {
        // Create new tab with content
        chatTab *newTab = new chatTab(this);
        newTab->setAiEngine(m_chatEngine);
        
        // Configure new tab
        QStringList models;
        for(int i = 0; i < ui->cb_models->count(); i++) {
            models << ui->cb_models->itemText(i);
        }
        newTab->setModels(models);
        newTab->setupConnections();
        
        // Add and switch to new tab
        QString fileName = QFileInfo(filePath).fileName();
        ui->tabWidget->addTab(newTab, 
                            QString("Chat %1 (%2)").arg(ui->tabWidget->count() + 1).arg(fileName));
        newTab->setChatHtml(chatContent);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    }
}

void MainWindow::addMessageToChat(const QString &sender, const QString &message)
{
    QString formattedMessage = QString("<b>%1:</b> %2<br>").arg(sender, message);
    ui->te_chat->append(formattedMessage);
}
