#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <iostream>
#include <qstandarditemmodel.h>
#include <set>
#include <QImage>
#include <graph.h>
#include <qpainter.h>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_openFile_clicked();
    void mousePressEvent(QMouseEvent *event);
    void setTable();
    void on_order_clicked();
    void on_pushButton_clicked();
    void drawImage(QImage &image, QImage icon, int x, int y);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *model;
    QStandardItemModel *model2;
    QString fileName;
    bool openFile = true;
    bool ordered = false;
    set<int> trash;
    int deletex;
    int deletey;
    Graph g;
    int agentId;
    QImage copy;
    QGraphicsScene *graphic;
    int numAgents;
    bool running;
    set<int> taken;
    int iconWidth = 50, iconHeight = 50;
    QImage agentIcon = QImage("agent.png");
    QImage lureIcon = QImage("lure.png");
};

#endif // MAINWINDOW_H
