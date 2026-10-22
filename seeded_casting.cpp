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

void addLine(std::vector<sf::VertexArray>& canvas, int x1, int y1, int x2, int y2, sf::Color color = sf::Color::White) {
    sf::VertexArray linePixels(sf::PrimitiveType::Points);

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
            putPixel(linePixels, y, x, color);
        } else {
            putPixel(linePixels, x, y, color);
        }

        d += 2 * dy;
        if (d >= dx) {
            y += ystep;
            d -= 2 * dx;
        }
    }
    
    canvas.push_back(linePixels);
}


void zal(sf::VertexArray& canvas, const std::vector<Point>& figure, Point start) {
    std::queue<Point> q;
    q.push(start);
    while(!q.empty()) {
        Point p = q.front();
        q.pop();
        if (p.x < 0 || p.x >= 1280 || p.y < 0 || p.y >= 720) continue;

        if (gridMap[p.x][p.y] == 0) {
            gridMap[p.x][p.y] = 2;
            canvas.append(sf::Vertex{{(float)p.x, (float)p.y}, sf::Color(0, 255, 0)});
            q.push({p.x + 1, p.y});
            q.push({p.x - 1, p.y});
            q.push({p.x, p.y + 1});
            q.push({p.x, p.y - 1});
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Pixel Line");

    std::vector<sf::VertexArray> Figures;
    std::vector<std::tuple<int, int, int, int, sf::Color>> lines = {
        // {400, 400, 700, 400, sf::Color(255, 50, 50)},    // 0°   - Красный
        // {400, 400, 650, 250, sf::Color(255, 128, 0)},   // 30°  - Оранжевый
        // {400, 400, 576, 176, sf::Color(255, 255, 0)},   // 45°  - Желтый
        // {400, 400, 550, 150, sf::Color(128, 255, 0)},   // 60°  - Салатовый
        // {400, 400, 400, 100, sf::Color(0, 255, 0)},     // 90°  - Зеленый
        // {400, 400, 250, 150, sf::Color(0, 255, 128)},   // 120° - Мятный
        // {400, 400, 223, 176, sf::Color(0, 255, 255)},   // 135° - Циан
        // {400, 400, 150, 250, sf::Color(0, 128, 255)},   // 150° - Небесный
        // {400, 400, 100, 400, sf::Color(0, 0, 255)},     // 180° - Синий
        // {400, 400, 150, 550, sf::Color(128, 0, 255)},   // 210° - Фиолетовый
        // {400, 400, 223, 623, sf::Color(255, 0, 255)},   // 225° - Пурпурный
        // {400, 400, 250, 650, sf::Color(255, 0, 128)},   // 240° - Розовый
        // {400, 400, 400, 700, sf::Color(255, 100, 150)}, // 270° - Коралловый
        // {400, 400, 550, 650, sf::Color(180, 255, 100)}, // 300° - Лаймовый
        // {400, 400, 623, 623, sf::Color(0, 150, 150)},   // 315° - Бирюзовый
        // {400, 400, 650, 550, sf::Color(220, 220, 220)}
    };

    std::vector<Point> figure = {
         {600, 300}, {500, 473}, {300, 473}, {200, 300}, {300, 126}, {500, 126}
        // {500, 126}, {300, 126}, {200, 300}, {300, 473}, {500, 473}, {600, 300}
        // {640, 100}, {900, 360}, {640, 620}, {380, 360}
    };
    for (int i = 1; i <= figure.size(); ++i) {
        lines.push_back({figure[i % figure.size()].x, figure[i % figure.size()].y, figure[i - 1].x, figure[i - 1].y, sf::Color(0, 255, 0)});
    }

    
    for (auto line : lines) {
        addLine(Figures, std::get<0>(line), std::get<1>(line), std::get<2>(line), std::get<3>(line), std::get<4>(line));
    }
    std::vector<Point> canvas;
    std::cout << figure.size() << std::endl;
    sf::VertexArray fillpoint(sf::PrimitiveType::Points);
    zal(fillpoint, figure, {400, 300});
    //zal(fillpoint, figure, {650, 300});
    Figures.push_back(fillpoint);
    std::cout << Figures.size() << std::endl;
    

    sf::VertexArray points(sf::PrimitiveType::Points);
    int dx = 9;
    int dy = 3;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        
        for (const auto& figure : Figures) {
            window.draw(figure);
        }

        window.draw(points);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        
        
        window.display();
    }
    return 0;
}
