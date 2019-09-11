#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <stack>
#include <QOpenGLWidget>
#include <math.h>
#include <qlabel.h>
#include <qpainter.h>

#define TOP(p) p.first, p.second-1
#define RIGHT(p) p.first+1, p.second
#define BOTTOM(p) p.first, p.second+1
#define LEFT(p) p.first-1, p.second

#define RED qRgb(255, 0, 0)
#define GREEN qRgb(0, 255, 0)
#define BLUE qRgb(13, 36, 129)
#define BLACK qRgb(0, 0, 0)
#define WHITE qRgb(255, 255, 255)
#define YELLOW qRgb(255,255,0)
#define PURPLE qRgb(77, 0, 102)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->showMaximized();


}

MainWindow::~MainWindow()
{
    delete ui;
}

pair<int, int> getTop(pair<int, int> p) {
    return make_pair(p.first, p.second-1);
}
pair<int, int> getRight(pair<int, int> p) {
    return make_pair(p.first+1, p.second);
}
pair<int, int> getBottom(pair<int, int> p) {
    return make_pair(p.first, p.second+1);
}
pair<int, int> getLeft(pair<int, int> p) {
    return make_pair(p.first-1, p.second);
}
pair<int, int> getDirection(pair<int, int> p, int x, int y) {
    return make_pair(p.first+x, p.second+y);
}

pair<int, int> midTop(QImage image, QRgb color) {
    pair<int, int> xy = make_pair(-1, -1);
    for(int y = 0; y < image.height(); ++y)
        for(int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y).rgb() == QRgb(color)) {
                int x_left = x;
                int x_right = x;
                while(image.pixelColor(x_right, y).rgb() == QRgb(color))
                    x_right++;
                x_right--;
                xy.first = (x_left + x_right) / 2;
                xy.second = y;
                return xy;
            }
    return xy;
}

pair<int, int> midBot(QImage image, QRgb color) {
    pair<int, int> xy = make_pair(-1, -1);
    for(int y = image.height()-1; y > 0; --y)
        for(int x = image.width()-1; x > 0; --x)
            if (image.pixelColor(x, y).rgb() == QRgb(color)) {
                int x_left = x;
                int x_right = x;
                while(image.pixelColor(x_left, y).rgb() == QRgb(color))
                    x_left--;
                x_left++;
                xy.first = (x_left + x_right) / 2;
                xy.second = y+1;
                return xy;
            }
    return xy;
}

pair<int, int> midLeft(QImage image, QRgb color) {
    pair<int, int> xy = make_pair(-1, -1);
    for(int x = 0; x < image.width(); ++x)
        for(int y = 0; y < image.height(); ++y)
            if (image.pixelColor(x, y).rgb() == QRgb(color)) {
                int y_up = y;
                int y_down = y;
                while(image.pixelColor(x, y_down).rgb() == QRgb(color))
                    y_down++;
                y_down--;
                xy.first = x;
                xy.second = (y_down + y_up) / 2;
                return xy;
            }
    return xy;
}

pair<int, int> midRight(QImage image, QRgb color) {
    pair<int, int> xy = make_pair(-1, -1);
    for(int x = image.width()-1; x > 0; --x)
        for(int y = image.height()-1; y > 0; --y)
            if (image.pixelColor(x, y).rgb() == QRgb(color)) {
                int y_up = y;
                int y_down = y;
                while(image.pixelColor(x, y_up).rgb() == QRgb(color))
                    y_up--;
                y_up++;
                xy.first = x+1;
                xy.second = (y_down + y_up) / 2;
                return xy;
            }
    return xy;
}

int radiusLen(pair<int, int> point, pair<int, int> center) {
    int maximum = 0;
    int minimum = 0;
    if (point.first == center.first) {
        maximum = max(point.second, center.second);
        minimum = min(point.second, center.second);
    }
    if (point.second == center.second) {
        maximum = max(point.first, center.first);
        minimum = min(point.first, center.first);
    }
    return maximum - minimum;
}

