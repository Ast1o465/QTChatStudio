#ifndef CHATTAB_H
#define CHATTAB_H

#include <QWidget>
#include <QString>
#include <QStringList>

class aichat;

namespace Ui {
class chatTab;
}

class chatTab : public QWidget
{
    Q_OBJECT

public:
    explicit chatTab(QWidget *parent = nullptr);
    ~chatTab();

    void setAiEngine(aichat *engine);
    void setModels(const QStringList &models);
    void setupConnections();
    
    // Add access to UI for MainWindow
    friend class MainWindow;
    
    // New method to get chat text
    QString getChatText() const;

private slots:
    void onSendButtonClicked();
    void onAiResponseReceived(const QString &response);
    void onAiError(const QString &error);

private:
    Ui::chatTab *ui;
    aichat *m_chatEngine; 

    void addMessageToChat(const QString &sender, const QString &message);
};

#endif // CHATTAB_H
