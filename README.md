## О проекте
Простая информационная система лекаственных препаратов, написанная на C++ с использованием Qt6


## Уствновка
1. Получите ключ GOOGLE maps Api [developers.google.com](https://developers.google.com/maps/documentation/javascript/get-api-key)
2. Установитезависимости
    ```bash
    sudo pacman -S qt6-base qt6-webengine cmake
    ```

3. Склонируйте репозиторий и соберите испольняемый файл
    ```bash
    git clone https://github.com/29121970t/cw2
    cd cw2
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
4. Выставите переменную окружения с полученным ключом
    ```bash
    export GOOGLE_MAPS_API_KEY=<Полученный-ключ>
    ```
5. Запустите приложение
    ```bash
    ./DrugInformationSystem
    ```

