#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <thread>
#include <queue>

int gridMap[1280][720] = {0};

struct Vector {
    int x;
    int y;
};

struct Point {
    int x, y;
};

// Функция для добавления пикселя в массив
void putPixel(sf::VertexArray& canvas, float x, float y, sf::Color color = sf::Color::White) {
    canvas.append(sf::Vertex{{x, y}, color});
    if (x >= 0 && x < 1280 && y >= 0 && y < 720) {
        gridMap[(int)x][(int)y] = 1;
    }
}

void addLine(sf::VertexArray& canvas, int x1, int y1, int x2, int y2, sf::Color color = sf::Color::White) {
    bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);

    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    int dx = x2 - x1;
    int ystep = (y1 < y2) ? 1 : -1;
    int y = y1;
    int dy = std::abs(y2 - y1);
    int d = 0;

    for (int x = x1; x <= x2; ++x) {
        if (steep) {
            putPixel(canvas, y, x, color);
        } else {
            putPixel(canvas, x, y, color);
        }

        d += 2 * dy;
        if (d >= dx) {
            y += ystep;
            d -= 2 * dx;
        }
    }
}


size_t floodFill(sf::VertexArray& canvas, Point start, sf::Color fillColor = sf::Color(0, 255, 0)) {
    std::queue<Point> q;
    q.push(start);
    size_t maxQueueSize = 0;

    while(!q.empty()) {
        maxQueueSize = std::max(maxQueueSize, q.size());

        Point p = q.front();
        q.pop();

        if (p.x < 0 || p.x >= 1280 || p.y < 0 || p.y >= 720) continue;

        if (gridMap[p.x][p.y] == 0) {
            gridMap[p.x][p.y] = 2;
            canvas.append(sf::Vertex{{(float)p.x, (float)p.y}, fillColor});
            q.push({p.x + 1, p.y});
            q.push({p.x - 1, p.y});
            q.push({p.x, p.y + 1});
            q.push({p.x, p.y - 1});
        }
    }
    return maxQueueSize;
}

struct Edge {
    int yMax;
    float xCurrent;
    float invSlope; // 1 / m = (x2 - x1) / (y2 - y1)
};

