#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QPainter>
#include <unistd.h>
#include <QTimer>
#include "QFileDialog"
#include "fstream"
#include "cmath"
#include "algorithm"

int speed = 1;
int W = 1, H = 1, mx_x = 0, mx_y = 0, R = 5;
int timer_interval = 1000;
QWidget *par;
QString fileName;
std::ifstream fin;
int N;
int N_player = 1;
int pl_now = 1;
bool skip = true;
bool game_not_stopped = true;
QVector<std::pair<int, int>> planets_coord;
QVector<std::tuple<int, int, int, int>> planet_state;
QVector<std::tuple<int, int, int, int, int>> groups_ship;
QVector<QVector<int>> distance;
QColor colors[] {Qt::lightGray, Qt::red, Qt::blue, Qt::green, Qt::magenta};
QBrush planet_br = Qt::DiagCrossPattern;
QBrush ship_br = Qt::SolidPattern;

std::pair<int, int> realcoord(int x, int y)
{
    int n_x = x * (W - 6 * R) / mx_x + 3 * R + 10;
    int n_y = y * (H - 6 * R - 30) / mx_y + 3 * R + 40;
    return {n_x, n_y};
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_2->setText("3");
    par = parent;
    int w, h;
    std::ifstream conf("viz.conf");
    conf >> w >> h >> R;
    this->resize(w, h);
    conf.close();
    W = this->size().width() - 20;
    H = this->size().height() - 40;
    // ui->label_2->setText(QString::number(H));
    fileName = QFileDialog::getOpenFileName(this);
    QFile tmp(fileName);
    if (!QFileInfo::exists(fileName)) return;
    try {
        fin.open(fileName.toStdString(), std::ios_base::in);
        fin >> N >> N_player;
        planets_coord.resize(N);
        planet_state.resize(N);
        distance.resize(N);
        for (int i = 0; i < N; ++i)
        {
            int x, y;
            fin >> x >> y;
            if (mx_x < x) mx_x = x;
            if (mx_y < y) mx_y = y;
            planets_coord[i] = {x, y};
        }
        for (int i = 0; i < N; ++i)
        {
            distance[i].resize(N);
            for (int j = 0; j < N; ++j)
            {
                int x = planets_coord[i].first - planets_coord[j].first;
                int y = planets_coord[i].second - planets_coord[j].second;
                distance[i][j] = std::round(std::sqrt(x * x + y * y));
            }
        }
        for (int i = 0; i < N; ++i)
        {
            planets_coord[i] = realcoord(planets_coord[i].first, planets_coord[i].second);
        }
        tmr = new QTimer();
        tmr->setInterval(timer_interval);
        connect(tmr, SIGNAL(timeout()), this, SLOT(updateTime()));
        tmr->start();
    } catch (...) {
        return;
    }

}

std::pair<int, int> coord_move(int from, int to, int time)
{
    auto [x1, y1] = planets_coord[from];
    auto [x2, y2] = planets_coord[to];
    int x = x1 + (x2 - x1) * time / distance[from][to];
    int y = y1 + (y2 - y1) * time / distance[from][to];
    return {x, y};
}

void MainWindow::paintEvent(QPaintEvent *event)
{

    QPainter canv(this);
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(R / 3);
    canv.setPen(pen);
    int PlayerId, ShipCount, Level, Armor, x, y, r;
    for (int i = 0; i < N; ++i)
    {
        std::tie(x, y) = planets_coord[i];
        std::tie(PlayerId, ShipCount, Level, Armor) = planet_state[i];
        r = (R * Level) + R * 2 / 3;
        QRadialGradient gradient(x, y, r, x, y);
        QColor c = colors[PlayerId];
        if (Level == 0)
        {
            c.setAlphaF(0.6);
        }
        else
        {
            c.setAlphaF(std::max(0.2, std::min(1.0, ShipCount / 100.0)));
        }
        gradient.setColorAt(0, c);
        gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
        canv.setBrush(QBrush(gradient));
        canv.drawEllipse(QRect(x - r, y - r, 2 * r, 2 * r));
        canv.drawText(x - r * 2 - 8, y + r, r * 4 + 16, 16, Qt::AlignCenter, QString::number(ShipCount));
    }
    r = R / 3;
    for (auto [FromPlanetId, ToPlanetId, Count, time, PlayerId]: groups_ship)
    {
        pen.setColor(colors[PlayerId]);
        canv.setPen(pen);
        std::tie(x, y) = coord_move(FromPlanetId, ToPlanetId, time);
        canv.drawText(x - r * 2 - 8, y - 2 * R, r * 4 + 16, 16, Qt::AlignCenter, QString::number(Count));
        canv.drawEllipse(QRect(x - r, y - r, 2 * r, 2 * r));
    }
}

bool read_next_event()
{
    int m;
    fin >> m;
    if (m == -1)
    {
        game_not_stopped = false;
        return false;
    }
    int FromPlanetId, ToPlanetId, Count, PlayerId;
    for (int i = 0; i < m; ++i)
    {
        fin >> PlayerId >> FromPlanetId >> ToPlanetId >> Count;
        groups_ship.push_back({--FromPlanetId, --ToPlanetId, Count, 0, PlayerId});
    }
    int ShipCount, Level, Armor;
    bool changed = false;
    for (int i = 0; i < N; ++i)
    {
        fin >> PlayerId >> ShipCount >> Level >> Armor;
        changed |= (Level != std::get<2>(planet_state[i]));
        planet_state[i] = {PlayerId, ShipCount, Level, Armor};
    }
    return changed || !skip;
}

void move_ships()
{
    QVector<std::tuple<int, int, int, int, int>> groups_ship_new;
    for (auto [FromPlanetId, ToPlanetId, Count, time, PlayerId]: groups_ship)
    {
        if (PlayerId != pl_now)
        {
            groups_ship_new.push_back({FromPlanetId, ToPlanetId, Count, time, PlayerId});
            continue;
        }
        if (++time < distance[FromPlanetId][ToPlanetId])
        {
            groups_ship_new.push_back({FromPlanetId, ToPlanetId, Count, time, PlayerId});
        }
    }
    pl_now = pl_now % N_player + 1;
    groups_ship = groups_ship_new;
}

void MainWindow::updateTime()
{
    tmr->setInterval(timer_interval);
    move_ships();
    bool ch = read_next_event();
    if (game_not_stopped)
    {
        QWidget::repaint();

        if (!ch && groups_ship.size() == 0)
        {
            tmr->setInterval(1);
            return;
        }
    }
    else
    {
        tmr->stop();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->label_2->setText(QString::number(value));
    timer_interval = 1000 / value;
    tmr->setInterval(timer_interval);
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{
    skip = arg1;
}
