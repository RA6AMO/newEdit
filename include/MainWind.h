#include <QMainWindow>
#include "ui_MainWindow.h"
#include "DBManager.h"
#include "LTreeWidget.h"
#include "memory"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(DatabaseManager *dbInit, QMainWindow *parent = nullptr);
    ~MainWindow();

private slots:
    void onTreeItemClicked(const QString &nodeId, const QString &nodeName);
    void onTreeItemDoubleClicked(const QString &nodeId, const QString &nodeName);

private:
    DatabaseManager *dbMan;
    std::unique_ptr<LTreeWidget> Ltree;

    bool tableCreater();
    bool tableFiller(const QString& tableName);
};
