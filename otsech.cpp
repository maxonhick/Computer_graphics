#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <thread>

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


Vector normal(Vector& vec) {
    Vector norm;
    norm.x = - vec.y;
    norm.y = vec.x;
    return norm;
}

std::tuple<float, bool, bool> point_perec(Vector& normal, Point& P1, Point& A, Point& B) {
    int znam = (B.x - A.x) * normal.x + (B.y - A.y) * normal.y;
    int chisl = (P1.x - A.x) * normal.x + (P1.y - A.y) * normal.y;
    if (znam == 0) {
        bool outside = (chisl < 0); 
        return {0, false, outside};
    }
    bool in = znam < 0 ? true : false;
    return {1.0f * chisl / znam, in, false};
}

std::pair<Point, Point> otsech(Point& A, Point& B, std::vector<Point>& figure) {
    float now_in_t = 0;
    float now_out_t = 1;
    Point in_point = {-1, -1};
    Point out_point = {-1, -1};
    for (int i = 1; i <= figure.size(); ++i) {
        Vector pi;
        pi.x = figure[i % figure.size()].x - figure[i - 1].x;
        pi.y = figure[i % figure.size()].y - figure[i - 1].y;
        Vector normal_vec = normal(pi);

        auto point_per = point_perec(normal_vec, figure[i - 1], A, B);
        float t = std::get<0>(point_per);
        bool in_flag = std::get<1>(point_per);
        bool is_parallel_outside = std::get<2>(point_per);
        
        if (is_parallel_outside) {
            return {{-1, -1}, {-1, -1}};
        }
        if ((B.x - A.x) * normal_vec.x + (B.y - A.y) * normal_vec.y == 0) {
            continue;
        }
        if (in_flag) {
            now_in_t = std::max(now_in_t, t);
        } else {
            now_out_t = std::min(now_out_t, t);
        }

        if (now_in_t > now_out_t) {
            return {{-1, -1}, {-1, -1}};
        }
    }
    Point resA = { (int)(A.x + now_in_t * (B.x - A.x)),  (int)(A.y + now_in_t * (B.y - A.y)) };
    Point resB = { (int)(A.x + now_out_t * (B.x - A.x)), (int)(A.y + now_out_t * (B.y - A.y)) };
    return {resA, resB};
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
        // {600, 300}, {500, 473}, {300, 473}, {200, 300}, {300, 126}, {500, 126}
        {500, 126}, {300, 126}, {200, 300}, {300, 473}, {500, 473}, {600, 300}
    };
    for (int i = 1; i <= figure.size(); ++i) {
        lines.push_back({figure[i % figure.size()].x, figure[i % figure.size()].y, figure[i - 1].x, figure[i - 1].y, sf::Color(0, 255, 0)});
    }

    std::vector<std::pair<Point, Point>> line = {
        {{350, 250}, {450, 350}},
        {{400, 200}, {400, 400}},

        // 2. Отрезок начинается внутри и выходит наружу (обрезается только одна точка)
        {{400, 300}, {700, 300}},
        {{350, 300}, {100, 300}},
        {{400, 200}, {400, 50}},

        // 3. Отрезок начинается снаружи и заканчивается внутри (обрезается только точка входа)
        {{100, 200}, {350, 200}},
        {{400, 600}, {400, 350}},

        // 4. Отрезок полностью снаружи и не пересекает многоугольник (должен полностью отсекаться)
        {{50, 50}, {150, 50}},
        {{650, 100}, {750, 200}},
        {{100, 600}, {600, 600}},

        // 5. Отрезок проходит через вершину многоугольника
        {{100, 300}, {700, 300}}, // проходит ровно через вершины (200, 300) и (600, 300)
        {{400, 0}, {200, 300}},   // целиком снаружи, но касается вершины (200, 300)

        // 6. Отрезок лежит точно на ребре многоугольника (граничный случай)
        {{300, 126}, {500, 126}}, // совпадает с верхним ребром
        {{250, 126}, {550, 126}}, // проходит вдоль верхней границы с выходом за неё

        // 7. Отрезок параллелен ребру, но снаружи
        {{200, 100}, {600, 100}}, // параллелен верхнему ребру y = 126

        // 8. Длинный отрезок, пересекающий фигуру по диагонали
        {{150, 100}, {550, 500}}
    };

    std::vector<std::pair<Point, Point>> draw_lines;
    for (auto lin : line) {
        draw_lines.push_back(otsech(lin.first, lin.second, figure));
    }

    for (auto lin : line) {
        addLine(Figures, lin.first.x, lin.first.y, lin.second.x, lin.second.y, sf::Color(0, 255, 0));
    }
    for (auto lin : draw_lines) {
        if (lin.first.x != -1 && lin.second.x != -1) {
            addLine(Figures, lin.first.x, lin.first.y, lin.second.x, lin.second.y);
        }
    }
    for (size_t i = 0; i < line.size(); ++i) {
        auto A = line[i].first;
        auto B = line[i].second;
        auto clipped_A = draw_lines[i].first;
        auto clipped_B = draw_lines[i].second;

        std::cout << "Отрезок [" << i << "] (" << A.x << ", " << A.y << ") -> (" << B.x << ", " << B.y << ") | Результат: ";

        if (clipped_A.x == -1) {
            std::cout << "ПОЛНОСТЬЮ УДАЛЕН (ошибка, если он пересекает фигуру)\n";
        } else {
            std::cout << "Отрисован (" << clipped_A.x << ", " << clipped_A.y << ") -> (" << clipped_B.x << ", " << clipped_B.y << ")\n";
        }
    }
    for (auto line : lines) {
        addLine(Figures, std::get<0>(line), std::get<1>(line), std::get<2>(line), std::get<3>(line), std::get<4>(line));
    }
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