void drawLine(QImage &image, pair<int, int> a, pair<int, int> b, QRgb color) {
    int maximum, minimum;
    if (a.first == b.first) {
        maximum = max(a.second, b.second);
        minimum = min(a.second, b.second);
        for(int y = minimum; y < maximum; ++y)
            image.setPixel(a.first, y, color);
    } else if (a.second == b.second) {
        maximum = max(a.first, b.first);
        minimum = min(a.first, b.first);
        for(int x = minimum; x < maximum; ++x)
            image.setPixel(x, a.second, color);
    }
}

pair<int, int> searchColor(QImage image, QRgb color) {
    for(int y = 0; y < image.height(); ++y)
        for(int x = 0; x < image.width(); ++x)
            if (QRgb(image.pixelColor(x, y).rgb()) == QRgb(color))
                return make_pair(x, y);
    return make_pair(-1, -1);
}

bool edge(QImage image, pair<int, int> p) {
    return (!image.valid(TOP(p)) || !image.valid(RIGHT(p)) || !image.valid(BOTTOM(p)) || !image.valid(LEFT(p)));
}
void deleteFigure(QImage &image, QRgb color, pair<int, int> s, QRgb(deletionColor)) {
    stack<pair<int, int>> queue;
    queue.push(s);

    while(!queue.empty()){
        s = queue.top();
        image.setPixel(s.first, s.second, deletionColor);
        queue.pop();
        if (image.pixelColor(getTop(s).first, getTop(s).second).rgb() == QRgb(color)) {
            queue.push(getTop(s));
        }
        if (image.pixelColor(getRight(s).first, getRight(s).second).rgb() == QRgb(color)) {
            queue.push(getRight(s));
        }
        if (image.pixelColor(getBottom(s).first, getBottom(s).second).rgb() == QRgb(color)) {
            queue.push(getBottom(s));
        }
        if (image.pixelColor(getLeft(s).first, getLeft(s).second).rgb() == QRgb(color)) {
            queue.push(getLeft(s));
        }
    }
}

int separator(QImage &image, QRgb color, pair<int, int> s, int colorChange, int circles, bool completeCircle) {
    if (s.first == -1 && s.second == -1) {
        return circles;
    }

    pair<int, int> start = s;
    stack<pair<int, int>> queue;
    queue.push(s);

    while(!queue.empty()){
        s = queue.top();
        if (edge(image, s) && completeCircle)
            return separator(image, BLUE, start, colorChange-2, circles-1, false);

        image.setPixel(s.first, s.second, color);
        queue.pop();

        if (image.pixelColor(getTop(s).first, getTop(s).second).black() == 255) {
            queue.push(getTop(s));
        }
        if (image.pixelColor(getRight(s).first, getRight(s).second).black() == 255) {
            queue.push(getRight(s));
        }
        if (image.pixelColor(getBottom(s).first, getBottom(s).second).black() == 255) {
            queue.push(getBottom(s));
        }
        if (image.pixelColor(getLeft(s).first, getLeft(s).second).black() == 255) {
            queue.push(getLeft(s));
        }
   }

    return separator(image, qRgb(colorChange+2, colorChange+2, colorChange+2), searchColor(image, BLACK), colorChange+2, circles+1, true);
}

bool isNoise(QColor color) {
    return (((color.red() == color.green() && (color.green() == color.blue())))
            && (color.red() > 0 && color.red() < 255)
            && (color.green() > 0 && color.green() < 255)
            && (color.blue() > 0 && color.blue() < 255));
}
void clean(QImage &image) {
    for(int y = 0; y < image.height(); ++y)
        for(int x = 0; x < image.width(); ++x)
            if (isNoise(image.pixelColor(x, y)))
                image.setPixel(x, y, BLACK);
}

