#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QPoint>
#include <cmath>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Алгоритмы растеризации");
    current_line = QVector<QPoint>();
    connect(ui->actionDDA, SIGNAL(triggered(bool)), this, SLOT(activate_algorithm()));
    connect(ui->action_bres, SIGNAL(triggered(bool)), this, SLOT(activate_algorithm()));
    connect(ui->action_circle, SIGNAL(triggered(bool)), this, SLOT(activate_algorithm()));
    connect(ui->action_step, SIGNAL(triggered(bool)), this, SLOT(activate_algorithm()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void drawPolylineWithPoints(QPainter &painter, const QVector<QPoint> &points, int offsetX, int offsetY, int scale) {
    if (points.size() < 2) {
        return;
    }
    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(4);
    painter.setPen(pen);

    for (int i = 0; i < points.size() - 1; ++i) {
        QPoint p1 = QPoint(points[i].x() * scale + offsetX, offsetY - points[i].y() * scale);
        QPoint p2 = QPoint(points[i + 1].x() * scale + offsetX, offsetY - points[i + 1].y() * scale);
        painter.drawLine(p1, p2);
    }

    pen.setColor(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::red));


    int circleRadius = 5;

    for (const QPoint &point : points) {

        QPoint p = QPoint(point.x() * scale + offsetX, offsetY - point.y() * scale);
        painter.drawEllipse(p.x() - circleRadius, p.y() - circleRadius, 2 * circleRadius, 2 * circleRadius);
    }
}

QPoint getCoordinatesFromUser(QWidget *parent) {
    bool ok;
    int x = QInputDialog::getInt(parent, "Ввод координаты X", "Введите координату X:", 0, -10000, 10000, 1, &ok);

    int y = QInputDialog::getInt(parent, "Ввод координаты Y", "Введите координату Y:", 0, -10000, 10000, 1, &ok);

    return QPoint(x, y);
}

void getTwoPointsFromUser(QWidget *parent, QPoint &point1, QPoint &point2) {
    point1 = getCoordinatesFromUser(parent);

    point2 = getCoordinatesFromUser(parent);

}

QVector<QPoint> stepByStepRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points;

    int dx = std::abs(p2.x() - p1.x());
    int dy = std::abs(p2.y() - p1.y());
    int sx = (p1.x() < p2.x()) ? 1 : -1;
    int sy = (p1.y() < p2.y()) ? 1 : -1;
    while (true) {
        points.append(p1);
        if (p1 == p2)
            break;
        if(p1.x() != p2.x())
            p1.setX(p1.x() + sx);
        if (p1.y() != p2.y()) {
            p1.setY(p1.y() + sy);
        }
    }

    return points;
}

QVector<QPoint> bresenhamRasterize(QPoint p1, QPoint p2){
    int x0 = p1.x();
    int y0 = p1.y();
    int x1 = p2.x();
    int y1 = p2.y();
    QVector<QPoint> points;
    int deltax = qAbs(x1 - x0);
    int deltay = qAbs(y1 - y0);
    int error = 0;
    int deltaerr = deltay + 1;
    int x = x0;
    int y = y0;
    int dirx = x0 < x1 ? 1 : -1;
    int diry = (y1 - y0) > 0 ? 1 : (y1 - y0) < 0 ? -1 : 0;

    if (deltax >= deltay) {
        deltaerr = deltay+ 1;
        for (x = x0; x != x1 + dirx; x += dirx) {
            points.append(QPoint(x, y));
            error += deltaerr;
            if (error >= deltax + 1) {
                y += diry;
                error -= deltax + 1;
            }
        }
    } else {
        deltaerr = deltax +  1;
        for (y = y0; y != y1 + diry; y += diry) {
            points.append(QPoint(x, y));
            error += deltaerr;
            if (error >= deltay + 1 ) {
                x += dirx;
                error -= deltay + 1;
            }
        }
    }

    return points;
}
QVector<QPoint> ddaRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points;
    int x1 = p1.x(), x2 = p2.x(), y1 = p1.y(), y2 = p2.y();
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));

    double xInc = dx / (double)steps;
    double yInc = dy / (double)steps;
    double x = x1;
    double y = y1;

    for (int i = 0; i <= steps; ++i) {
        points.push_back(QPoint(std::round(x), std::round(y)));
        x += xInc;
        y += yInc;
    }
    return points;
}

