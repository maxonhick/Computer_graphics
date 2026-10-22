#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <thread>
#include <random>

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
            F += 4 * (y + x) + 10;
            x++;
        }
        y++;
    }

    canvas.push_back(circlePixels);
}

void addRectangle(std::vector<sf::VertexArray>& canvas, int x1, int y1, int width, int height, sf::Color color = sf::Color::White) {
    addLine(canvas, x1, y1, x1 + width, y1, color);
    addLine(canvas, x1, y1, x1, y1 + height, color);
    addLine(canvas, x1 + width, y1, x1 + width, y1 + height, color);
    addLine(canvas, x1, y1 + height, x1 + width, y1 + height, color);
}

int main() {
    srand(time(NULL));
    unsigned int WINDOW_WIDTH = 800;
    unsigned int WINDOW_HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Pixel Line");

    int dt = 17;

    int PADDLE_WIDTH = 16;
    int PADDLE_HEIGHT = 64;
    int p1_x = 30, p1_y = 252;
    int p2_x = 754, p2_y = 252;
    int PADDLE_SPEED = 11;

    int ball_x = 400;
    int ball_y = 300;
    int ball_radius = 10;
    int ball_start_speed = 6;
    int ball_max_speed = 13;
    int ball_speed = ball_start_speed;
    double max_bounce_angle = 1.05;
    double angle = (rand() % 60 - 30) / 180.0 * M_PI;
    int ball_direction = (rand() % 2 == 0) ? 1 : -1;
    int ball_speed_x = ball_direction * ball_start_speed * cos(angle);
    int ball_speed_y = ball_start_speed * sin(angle);

    std::vector<sf::VertexArray> canvas;
    addRectangle(canvas, p1_x, p1_y, PADDLE_WIDTH, PADDLE_HEIGHT);
    addRectangle(canvas, p2_x, p2_y, PADDLE_WIDTH, PADDLE_HEIGHT);
    addCircle(canvas, ball_x, ball_y, ball_radius);
    while (window.isOpen()) {
        window.clear(sf::Color::Black);
        
        for (auto& figure : canvas) {
            window.draw(figure);
        }
        
        window.display();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                    p1_y -= PADDLE_SPEED;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                    p1_y += PADDLE_SPEED;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                    p2_y -= PADDLE_SPEED;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                    p2_y += PADDLE_SPEED;
                }
            }
        }

        ball_x += ball_speed_x;
        ball_y += ball_speed_y;

        canvas.clear();
        addRectangle(canvas, p1_x, p1_y, PADDLE_WIDTH, PADDLE_HEIGHT);
        addRectangle(canvas, p2_x, p2_y, PADDLE_WIDTH, PADDLE_HEIGHT);
        addCircle(canvas, ball_x, ball_y, ball_radius);

        if (ball_y - ball_radius< 0) {
            ball_y = 0 + ball_radius;
            ball_speed_y *= -1;
        }
        if (ball_y + ball_radius> WINDOW_HEIGHT) {
            ball_y = WINDOW_HEIGHT - ball_radius;
            ball_speed_y *= -1;
        }
        if (ball_x - ball_radius <= p1_x + PADDLE_WIDTH &&
            ball_x + ball_radius >= p1_x &&
            ball_y + ball_radius >= p1_y &&
            ball_y - ball_radius <= p1_y + PADDLE_HEIGHT
        ) {
            double d = (double)(2 * ball_y - (2 * p1_y + PADDLE_HEIGHT)) / (double)(PADDLE_HEIGHT + 2 * ball_radius);
            std::cout << d << std::endl;
            ball_direction *= -1;
            ball_speed += 1;
            if (ball_speed > ball_max_speed) {
                ball_speed = ball_max_speed;
            }
            angle = d * max_bounce_angle;
            ball_speed_x = ball_direction * ball_speed * cos(angle);
            ball_speed_y = ball_speed * sin(angle);
            ball_x = p1_x + PADDLE_WIDTH + ball_radius;
        }
        if (ball_x - ball_radius <= p2_x + PADDLE_WIDTH &&
            ball_x + ball_radius >= p2_x &&
            ball_y + ball_radius >= p2_y &&
            ball_y - ball_radius <= p2_y + PADDLE_HEIGHT
        ) {
            double d = (double)(2 * ball_y - (2 * p2_y + PADDLE_HEIGHT)) / (double)(PADDLE_HEIGHT + 2 * (ball_radius));
            std::cout << d << std::endl;
            ball_direction *= -1;
            ball_speed += 1;
            if (ball_speed > ball_max_speed) {
                ball_speed = ball_max_speed;
            }
            angle = d * max_bounce_angle;
            ball_speed_x = ball_direction * ball_speed * cos(angle);
            ball_speed_y = ball_speed * sin(angle);
            ball_x = p2_x - ball_radius;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(dt));
    }
    return 0;
}