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
    //setFixedSize(800, 800); // размер окна
}

MainWindow::~MainWindow()
{
    delete ui;
}

void drawPolylineWithPoints(QPainter &painter, const QVector<QPoint> &points, int offsetX, int offsetY, int scale) {
    if (points.size() < 2) {
        return; // Если точек меньше 2, рисовать ломаную не имеет смысла
    }

    // Настройка пера для рисования ломаной линии
    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(4);
    painter.setPen(pen);

    // Рисуем ломаную линию, соединяя соседние точки
    for (int i = 0; i < points.size() - 1; ++i) {
        // Преобразуем координаты точек, чтобы они учитывали сдвиг и масштаб
        QPoint p1 = QPoint(points[i].x() * scale + offsetX, offsetY - points[i].y() * scale);  // Инвертируем координату Y
        QPoint p2 = QPoint(points[i + 1].x() * scale + offsetX, offsetY - points[i + 1].y() * scale);  // Инвертируем координату Y
        painter.drawLine(p1, p2);
    }

    // Настройка пера для рисования окружностей
    pen.setColor(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::red));  // Круги будут залиты красным цветом

    // Рисуем круги в каждой точке
    int circleRadius = 5; // Радиус окружности

    for (const QPoint &point : points) {
        // Преобразуем координаты точки с учетом смещения и масштаба
        QPoint p = QPoint(point.x() * scale + offsetX, offsetY - point.y() * scale);  // Инвертируем координату Y
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

    // Вычисляем разницу по осям
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();

    // Определяем направления движения по осям
    int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

    // Абсолютные значения разностей по осям
    dx = std::abs(dx);
    dy = std::abs(dy);

    // Начальная точка
    int x = p1.x();
    int y = p1.y();

    // Добавляем первую точку в вектор
    points.append(QPoint(x, y));

    // Решение, в какую сторону идти, на основании большей разницы по осям
    if (dx > dy) {
        // Алгоритм по оси X
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
        // Алгоритм по оси Y
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

QVector<QPoint> bresenhemRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points; // Вектор точек, по которым пройдет ломаная линия

    // Вычисление разницы между точками
    int dx = std::abs(p2.x() - p1.x());
    int dy = std::abs(p2.y() - p1.y());
    int sx = (p1.x() < p2.x()) ? 1 : -1;
    int sy = (p1.y() < p2.y()) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        points.append(p1); // Добавляем текущую точку в вектор

        // Если достигли второй точки, завершаем цикл
        if (p1 == p2) break;

        // Вычисляем ошибку
        int e2 = err * 2;

        // Если ошибка больше чем ноль по оси X
        if (e2 > -dy) {
            err -= dy;
            p1.setX(p1.x() + sx); // Двигаемся по оси X
        }

        // Если ошибка меньше чем ноль по оси Y
        if (e2 < dx) {
            err += dx;
            p1.setY(p1.y() + sy); // Двигаемся по оси Y
        }
    }

    return points;
}

QVector<QPoint> ddaRasterize(QPoint p1, QPoint p2) {
    QVector<QPoint> points; // Вектор точек, представляющих растеризованный отрезок

    // Вычисляем разницу по осям
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();

    // Количество шагов (количество точек для растеризации)
    int steps = std::max(std::abs(dx), std::abs(dy));

    // Вычисляем шаги для осей X и Y
    float stepX = dx / static_cast<float>(steps);
    float stepY = dy / static_cast<float>(steps);

    // Начальная точка
    float x = p1.x();
    float y = p1.y();

    // Добавляем первую точку в вектор
    points.append(QPoint(std::round(x), std::round(y)));

    // Растеризуем отрезок
    for (int i = 1; i <= steps; ++i) {
        x += stepX; // Двигаемся по оси X
        y += stepY; // Двигаемся по оси Y

        // Добавляем округленные координаты в вектор
        points.append(QPoint(std::round(x), std::round(y)));
    }

    return points;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Настройка пера для рисования линий
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    // Оси X и Y
    painter.drawLine(width() / 2, 0, width() / 2, height());  // ось Y
    painter.drawLine(0, height() / 2, width(), height() / 2);  // ось X

    // Рисуем дополнительный сеточный линии с шагом 50 пикселей
    int step = 50;  // шаг сетки (масштаб)
    int gridSizeX = this->size().width() / step + 1;
    int gridSizeY = this->size().height() / step + 1;
    QPen grid_pen;
    grid_pen.setWidth(1);
    painter.setPen(grid_pen);
    for (int i = -gridSizeX; i <= gridSizeX; ++i) {
        // Вертикальные линии сетки
        painter.drawLine(width() / 2 + i * step, 0, width() / 2 + i * step, height());
    }
    for (int i = -gridSizeY; i <= gridSizeY; ++i) {
        // Горизонтальные линии сетки
        painter.drawLine(0, height() / 2 + i * step, width(), height() / 2 + i * step);
    }

    painter.setPen(pen);
    // Рисуем подписанные метки на осях
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    // Подписи на оси X
    for (int i = -gridSizeX; i <= gridSizeX; ++i) {
        if (i != 0) {
            painter.drawText(width() / 2 + i * step - 10, height() / 2 + 20, QString::number(i));
        }
    }

    // Подписи на оси Y
    for (int i = -gridSizeY; i <= gridSizeY; ++i) {
        if (i != 0) {
            painter.drawText(width() / 2 + 20, height() / 2 - i * step + 5, QString::number(i));
        }
    }

    int offsetX = width() / 2;
    int offsetY = height() / 2;
    int scale = 50;

    drawPolylineWithPoints(painter, current_line, offsetX, offsetY, scale);
}

void MainWindow::activate_algorithm()
{
    QPoint p1, p2;

    getTwoPointsFromUser(this, p1, p2);
    if(sender() == ui->action_step){
        current_line = stepByStepRasterize(p1, p2);
    }
    if(sender() == ui->action_bres){
        current_line = bresenhemRasterize(p1, p2);
    }
    if(sender() == ui->actionDDA){
        current_line = ddaRasterize(p1, p2);
    }
    update();
}