QVector<QPoint> bresenhamCircle(const QPoint& center, int radius) {
    QVector<QPoint> points;
    int x = 0;
    int y = radius;
    int delta = 2 - 2 * radius;
    while (y >= 0) {
        points.append(QPoint(center.x() + x, center.y() + y));
        points.append(QPoint(center.x() + y, center.y() + x));
        points.append(QPoint(center.x() - y, center.y() + x));
        points.append(QPoint(center.x() - x, center.y() + y));
        points.append(QPoint(center.x() - x, center.y() - y));
        points.append(QPoint(center.x() - y, center.y() - x));
        points.append(QPoint(center.x() + y, center.y() - x));
        points.append(QPoint(center.x() + x, center.y() - y));

        int delta_minus = 2 * delta + 2  * y - 1;
        int delta_plus = 2 * delta - 2 * x - 1;
        if(delta < 0 && delta_minus <= 0) {
            x++;
            delta = delta + 2 * x + 1;
        }
        else if(delta > 0 && delta_plus > 0) {
            y--;
            delta = delta - 2 * y + 1;
        }
        else {
            x++;
            y--;
            delta = delta + 2*(x - y) + 2;
        }

    }

    std::sort(points.begin(), points.end(), [&center](const QPoint& a, const QPoint& b) {
        double angleA = std::atan2(a.y() - center.y(), a.x() - center.x());
        double angleB = std::atan2(b.y() - center.y(), b.x() - center.x());
        return angleA < angleB;
    });


    points.push_back(points[0]);
    return points;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    painter.drawLine(width() / 2, 0, width() / 2, height());  // ось Y
    painter.drawLine(0, height() / 2, width(), height() / 2);  // ось X

    int step = 30;
    int gridSizeX = this->size().width() / step + 1;
    int gridSizeY = this->size().height() / step + 1;
    QPen grid_pen;
    grid_pen.setWidth(0);
    painter.setPen(grid_pen);
    for (int i = -gridSizeX; i <= gridSizeX; ++i) {
        painter.drawLine(width() / 2 + i * step, 0, width() / 2 + i * step, height());
    }
    for (int i = -gridSizeY; i <= gridSizeY; ++i) {
        painter.drawLine(0, height() / 2 + i * step, width(), height() / 2 + i * step);
    }

    painter.setPen(pen);
    QFont font = painter.font();
    font.setPointSize(step/5);
    painter.setFont(font);

    for (int i = -gridSizeX; i <= gridSizeX; ++i) {
        if (i != 0) {
            painter.drawText(width() / 2 + i * step - 10, height() / 2 + 20, QString::number(i));
        }
    }

    for (int i = -gridSizeY; i <= gridSizeY; ++i) {
        if (i != 0) {
            painter.drawText(width() / 2 + 20, height() / 2 - i * step + 5, QString::number(i));
        }
    }

    int offsetX = width() / 2;
    int offsetY = height() / 2;
    int scale = step;

    drawPolylineWithPoints(painter, current_line, offsetX, offsetY, scale);
}

void MainWindow::activate_algorithm()
{
    QPoint p1, p2;
    if(sender() == ui->action_circle) {
        p1 = getCoordinatesFromUser(this);
        bool ok;
        int x = QInputDialog::getInt(this, "Ввод радиуса", "Введите радиус:", 0, -10000, 10000, 1, &ok);
        current_line = bresenhamCircle(p1, x);
    }
    else {
        getTwoPointsFromUser(this, p1, p2);
        if(sender() == ui->action_step){
            current_line = stepByStepRasterize(p1, p2);
        }
        if(sender() == ui->action_bres){
            current_line = bresenhamRasterize(p1, p2);
        }
        if(sender() == ui->actionDDA){
            current_line = ddaRasterize(p1, p2);
        }
    }
    update();
}
