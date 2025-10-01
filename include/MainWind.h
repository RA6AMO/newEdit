#include <QMainWindow>
#include "ui_MainWindow.h"
#include "DBManager.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(DatabaseManager *dbInit, QWidget *parent = nullptr);
    ~MainWindow();

private:
    DatabaseManager *dbMan;
};
