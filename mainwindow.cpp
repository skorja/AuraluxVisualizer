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
bool ships_cnt = true;
bool game_not_stopped = true;
bool full_turn = false;
bool skip_step = false;
int skip_next_X_steps = 0;
int turn_counter = 0;
Ui::MainWindow * uii;
QVector<std::pair<int, int>> planets_coord;
QVector<std::tuple<int, int, int, int>> planet_state;
QVector<std::tuple<int, int, int, int, int>> groups_ship;
QVector<QVector<int>> distance;
QVector<QColor> colors {Qt::lightGray, Qt::red, Qt::blue, Qt::green, Qt::magenta};
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
    uii = ui;
    ui->setupUi(this);
    ui->label_2->setText("3");
    par = parent;
    int w, h;
    try {
        std::ifstream conf("viz.conf");
        conf >> w >> h >> R;
        conf.close();
    } catch (...) {
        w = 900;
        h = 600;
    }
    this->resize(w, h);
    W = this->size().width() - 20;
    H = this->size().height() - 40;
    // ui->label_2->setText(QString::number(H));
    fileName = QFileDialog::getOpenFileName(this);
    QFile tmp(fileName);
    if (!QFileInfo::exists(fileName)) return;
    try {
        fin.open(fileName.toStdString(), std::ios_base::in);
        fin >> N >> N_player;
        if (N_player > 1)
        {
            colors.push_back(QColor("#7FFFD4"));
            colors.push_back(QColor("#E32636"));
            colors.push_back(QColor("#FF2400"));
            colors.push_back(QColor("#AB274F"));
            colors.push_back(QColor("#9966CC"));
            colors.push_back(QColor("#CD9575"));
            colors.push_back(QColor("#44944A"));
            colors.push_back(QColor("#2F4F4F"));
            colors.push_back(QColor("#6A5ACD"));
            colors.push_back(QColor("#A8E4A0"));
            colors.push_back(QColor("#614051"));
            colors.push_back(QColor("#990066"));
            colors.push_back(QColor("#FAE7B5"));
            colors.push_back(QColor("#79553D"));
            colors.push_back(QColor("#C1876B"));
            colors.push_back(QColor("#003153"));
            colors.push_back(QColor("#3D2B1F"));
            colors.push_back(QColor("#F984E5"));
            colors.push_back(QColor("#CED23A"));
            colors.push_back(QColor("#FFDC33"));
            colors.push_back(QColor("#FF97BB"));
            colors.push_back(QColor("#DD80CC"));
            colors.push_back(QColor("#009B76"));
            colors.push_back(QColor("#480607"));
            colors.push_back(QColor("#B00000"));
            colors.push_back(QColor("#3E5F8A"));
            colors.push_back(QColor("#FFB02E"));
            colors.push_back(QColor("#900020"));
            colors.push_back(QColor("#45161C"));
            colors.push_back(QColor("#D5713F"));
            colors.push_back(QColor("#34C924"));
            colors.push_back(QColor("#00FF7F"));
            colors.push_back(QColor("#A7FC00"));
            colors.push_back(QColor("#BD33A4"));
            colors.push_back(QColor("#702963"));
            colors.push_back(QColor("#5E2129"));
            colors.push_back(QColor("#911E42"));
            colors.push_back(QColor("#256D7B"));
            colors.push_back(QColor("#FFCF48"));
            colors.push_back(QColor("#DF73FF"));
            colors.push_back(QColor("#734222"));
            colors.push_back(QColor("#C154C1"));
            colors.push_back(QColor("#F64A46"));
            colors.push_back(QColor("#00541F"));
            colors.push_back(QColor("#B57900"));
            colors.push_back(QColor("#00693E"));
            colors.push_back(QColor("#CA3767"));
            colors.push_back(QColor("#78866B"));
            colors.push_back(QColor("#158078"));
            colors.push_back(QColor("#2E8B57"));
            colors.push_back(QColor("#006633"));
            colors.push_back(QColor("#004953"));
            colors.push_back(QColor("#FFD700"));
            colors.push_back(QColor("#4B0082"));
            colors.push_back(QColor("#1B5583"));
            colors.push_back(QColor("#FF0033"));
        }
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
    if (skip_step) return;
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
        if (ships_cnt)
        {
            canv.drawText(x - r * 2 - 8, y + r, r * 4 + 16, 16, Qt::AlignCenter, QString::number(ShipCount));
        }
    }
    r = R / 3;
    for (auto [FromPlanetId, ToPlanetId, Count, time, PlayerId]: groups_ship)
    {
        pen.setColor(colors[PlayerId]);
        canv.setPen(pen);
        std::tie(x, y) = coord_move(FromPlanetId, ToPlanetId, time);
        canv.drawEllipse(QRect(x - r, y - r, 2 * r, 2 * r));
        if (ships_cnt)
        {
            canv.drawText(x - r * 2 - 8, y - 2 * R, r * 4 + 16, 16, Qt::AlignCenter, QString::number(Count));
        }
    }
}

bool read_next_event()
{
    uii->Turn_counter_text->setTitle(QString::number(++turn_counter) + '/' + QString::number(turn_counter / N_player));
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
        if ((full_turn && pl_now != 1) || skip_next_X_steps > 0 || (!ch && groups_ship.size() == 0))
        {
            if (skip_next_X_steps > 0)
            {
                --skip_next_X_steps;
            }
            skip_step = true;
            tmr->setInterval(0);
            return;
        }
        skip_step = false;
        QWidget::repaint();
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

void MainWindow::on_actionSkip_empty_turns_changed()
{
    skip = ui->actionSkip_empty_turns->isChecked();
}

void MainWindow::on_actionShip_counter_text_changed()
{
    ships_cnt = ui->actionShip_counter_text->isChecked();
}

void MainWindow::on_pushButton_clicked()
{
    skip_next_X_steps = 100;
    tmr->setInterval(1);
}

void MainWindow::on_actionFull_turn_changed()
{
    full_turn = ui->actionFull_turn->isChecked();
}

void MainWindow::on_pushButton_2_clicked()
{
    skip_next_X_steps = 500 * N_player;
    tmr->setInterval(1);
}
