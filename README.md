# AuraluxVisualizer
Сильные и упорные программисты смогут собрать этот проект и посмотреть как играют стратегии) Для получения лога необходимо запустить https://github.com/chegoryu/Auralux/tree/master/bin/game_runner (https://github.com/chegoryu/Auralux/tree/master/data/game_runner_configs пример конфига), после чего вы получите файл game_visualizer.log, если у вас всё получилось, то сможете даже посмотреть игру! Удачи ;)

# Установка
## Установка под мак и линукс

1) Установите https://github.com/chegoryu/Auralux под мак по инструкции
2) Скачайте репозиторий
```
git clone https://github.com/skorja/AuraluxVisualizer.git && cd AuraluxVisualizer
```

3) Установите всё. Зайдите в папку ```AuraluxVisualizer``` и запустите
```
cmake -S . -B build && cmake --build build
```
После этого в папке `build` появится приложение запуска визуализатора

