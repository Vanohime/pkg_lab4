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
    if (!ok) {
        QMessageBox::warning(parent, "Ошибка", "Не удалось получить координату X.");
        return QPoint(0, 0);
    }

    int y = QInputDialog::getInt(parent, "Ввод координаты Y", "Введите координату Y:", 0, -10000, 10000, 1, &ok);
    if (!ok) {
        QMessageBox::warning(parent, "Ошибка", "Не удалось получить координату Y.");
        return QPoint(0, 0);
    }

    return QPoint(x, y);
}

void getTwoPointsFromUser(QWidget *parent, QPoint &point1, QPoint &point2) {
    point1 = getCoordinatesFromUser(parent);
    if (point1 == QPoint(0, 0)) {
        return;
    }

    point2 = getCoordinatesFromUser(parent);
    if (point2 == QPoint(0, 0)) {
        return;
    }
}

QVector<QPoint> stepByStepRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points;


    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();


    int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

    dx = std::abs(dx);
    dy = std::abs(dy);

    int x = p1.x();
    int y = p1.y();

    points.append(QPoint(x, y));

    if (dx > dy) {

        int err = dx / 2;
        while (x != p2.x()) {
            err -= dy;
            if (err < 0) {
                y += stepY;
                err += dx;
            }
            x += stepX;
            points.append(QPoint(x, y));
        }
    } else {
        int err = dy / 2;
        while (y != p2.y()) {
            err -= dx;
            if (err < 0) {
                x += stepX;
                err += dy;
            }
            y += stepY;
            points.append(QPoint(x, y));
        }
    }

    return points;
}

QVector<QPoint> bresenhamRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points;

    int dx = std::abs(p2.x() - p1.x());
    int dy = std::abs(p2.y() - p1.y());
    int sx = (p1.x() < p2.x()) ? 1 : -1;
    int sy = (p1.y() < p2.y()) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        points.append(p1);

        if (p1 == p2) break;

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            p1.setX(p1.x() + sx);
        }
        if (e2 < dx) {
            err += dx;
            p1.setY(p1.y() + sy);
        }
    }

    return points;
}

QVector<QPoint> ddaRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points;
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();

    int steps = std::max(std::abs(dx), std::abs(dy));

    float stepX = dx / static_cast<float>(steps);
    float stepY = dy / static_cast<float>(steps);

    float x = p1.x();
    float y = p1.y();
    points.append(QPoint(std::round(x), std::round(y)));
    for (int i = 1; i <= steps; ++i) {
        x += stepX;
        y += stepY;
        points.append(QPoint(std::round(x), std::round(y)));
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

    int step = 25;
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
        if (!ok) {
            QMessageBox::warning(this, "Ошибка", "Не удалось получить радиус.");
        }
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