void drawBox(QImage &image, pair<int, int> top, pair<int, int> right, pair<int, int> down, pair<int, int> left, int id) {
    pair<int, int> topLeft, topRight, bottomLeft,bottomRight;
    topLeft = make_pair(left.first-5, top.second-5);
    topRight = make_pair(right.first+5, top.second-5);
    bottomLeft = make_pair(left.first-5, down.second+5);
    bottomRight = make_pair(right.first+5, down.second+5);

    drawLine(image, topLeft, topRight, GREEN);
    drawLine(image, topRight, bottomRight, GREEN);
    drawLine(image, bottomRight, bottomLeft, GREEN);
    drawLine(image, bottomLeft, topLeft, GREEN);

    QPainter p(&image);
    p.setPen(QPen(Qt::red));
    p.setFont(QFont("Times", 16, QFont::Bold));
    p.drawText(QPointF(topLeft.first, topRight.second+5), QString::fromStdString(to_string(id)));
    p.end();
}

int sign(double x) {
    if (x < 0) {
        return -1;
    } else {
        return 1;
    }
}
void line(QImage &image, QRgb color, int x0, int y0, int x1, int y1) {
   double dx =  abs(x1-x0);
   double sx = x0<x1 ? 1 : -1;
   double dy = -abs(y1-y0);
   double sy = y0<y1 ? 1 : -1;
   double err = dx+dy;
   while (true) {
       if (x0==x1 && y0==y1) break;
       image.setPixel(x0, y0, color);
       double e2 = 2*err;
       if (e2 >= dy)  {
           err += dy;
           x0 += sx;
       }
       if (e2 <= dx) {
           err += dx;
           y0 += sy;
       }
   }
}

