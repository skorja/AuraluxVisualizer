# AuraluxVisualizer
Сильные и упорные программисты смогут собрать этот проект и посмотреть как играют стратегии) Для получения лога необходимо запустить https://github.com/chegoryu/Auralux/tree/master/bin/game_runner (https://github.com/chegoryu/Auralux/tree/master/data/game_runner_configs пример конфига), после чего вы получите файл game_visualizer.log, если у вас всё получилось, то сможете даже посмотреть игру! Удачи ;)

# Установка
## Установка под мак

1) Установите https://github.com/chegoryu/Auralux под мак по инструкции
2) Скачайте репозиторий
```
git clone https://github.com/skorja/AuraluxVisualizer.git && cd AuraluxVisualizer
```
3) На маке не работеат нормально получение размера окна, поэтому придётся полезть в код.
Откройте файл ```mainwindow.cpp``` и найдите там строки
```
    std::ifstream conf("viz.conf");
    conf >> w >> h >> R;
    this->resize(w, h);
    conf.close();
```
Как показал опыт, работает замена этих 4 строк на такие строки:
```
    w = 700;
    h = 450;
    R = 9;
    this->resize(w, h);
```
5) Установите всё. Зайдите в папку ```AuraluxVisualizer``` и запустите
```
qmake AuraluxVisualizer.pro -o Makefile && make
```
После этого в папке появится приложение запуска визуализатора

