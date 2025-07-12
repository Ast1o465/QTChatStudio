#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include "chattab.h"
#include "aichat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendButtonClicked();
    void onUpdateModelsButtonClicked();

    void onSaveChatActionTriggered();
    void onNewChatActionTriggered();
    void onOpenChatActionTriggered();
    
    void onAiResponseReceived(const QString &response);
    void onAiError(const QString &error);
    void onTabChanged(int index); // Track active tab

private:
    Ui::MainWindow *ui;
    aichat *m_chatEngine;
    void setupConnections();
    void addMessageToChat(const QString &sender, const QString &message);
    
    // Flag indicating that the request was sent from the main tab
    bool m_requestFromMainTab;
    // Current tab index
    int m_currentTabIndex;
};
#endif // MAINWINDOW_H