void MainWindow::on_openFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/home/uriel/Desktop/Seminario de algoritmia/etapa1/etapa1", tr("Image Files (*.png)"));
    QImage image = QImage(fileName);
    QImage copy = image;
    QGraphicsScene *graphic = new QGraphicsScene(this);
    graphic->addPixmap(QPixmap::fromImage(image));
    ui->graphicsViewOriginal->setScene(graphic);

    clean(copy);
    int circles = separator(copy, qRgb(2, 2, 2), searchColor(copy, BLACK), 2, 0, true);

    QRgb color;
    model = new QStandardItemModel(circles, 6, this);
    vector<vector<pair<int, int>>> labels;
    for(int i = 1; i <= circles; ++i) {
        color = qRgb(2*i, 2*i, 2*i);
        // Puntos fundamentales
        pair<int, int> top = midTop(copy, color);
        pair<int, int> bot = midBot(copy, color);
        pair<int, int> right = midRight(copy, color);
        pair<int, int> left = midLeft(copy, color);

        // Ajustar diferencias
        top.first = max(top.first, bot.first);
        bot.first = top.first;
        left.second = max(left.second, right.second);
        right.second = left.second;

        pair<int, int> center = make_pair(top.first, left.second);

        // Radio
        int topRadio = radiusLen(top, center);
        int rightRadio = radiusLen(right, center);
        int botRadio = radiusLen(bot, center);
        int leftRadio = radiusLen(left, center);

        // Diferencia entre radios

        int yLen = topRadio + botRadio;
        int xLen = leftRadio + rightRadio;
        int diff = abs(xLen - yLen);

        // Eliminar donas
        if (image.pixelColor(center.first, center.second).rgb() == QRgb(WHITE)) {
            deleteFigure(copy, color, searchColor(copy, color), BLUE);
            i--;
            continue;
        }
        // Eliminar óvalos grandes
        else if (diff > 10) {
            deleteFigure(copy, color, searchColor(copy, color), WHITE);
            i--;
            continue;
        }
        // Entonces es círculo
        if (diff <= 10 && image.pixelColor(center.first, center.second).rgb() != QRgb(WHITE)){
            //Dibujar radio
            drawLine(copy, top, center, YELLOW);
            drawLine(copy, right, center, YELLOW);
            drawLine(copy, bot, center, YELLOW);
            drawLine(copy, left, center, YELLOW);


            // Dibujar Centroide
            QRgb centroidColor = RED;
            copy.setPixel(center.first, center.second, centroidColor);
            copy.setPixel(TOP(center), centroidColor);
            copy.setPixel(RIGHT(center), centroidColor);
            copy.setPixel(BOTTOM(center), centroidColor);
            copy.setPixel(LEFT(center), centroidColor);
            copy.setPixel(TOP(make_pair(center.first, center.second-1)), centroidColor);
            copy.setPixel(RIGHT(make_pair(center.first+1, center.second)), centroidColor);
            copy.setPixel(BOTTOM(make_pair(center.first, center.second+1)), centroidColor);
            copy.setPixel(LEFT(make_pair(center.first-1, center.second)), centroidColor);

            QStringList coordinates;
            coordinates << QString::fromStdString(to_string(top.first)) + ", " + QString::fromStdString(to_string(top.second));
            coordinates << QString::fromStdString(to_string(right.first)) + ", " + QString::fromStdString(to_string(right.second));
            coordinates << QString::fromStdString(to_string(bot.first)) + ", " + QString::fromStdString(to_string(bot.second));
            coordinates << QString::fromStdString(to_string(left.first)) + ", " + QString::fromStdString(to_string(left.second));
            coordinates << QString::fromStdString(to_string(center.first)) + ", " + QString::fromStdString(to_string(center.second));

            if (top.first != -1) {
                vector<pair<int, int>> v;
                v.push_back(top);
                v.push_back(right);
                v.push_back(bot);
                v.push_back(left);
                v.push_back(center);
                v.push_back(make_pair((topRadio+leftRadio)/2, 0));
                labels.push_back(v);
            }
    }
}
    QModelIndex index;
    for(int i = 0; i < labels.size(); ++i) {
        for(int j = 0; j < 6; ++j) {
            ui->tableView->setModel(model);
            index = model->index(i, j, QModelIndex());
            QStringList coordinates;
            coordinates << "Arriba \n" + QString::fromStdString(to_string(labels[i][0].first)) + ", " + QString::fromStdString(to_string(labels[i][0].second));
            coordinates << "Derecha \n" + QString::fromStdString(to_string(labels[i][1].first)) + ", " + QString::fromStdString(to_string(labels[i][1].second));
            coordinates << "Abajo \n" + QString::fromStdString(to_string(labels[i][2].first)) + ", " + QString::fromStdString(to_string(labels[i][2].second));
            coordinates << "Izquierda \n" + QString::fromStdString(to_string(labels[i][3].first)) + ", " + QString::fromStdString(to_string(labels[i][3].second));
            coordinates << "Centro \n" + QString::fromStdString(to_string(labels[i][4].first)) + ", " + QString::fromStdString(to_string(labels[i][4].second));
            coordinates << "Radio \n" + QString::fromStdString(to_string(labels[i][5].first));
            model->setData(index, coordinates[j]);
            // Caja verde
            drawBox(copy, labels[i][0], labels[i][1], labels[i][2], labels[i][3], i+1);
        }
    }
    int sz = labels.size();
    vector<vector<int>> graph;
    for(int i = 0; i < sz; ++i) {
        vector<int> neighbors;
        for(int j = 0; j < sz; ++j) {
            if (i != j) {
                neighbors.push_back(j);
            }
        }
        graph.push_back(neighbors);
    }
    int x0, x1, y0, y1;
    vector<bool> vis(sz);
    for(int node = 0; node < sz; ++node) {
        cout<<labels[node][4].first<<" "<<labels[node][4].second<<endl;
        for(auto neighbor : graph[node]) {
            cout<<labels[neighbor][4].first<<","<<labels[neighbor][4].second<<" ";
            if (!vis[neighbor]) {
                x0 = labels[node][4].first;
                y0 = labels[node][4].second;
                x1 = labels[neighbor][4].first;
                y1 = labels[neighbor][4].second;
                line(copy, PURPLE, x0, y0, x1, y1);
            }
        }
        cout<<endl;
    }

    graphic = new QGraphicsScene(this);
    graphic->addPixmap(QPixmap::fromImage(copy));
    ui->graphicsViewResult->setScene(graphic);
}




