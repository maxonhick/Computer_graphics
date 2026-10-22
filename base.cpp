#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <thread>

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

void addCircle(std::vector<sf::VertexArray>& canvas, int centerX, int centerY, int radius, sf::Color color = sf::Color::White) {
    sf::VertexArray circlePixels(sf::PrimitiveType::Points);

    int F = 3 - 2 * radius;
    int x = (-radius);
    int y = 0;

    while (y < (-x)) {
        putPixel(circlePixels, centerX + x, centerY + y, color);
        putPixel(circlePixels, centerX + x, centerY - y, color);
        putPixel(circlePixels, centerX - x, centerY + y, color);
        putPixel(circlePixels, centerX - x, centerY - y, color);
        putPixel(circlePixels, centerX + y, centerY + x, color);
        putPixel(circlePixels, centerX + y, centerY - x, color);
        putPixel(circlePixels, centerX - y, centerY + x, color);
        putPixel(circlePixels, centerX - y, centerY - x, color);

        if (F < 0) {
            F += 4 * y + 6;
        } else {
            F += 4 * y + 2 * x + 10;
            x++;
        }
        y++;
    }

    canvas.push_back(circlePixels);
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

    for (auto line : lines) {
        addLine(Figures, std::get<0>(line), std::get<1>(line), std::get<2>(line), std::get<3>(line), std::get<4>(line));
    }
    std::cout << Figures.size() << std::endl;
    
    std::vector<std::tuple<int, int, int, sf::Color>> circles = {
        // {400, 400, 100, sf::Color(255, 255, 255)}, // Белый
        // {300, 300, 200, sf::Color(255, 0, 0)},     // Красный
        // {500, 500, 100, sf::Color(0, 255, 0)},     // Зеленый
        // {450, 425, 50, sf::Color(0, 0, 255)}      // Синий
    };

    for (auto circle : circles) {
        addCircle(Figures, std::get<0>(circle), std::get<1>(circle), std::get<2>(circle), std::get<3>(circle));
    }

    sf::VertexArray points(sf::PrimitiveType::Points);
    std::tuple<int, int, int, sf::Color> circle = {400, 400, 100, sf::Color(255, 255, 255)};
    addCircle(Figures, std::get<0>(circle), std::get<1>(circle), std::get<2>(circle), std::get<3>(circle));
    int dx = 9;
    int dy = 3;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (event->is<sf::Event::MouseButtonPressed>()) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                putPixel(points, mousePosition.x, mousePosition.y);
            }
        }

        window.clear(sf::Color::Black);
        
        for (const auto& figure : Figures) {
            window.draw(figure);
        }

        window.draw(points);

        Figures.pop_back();
        if (std::get<0>(circle) + std::get<2>(circle) > window.getSize().x || std::get<0>(circle) - std::get<2>(circle) < 0) {
            dx = -dx;
        }
        if (std::get<1>(circle) + std::get<2>(circle) > window.getSize().y || std::get<1>(circle) - std::get<2>(circle) < 0) {
            dy = -dy;
        }
        // std::cout << "x: " << std::get<0>(circle) << " dx: " << dx << std::endl;
        int new_x = std::get<0>(circle) + dx;
        int new_y = std::get<1>(circle) + dy;

        addCircle(Figures, new_x, new_y, 100, std::get<3>(circle));
        circle = {new_x, new_y, 100, std::get<3>(circle)};
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        
        
        window.display();
    }
    return 0;
}