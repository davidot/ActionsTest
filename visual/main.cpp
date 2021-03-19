#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

#include <chess/Board.h>
#include <chess/MoveGen.h>

int main() {
    std::string baseFolder = "visual/resources/";
    sf::Texture texture;
    if (!texture.loadFromFile(baseFolder + "pieces.png"))
    {
        return -1;
    }

    sf::Font font;
    if (!font.loadFromFile(baseFolder + "font.otf")) {
        return -1;
    }

    static std::map<Chess::Piece::Type, int> pieceValMapping = {
            {Chess::Piece::Type::Pawn, 1},
            {Chess::Piece::Type::Knight, 2},
            {Chess::Piece::Type::Bishop, 3},
            {Chess::Piece::Type::Rook, 4},
            {Chess::Piece::Type::Queen, 5},
            {Chess::Piece::Type::King, 6},
    };

    static int blackPieceOffset = pieceValMapping.size();

    constexpr float squareSize = 64;

    sf::RenderWindow window(sf::VideoMode(1280, 960), "Visual");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    Chess::Board board = Chess::Board::fromFEN("rnbqk3/"
                                               "ppppp3/"
                                               "8/"
                                               "8/"
                                               "8/"
                                               "8/"
                                               "3PPPPP/"
                                               "3QKBNR"
                                               " w KQkq - 0 1").extract();
    sf::Vector2f boardOffset = {100, 100};

    sf::Text text;
    text.setFont(font); // font is a sf::Font
    text.setCharacterSize(24); // in pixels, not points!
    text.setFillColor(sf::Color::Red);


    sf::Sprite piece{texture};
    piece.setScale(squareSize / 32.0f, squareSize / 32.0f);

    sf::RectangleShape highlightSquare{{squareSize, squareSize}};
    highlightSquare.setOutlineThickness(2.5);
    highlightSquare.setFillColor(sf::Color::Transparent);


    sf::Vector2i selectedSquare = {-1, -1};

    constexpr const int boardSquareSize = 8;
    auto colRowToRect = [&](uint8_t col, uint8_t row) {
      return boardOffset + sf::Vector2f {squareSize * col, squareSize * (boardSquareSize - row)};
    };

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        bool clicked = false;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed
                || (event.type == sf::Event::KeyPressed
                    && (event.key.code == sf::Keyboard::Escape
                        || event.key.code == sf::Keyboard::Q))) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                clicked = true;
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                board.makeNullMove();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear(sf::Color{0, 142, 29});

        sf::RectangleShape square {{squareSize, squareSize}};


        for (int col = 0; col < boardSquareSize; col++) {
            for (int row = 0; row < boardSquareSize; row++) {
                if ((col + row) % 2 == 0) {
                    square.setFillColor(sf::Color::White);
                } else {
                    square.setFillColor(sf::Color::Black);
                }

                sf::Vector2f position = colRowToRect(col, row);
                square.setPosition(position);

                window.draw(square);

                auto pieceAt = board.pieceAt(col, row);
                if (pieceAt.has_value()) {
                    int pieceVal = pieceValMapping[pieceAt->type()] + (pieceAt->color() == Chess::Color::Black ? blackPieceOffset : 0);
                    piece.setTextureRect({32 * pieceVal, 0, 32, 32});
                    piece.setPosition(position);
                    window.draw(piece);
                }
            }
        }

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        mousePosition -= sf::Vector2i(boardOffset.x, boardOffset.y);
        mousePosition /= (int)squareSize;
        mousePosition = sf::Vector2i(mousePosition.x, boardSquareSize - mousePosition.y);
        if (mousePosition.x >= 0 && mousePosition.x < boardSquareSize
           && mousePosition.y >= 0 && mousePosition.y < boardSquareSize) {

            sf::Vector2f position = colRowToRect(mousePosition.x, mousePosition.y);
            highlightSquare.setPosition(position);

            highlightSquare.setOutlineColor(sf::Color(0, 0, 255, 200));
            window.draw(highlightSquare);
            if (clicked) {
                sf::Vector2i newPosition {mousePosition.x, mousePosition.y};
                if (selectedSquare.x >= 0) {
                    auto pieceFrom = board.pieceAt(selectedSquare.x, selectedSquare.y);
                    board.setPiece(selectedSquare.x, selectedSquare.y, std::nullopt);
                    board.setPiece(newPosition.x, newPosition.y, pieceFrom);
                    selectedSquare = {-1, -1};
                } else {
                    selectedSquare = newPosition;
                }
            }
        } else if (clicked) {
            selectedSquare = {-1, -1};
        }

        if (selectedSquare.x >= 0) {
            auto list = Chess::generateAllMoves(board);
            sf::Vector2f position = colRowToRect(selectedSquare.x, selectedSquare.y);
            highlightSquare.setPosition(position);

            highlightSquare.setOutlineColor(sf::Color(255, 0, 0, 200));
            window.draw(highlightSquare);

            list.forEachMoveFrom(selectedSquare.x, selectedSquare.y, [&](const Chess::Move& move) {
                auto [col, row] = move.colRowToPosition();
                sf::Vector2f position = colRowToRect(col, row);
                if (move.flag == Chess::Move::Flag::None) {
                    highlightSquare.setOutlineColor(sf::Color(0, 255, 0, 200));
                } else {
                    highlightSquare.setOutlineColor(sf::Color(0, 255, 255, 200));
                }
                highlightSquare.setPosition(position);
                window.draw(highlightSquare);
            });
        }

        text.setString(std::to_string(mousePosition.x) + "," + std::to_string(mousePosition.y) + (board.colorToMove() == Chess::Color::White ? " W" : " B"));
        window.draw(text);


        ImGui::SFML::Render(window);
        window.display();
    }


    ImGui::SFML::Shutdown();
}
