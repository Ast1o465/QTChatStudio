#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void onAiResponseReceived(const QString &response);
    void onAiError(const QString &error);

private:
    Ui::MainWindow *ui;
    aichat *m_chatEngine;
    void setupConnections();
    void addMessageToChat(const QString &sender, const QString &message);
};
#endif // MAINWINDOW_H
