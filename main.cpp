#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

void srednFiltr(sf::Image& image, sf::Image& tempImage, int kernelSize = 3) {
    sf::Vector2u size = image.getSize();
    int squareKernelSize = kernelSize * kernelSize;
    for (unsigned int y = kernelSize / 2; y < size.y - kernelSize / 2; ++y) {
        for (unsigned int x = kernelSize / 2; x < size.x - kernelSize / 2; ++x) {
            unsigned int sumR = 0, sumG = 0, sumB = 0;
            
            for (int j = -kernelSize / 2; j <= kernelSize / 2; ++j) {
                for (int i = -kernelSize / 2; i <= kernelSize / 2; ++i) {
                    sf::Color pixelColor = tempImage.getPixel({x + i, y + j});
                    sumR += pixelColor.r;
                    sumG += pixelColor.g;
                    sumB += pixelColor.b;
                }
            }
            sf::Color newColor(sumR / squareKernelSize, sumG / squareKernelSize, sumB / squareKernelSize);
            image.setPixel({x, y}, newColor);
        }
    }
}

void GaussFiltr(sf::Image& image, sf::Image& tempImage, int kernelSize = 3, float sigma = 1.0f) {
    sf::Vector2u size = image.getSize();
    for (unsigned int y = kernelSize / 2; y < size.y - kernelSize / 2; ++y) {
        for (unsigned int x = kernelSize / 2; x < size.x - kernelSize / 2; ++x) {
            unsigned int sumR = 0, sumG = 0, sumB = 0;
            
            for (int j = -kernelSize / 2; j <= kernelSize / 2; ++j) {
                for (int i = -kernelSize / 2; i <= kernelSize / 2; ++i) {
                    int curX = x + i;
                    int curY = y + j;
                    sf::Color pixelColor = tempImage.getPixel({x + i, y + j});

                    float weight = std::exp(-(i * i + j * j) / (2.0f * sigma * sigma)) / (M_PI * 2.0f * sigma * sigma);
                    
                    sumR += pixelColor.r * weight;
                    sumG += pixelColor.g * weight;
                    sumB += pixelColor.b * weight;
                }
            }
            sf::Color newColor(sumR, sumG, sumB);
            image.setPixel({x, y}, newColor);
        }
    }
}

void SobelFiltr(sf::Image& image, sf::Image& tempImage) {
    int kernelSize = 3;
    sf::Vector2u size = image.getSize();
    for (unsigned int y = kernelSize / 2; y < size.y - kernelSize / 2; ++y) {
        for (unsigned int x = kernelSize / 2; x < size.x - kernelSize / 2; ++x) {
            unsigned int sumRx = 0, sumGx = 0, sumBx = 0, sumRy = 0, sumGy = 0, sumBy = 0;
            
            int gx[3][3] = {
                {-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}
            };
            int gy[3][3] = {
                {-1, -2, -1},
                {0, 0, 0},
                {1, 2, 1}
            };

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    sf::Color pixelColor = tempImage.getPixel({x + i - 1, y + j - 1});

                    sumRx += pixelColor.r * gx[i][j];
                    sumGx += pixelColor.g * gx[i][j];
                    sumBx += pixelColor.b * gx[i][j];

                    sumRy += pixelColor.r * gy[i][j];
                    sumGy += pixelColor.g * gy[i][j];
                    sumBy += pixelColor.b * gy[i][j];
                }
            }
            sf::Color newColor(std::sqrt(sumRx * sumRx + sumRy * sumRy), 
                               std::sqrt(sumGx * sumGx + sumGy * sumGy), 
                               std::sqrt(sumBx * sumBx + sumBy * sumBy));
            image.setPixel({x, y}, newColor);
        }
    }
}

