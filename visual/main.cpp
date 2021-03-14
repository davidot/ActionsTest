#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 960), "Visual");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear(sf::Color{0, 142, 29});

        float squareSize = 64;
        sf::RectangleShape square {{squareSize, squareSize}};

        square.setPosition(100, 100);
        for (int i = 0; i < 64; i++) {
            if (((i / 8) + i % 8) % 2 == 0) {
                square.setFillColor(sf::Color::White);
            } else {
                square.setFillColor(sf::Color::Black);
            }
            window.draw(square);

            if (i % 8 == 7) {
                square.move({-7 * squareSize, squareSize});
            } else {
                square.move({squareSize, 0});
            }
        }

        ImGui::SFML::Render(window);
        window.display();
    }


    ImGui::SFML::Shutdown();
}