size_t scanlineFill(sf::VertexArray& canvas, const std::vector<Point>& poly, sf::Color fillColor = sf::Color(0, 255, 0)) {
    if (poly.size() < 3) return 0;

    int minY = poly[0].y;
    int maxY = poly[0].y;
    for (const auto& p : poly) {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    // Таблица ребер (Edge Table), сгруппированная по yMin
    std::vector<std::vector<Edge>> edgeTable(maxY - minY + 1);
    size_t edgeCount = 0;

    size_t n = poly.size();
    for (size_t i = 0; i < n; ++i) {
        Point p1 = poly[i];
        Point p2 = poly[(i + 1) % n];

        if (p1.y == p2.y) continue; // Горизонтальные ребра пропускаются

        if (p1.y > p2.y) std::swap(p1, p2);

        Edge e;
        e.yMax = p2.y;
        e.xCurrent = static_cast<float>(p1.x);
        e.invSlope = static_cast<float>(p2.x - p1.x) / static_cast<float>(p2.y - p1.y);

        edgeTable[p1.y - minY].push_back(e);
        edgeCount++;
    }

    std::vector<Edge> activeEdgeList;

    for (int y = minY; y <= maxY; ++y) {
        // 1. Добавляем новые ребра из Edge Table
        const auto& newEdges = edgeTable[y - minY];
        activeEdgeList.insert(activeEdgeList.end(), newEdges.begin(), newEdges.end());

        // 2. Удаляем ребра, у которых yMax <= y
        activeEdgeList.erase(
            std::remove_if(activeEdgeList.begin(), activeEdgeList.end(),
                           [y](const Edge& e) { return e.yMax <= y; }),
            activeEdgeList.end()
        );

        // 3. Сортируем активные ребра по xCurrent
        std::sort(activeEdgeList.begin(), activeEdgeList.end(),
                  [](const Edge& a, const Edge& b) { return a.xCurrent < b.xCurrent; });

        // 4. Закрашиваем пиксели парами (правило чет-нечет)
        for (size_t i = 0; i + 1 < activeEdgeList.size(); i += 2) {
            int xStart = static_cast<int>(std::ceil(activeEdgeList[i].xCurrent));
            int xEnd = static_cast<int>(std::floor(activeEdgeList[i + 1].xCurrent));

            for (int x = xStart; x <= xEnd; ++x) {
                if (x >= 0 && x < 1280 && y >= 0 && y < 720) {
                    canvas.append(sf::Vertex{sf::Vector2f(static_cast<float>(x), static_cast<float>(y)), fillColor});
                }
            }
        }

        // 5. Пересчитываем X для следующей строки
        for (auto& e : activeEdgeList) {
            e.xCurrent += e.invSlope;
        }
    }

    return edgeCount * sizeof(Edge);
}

int main() {
    // 1. Задаем фигуру (многоугольник)
    std::vector<Point> figure = {
        {600, 300}, {500, 473}, {300, 473}, {200, 300}, {300, 126}, {500, 126}
    };
    Point seedPoint = {400, 300};

    // Отрисовка контура фигуры для flood fill
    sf::VertexArray boundary(sf::PrimitiveType::Points);
    for (size_t i = 0; i < figure.size(); ++i) {
        Point p1 = figure[i];
        Point p2 = figure[(i + 1) % figure.size()];
        addLine(boundary, p1.x, p1.y, p2.x, p2.y, sf::Color::White);
    }

    // --- ТЕСТ 1: Закрашивание с затравкой (Flood Fill) ---
    sf::VertexArray floodResult(sf::PrimitiveType::Points);
    
    auto startFlood = std::chrono::high_resolution_clock::now();
    size_t peakQueue = floodFill(floodResult, seedPoint, sf::Color(50, 205, 50));
    auto endFlood = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> floodDuration = endFlood - startFlood;
    size_t floodMemory = sizeof(gridMap) + (peakQueue * sizeof(Point));

    // --- ТЕСТ 2: Закрашивание со сканирующей строкой (Scanline) ---
    sf::VertexArray scanlineResult(sf::PrimitiveType::Points);
    
    auto startScan = std::chrono::high_resolution_clock::now();
    size_t scanlineMemory = scanlineFill(scanlineResult, figure, sf::Color(30, 144, 255));
    auto endScan = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> scanDuration = endScan - startScan;

    // --- Вывод результатов сравнения в консоль ---
    std::cout << "========== РЕЗУЛЬТАТЫ СРАВНЕНИЯ ==========\n";
    std::cout << "[1] Заливка с затравкой (Flood Fill):\n";
    std::cout << "  - Время выполнения: " << floodDuration.count() << " мс\n";
    std::cout << "  - Закрашено точек:  " << floodResult.getVertexCount() << "\n";
    std::cout << "  - Оценка памяти:    " << floodMemory / 1024.0 << " КБ "
              << "(gridMap: " << sizeof(gridMap) / 1024.0 << " КБ + очередь: " 
              << (peakQueue * sizeof(Point)) / 1024.0 << " КБ)\n\n";

    std::cout << "[2] Построчное заполнение (Scanline Fill):\n";
    std::cout << "  - Время выполнения: " << scanDuration.count() << " мс\n";
    std::cout << "  - Закрашено точек:  " << scanlineResult.getVertexCount() << "\n";
    std::cout << "  - Оценка памяти:    " << scanlineMemory / 1024.0 << " КБ\n";
    std::cout << "===========================================\n";
    std::cout << "Управление в окне:\n";
    std::cout << "  Клавиша [1] - Показать заливку с затравкой (зеленая)\n";
    std::cout << "  Клавиша [2] - Показать Scanline заливку (синяя)\n";

    // 2. Отображение в окне
    sf::RenderWindow window(sf::VideoMode({1280, 720}), L"Сравнение алгоритмов закрашивания");
    int currentMode = 1;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Num1) {
                    currentMode = 1;
                } else if (keyPressed->code == sf::Keyboard::Key::Num2) {
                    currentMode = 2;
                }
            }
        }

        window.clear(sf::Color::Black);

        // Отрисовка контура
        window.draw(boundary);

        // Отрисовка выбранного алгоритма
        if (currentMode == 1) {
            window.draw(floodResult);
        } else if (currentMode == 2) {
            window.draw(scanlineResult);
        }

        window.display();
    }

    return 0;
}