void updateWindowSizeForImage(sf::RenderWindow& window, sf::Sprite& sprite, const sf::Vector2u& imgSize) {
    const unsigned int MAX_WIDTH = 1440;
    const unsigned int MAX_HEIGHT = 900;

    unsigned int targetW = imgSize.x;
    unsigned int targetH = imgSize.y;

    // Если картинка больше экрана, уменьшаем пропорционально
    if (targetW > MAX_WIDTH || targetH > MAX_HEIGHT) {
        float ratio = std::min(static_cast<float>(MAX_WIDTH) / targetW, 
                               static_cast<float>(MAX_HEIGHT) / targetH);
        targetW = static_cast<unsigned int>(targetW * ratio);
        targetH = static_cast<unsigned int>(targetH * ratio);
    }

    // 1. Меняем размер физического окна
    window.setSize({targetW, targetH});

    // 2. Обновляем View (область видимости), чтобы не было искажений
    window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(targetW), static_cast<float>(targetH)})));

    // 3. Подгоняем масштаб спрайта под целевой размер
    sprite.setScale({
        static_cast<float>(targetW) / imgSize.x,
        static_cast<float>(targetH) / imgSize.y
    });
    sprite.setPosition({0.f, 0.f});
}

int main() {
    std::string currentPath = fs::current_path().parent_path().string().append("/images/");

    std::vector<fs::path> images;
    for (const auto& entry : fs::directory_iterator(currentPath)) {
        if (fs::is_regular_file(entry)) {
            images.push_back(entry.path());
        }
    }

    if (images.empty()) {
        std::cerr << "В папке images/ не найдено ни одного изображения!" << std::endl;
        return -1;
    }

    // 1. Загрузка исходного изображения
    int ImageIndex = 0;
    sf::Image originalImage;
    if (!originalImage.loadFromFile(images[ImageIndex])) {
        std::cerr << "Ошибка загрузки первого изображения!" << std::endl;
        return -1;
    }

    // Рабочая копия изображения для применения фильтров
    sf::Image processedImage = originalImage;

    // 2. Создание окна под размер картинки
    sf::RenderWindow window(sf::VideoMode(originalImage.getSize()), "SFML Filter Sandbox");
    window.setFramerateLimit(60);

    // 3. Текстура и спрайт для отрисовки
    sf::Texture texture;
    (void)texture.loadFromImage(processedImage);
    sf::Sprite sprite(texture);

    // 4. Главный цикл
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Num1) {
                    processedImage = originalImage;
                    srednFiltr(processedImage, originalImage);
                    (void)texture.loadFromImage(processedImage);
                } else if (keyPressed->code == sf::Keyboard::Key::Num2) {
                    processedImage = originalImage;
                    srednFiltr(processedImage, originalImage, 5);
                    (void)texture.loadFromImage(processedImage);
                } else if (keyPressed->code == sf::Keyboard::Key::Num3) {
                    processedImage = originalImage;
                    GaussFiltr(processedImage, originalImage, 3, 0.5f);
                    (void)texture.loadFromImage(processedImage);
                } else if (keyPressed->code == sf::Keyboard::Key::Num4) {
                    processedImage = originalImage;
                    SobelFiltr(processedImage, originalImage);
                    (void)texture.loadFromImage(processedImage);
                } else if (keyPressed->code == sf::Keyboard::Key::R) {
                    // Сброс к оригиналу
                    processedImage = originalImage;
                    (void)texture.loadFromImage(processedImage);
                } else if (keyPressed->code == sf::Keyboard::Key::S) {
                    // Сохранение результата
                    if (processedImage.saveToFile("output.png")) {
                        std::cout << "Сохранено в output.png" << std::endl;
                    } else {
                        std::cerr << "Не удалось сохранить файл output.png" << std::endl;
                    }
                } else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                } else if (keyPressed->code == sf::Keyboard::Key::Left) {
                    // Переключение на предыдущее изображение
                    ImageIndex = (ImageIndex + images.size() - 1) % images.size();
                    if (originalImage.loadFromFile(images[ImageIndex])) {
                        processedImage = originalImage;
                        (void)texture.loadFromImage(processedImage);
                        sprite.setTexture(texture, true); 
                        updateWindowSizeForImage(window, sprite, originalImage.getSize());
                    }
                } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                    // Переключение на следующее изображение
                    ImageIndex = (ImageIndex + 1) % images.size();
                    if (originalImage.loadFromFile(images[ImageIndex])) {
                        processedImage = originalImage;
                        (void)texture.loadFromImage(processedImage);
                        sprite.setTexture(texture, true); 
                        updateWindowSizeForImage(window, sprite, originalImage.getSize());
                    }
                }
            }
        }
        
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
/*
Библиотека с реализацией фильтров (средний, гауссовский, собеля) и демонстрацией их работы на изображении.
*/