# DjVu Reader

Лёгкий кроссплатформенный просмотрщик DjVu на Qt6 + libdjvulibre.  
Поддерживает Windows и macOS.

## Возможности

- Открытие .djvu / .djv файлов
- Панель миниатюр страниц
- Зум колесом мыши (Ctrl + scroll) или кнопками
- Fit-to-width режим
- Навигация клавишами ←/→, PageUp/PageDown, Home/End
- Drag & drop файла в окно
- Тёмная тема

---

## Сборка на Windows

### 1. Установи зависимости

**Qt 6** (>= 6.4):  
https://www.qt.io/download-qt-installer  
Выбери компонент: `Qt 6.x → MSVC 2022 64-bit`

**vcpkg** (менеджер C++ пакетов):
```cmd
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install djvulibre:x64-windows
```

**CMake** (>= 3.20): https://cmake.org/download/

### 2. Собери проект

Открой **x64 Native Tools Command Prompt for VS 2022**:

```cmd
cd djvu-reader
cmake -B build -S . ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
cmake --build build --config Release
```

Готовый .exe будет в `build\Release\DjVuReader.exe`.

### 3. Деплой (чтобы запускалось без Qt в системе)

```cmd
cd build\Release
C:\Qt\6.x.x\msvc2022_64\bin\windeployqt.exe DjVuReader.exe
```

---

## Сборка на macOS

### 1. Установи зависимости

```bash
# Homebrew (если нет: https://brew.sh)
brew install qt djvulibre cmake
```

### 2. Собери проект

```bash
cd djvu-reader
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
cmake --build build --config Release
```

### 3. Запуск

```bash
open build/DjVuReader.app
# или:
./build/DjVuReader
```

### 4. Деплой (standalone .app)

```bash
$(brew --prefix qt)/bin/macdeployqt build/DjVuReader.app
```

---

## Управление

| Действие | Клавиша / мышь |
|---|---|
| Открыть файл | Ctrl+O |
| Следующая страница | → / PageDown |
| Предыдущая страница | ← / PageUp |
| Первая / последняя страница | Home / End |
| Zoom in | Ctrl + / Ctrl+колесо вверх |
| Zoom out | Ctrl − / Ctrl+колесо вниз |
| Fit to width | F |
| Открыть drag & drop | перетащи файл в окно |

---

## Структура проекта

```
djvu-reader/
├── CMakeLists.txt
└── src/
    ├── main.cpp              # точка входа
    ├── MainWindow.h/.cpp     # главное окно
    ├── DjVuDocument.h/.cpp   # обёртка над libdjvulibre
    ├── PageView.h/.cpp       # виджет просмотра с зумом
    └── ThumbnailPanel.h/.cpp # боковая панель миниатюр
```

## Stub-режим

Если libdjvulibre не найдена при сборке, CMake автоматически включит заглушку —
приложение соберётся и запустится, но вместо страниц будут серые плейсхолдеры.
Это удобно для разработки UI без установки библиотеки.

## Лицензия

MIT — делай что хочешь. libdjvulibre распространяется под GPL v2.
