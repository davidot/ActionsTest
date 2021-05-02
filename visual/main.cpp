#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

#include <chess/Board.h>
#include <chess/MoveGen.h>
#include <util/RandomUtil.h>

int main() {
    // Tiny single pawn fast mate:
    // 1. h4 g5 2. hxg5 Nf6 3. gxf6 Bg7 4. fxg7 Nc6 5. gxh8=Q#
    // :( https://chess.stackexchange.com/questions/23775/what-are-the-quckest-helpmates-delivered-by-the-rook-knight-bishop-and-pawn

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

    Chess::Board board = Chess::Board::standardBoard();
    Chess::Move lastMove{};
    sf::Vector2f boardOffset = {100, 100};

    sf::Text edit;
    edit.setFont(font);
    edit.setCharacterSize(24);
    edit.setString("Edit mode enabled");
    edit.setFillColor(sf::Color::Green);
    edit.setPosition(boardOffset);

    sf::Text text;
    text.setFont(font); // font is a sf::Font
    text.setCharacterSize(24); // in pixels, not points!
    text.setFillColor(sf::Color::Red);


    constexpr float pieceSize = 32.f;

    sf::Sprite piece{texture};
    piece.setScale(squareSize / pieceSize, squareSize / pieceSize);

    sf::RectangleShape highlightSquare{{squareSize, squareSize}};
    highlightSquare.setOutlineThickness(2.5);
    highlightSquare.setFillColor(sf::Color::Transparent);

    bool editMode = false;
    sf::Vector2i selectedSquare = {-1, -1};

    constexpr const float boardSquareSize = 8;
    auto colRowToRect = [&](uint8_t col, uint8_t row) {
      return boardOffset + sf::Vector2f {squareSize * col, squareSize * (boardSquareSize - row)};
    };

    auto drawArrow = [&](int colFrom, int rowFrom, int colTo, int rowTo,
                         sf::Color bottomCol = {0x59, 0x98, 0x1A},
                         sf::Color topCol = {0x81, 0xB6, 0x22}) {
        if (colFrom == colTo && rowFrom == rowTo) {
            return;
        }
        auto centerBack = colRowToRect(colFrom, rowFrom) + sf::Vector2f{squareSize / 2.0, squareSize / 2.0};

        auto centerFront = colRowToRect(colTo, rowTo) + sf::Vector2f{squareSize / 2.0, squareSize / 2.0};
//
//        sf::VertexArray li{sf::Lines};
//        li.append(centerBack);
//        li.append(centerFront);
//        li[1].color = sf::Color::Blue;

        auto line = centerFront - centerBack;
        line *= 1.0f / std::sqrt(line.x * line.x + line.y * line.y);

        float thickness = 6.f;
        float relArrowSize = 4.f;

        auto lineHead = centerFront - (relArrowSize * thickness * line);

        sf::Vector2f normal1{-line.y, line.x};
        sf::Vector2f normal2{line.y, -line.x};

        sf::VertexArray v{sf::Triangles};

        v.append(centerBack + (normal1 * thickness));
        v.append(centerBack + (normal2 * thickness));
        v.append(lineHead + (normal2 * thickness));

        v.append(centerBack + (normal1 * thickness));
        v.append(lineHead + (normal1 * thickness));
        v.append(lineHead + (normal2 * thickness));

        v.append(lineHead + (normal1 * thickness * relArrowSize));
        v.append(centerFront);
        v.append(lineHead + (normal2 * thickness * relArrowSize));

        for (size_t i = 0; i < v.getVertexCount(); i++) {
            sf::Color& c = (i < 2u || i == 3) ? bottomCol : topCol;
            v[i].color = c;
        }

        window.draw(v);
//
//        sf::VertexArray li2{sf::Lines};
//        li2.append(centerBack);
//        li2.append(centerBack + (normal1 * thickness));
//        li2[0].color = sf::Color::Red;
//        li2[1].color = sf::Color::Red;
//
//        window.draw(li);
//        window.draw(li2);
    };

    std::string fenStatus;

    auto rng = util::seedRNG<std::mt19937_64>();

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event{};
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

            if (event.type == sf::Event::KeyPressed){
                switch (event.key.code) {
                    case sf::Keyboard::Space:
                        board.makeNullMove();
                        break;
                    case sf::Keyboard::E:
                        editMode = !editMode;
                        break;
                    default:
                        break;
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        auto list = Chess::generateAllMoves(board);

        if (ImGui::Begin("Hello, world!")) {
            std::string FEN = board.toFEN();

            ImGui::Separator();
            ImGui::Indent();
            ImGui::Dummy(ImVec2(0, 3));
            ImGui::TextWrapped("%s", FEN.c_str());
            ImGui::Dummy(ImVec2(0, 3));
            ImGui::Unindent();
            ImGui::Separator();

            if (ImGui::Button("Copy string value")) {
                sf::Clipboard::setString(FEN);
                fenStatus = "Copied!";
            }
            ImGui::SameLine(0.0f, 5.0f);

            bool boardChanged = false;
            if (ImGui::Button("Paste string value")) {
                auto s = sf::Clipboard::getString().toAnsiString();
                auto eBoard = Chess::Board::fromFEN(s);
                if (!eBoard) {
                    fenStatus = "Invalid FEN: _" + s + "_\n Got: " + eBoard.error();
                } else {
                    board = eBoard.extract();
                    boardChanged = true;
                    fenStatus = "Loaded";
                }
            }
            if (ImGui::Button("Reset board")) {
                board = Chess::Board::standardBoard();
                boardChanged = true;
            }

            ImGui::Text("%s", fenStatus.c_str());

            if (boardChanged) {
                list = Chess::generateAllMoves(board);
            }

            ImGui::Separator();
            ImGui::Text("%zu moves!", list.size());
            ImGui::Indent();
            if (list.size() > 0) {
                Chess::Move selected{};
                ImVec2 button_sz(50, 20);

                bool random = false;

                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.85f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.85f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.85f, 0.8f, 0.8f));
//                ImGui::Dummy(ImVec2((ImGui::GetContentRegionAvailWidth() - button_sz.x) / 2.0f, 25));
                ImGui::Dummy(ImVec2(0, 25));
                ImGui::SameLine();
                if (ImGui::Button("Random move", button_sz)) {
                    random = true;
                }
                static bool randomContinuous = false;
                static int randomCountdown = 0;
                static int randomTimer = 20;
                ImGui::SameLine();
                ImGui::Checkbox("Constant random", &randomContinuous);
                ImGui::SameLine();
                ImGui::DragInt("Timer", &randomTimer, 1.0f, 0, 300);
                ImGui::PopStyleColor(3);


                if (random || (randomContinuous && randomCountdown == 0)) {
                    size_t moveIndex = std::uniform_int_distribution<size_t>(0, list.size() - 1)(rng);
                    list.forEachMove([&selected, moveIndex, i = 0](const Chess::Move& move) mutable {
                        if (i++ == moveIndex) {
                            selected = move;
                        }
                    });
                    if (randomContinuous) {
                        randomCountdown = randomTimer;
                    }
                }
                if (randomCountdown > 0) {
                    --randomCountdown;
                }

                bool first = true;

                ImGuiStyle& style = ImGui::GetStyle();
                float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

                list.forEachMove([&](const Chess::Move& move) {
                    std::string val = board.moveToSAN(move);
                    // should! be unique

                    float last_button_x2 = ImGui::GetItemRectMax().x;
                    float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
                    if (!first && next_button_x2 < window_visible_x2)
                        ImGui::SameLine();

                    if (ImGui::Button(val.c_str(), button_sz)) {
                        selected = move;
                    }

                    first = false;
                });

                if (selected.fromPosition != selected.toPosition) {
                    lastMove = selected;
                    board.makeMove(selected);
                }
            } else {
                if (list.isCheckMate()) {
                    ImGui::Text("Checkmate");
                } else if (list.isStaleMate()) {
                    ImGui::Text("Stalemate");
                }
            }
            ImGui::Unindent();
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        uint8_t gray = 180;
        window.clear(sf::Color{gray, gray, gray});

        sf::RectangleShape square {{squareSize, squareSize}};


        for (int col = 0; col < boardSquareSize; col++) {
            for (int row = 0; row < boardSquareSize; row++) {
                if ((col + row) % 2 == 0) {
                    square.setFillColor(sf::Color::Black);
                } else {
                    square.setFillColor(sf::Color::White);
                }

                sf::Vector2f position = colRowToRect(col, row);
                square.setPosition(position);

                window.draw(square);

                auto pieceAt = board.pieceAt(col, row);
                if (pieceAt.has_value()) {
                    int pieceVal = pieceValMapping[pieceAt->type()] + (pieceAt->color() == Chess::Color::Black ? blackPieceOffset : 0);
                    piece.setTextureRect(sf::IntRect(pieceSize * pieceVal, 0, pieceSize, pieceSize));
                    piece.setPosition(position);
                    window.draw(piece);
                }
            }
        }

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        mousePosition -= sf::Vector2i(boardOffset.x, boardOffset.y);
        mousePosition /= (int)squareSize;
        mousePosition = sf::Vector2i(mousePosition.x, boardSquareSize - mousePosition.y);

        std::optional<sf::Vector2i> moveTo;

        if (mousePosition.x >= 0 && mousePosition.x < boardSquareSize
           && mousePosition.y >= 0 && mousePosition.y < boardSquareSize) {

            sf::Vector2f position = colRowToRect(mousePosition.x, mousePosition.y);
            highlightSquare.setPosition(position);

            highlightSquare.setOutlineColor(sf::Color(0, 0, 255, 200));
            window.draw(highlightSquare);
            if (clicked) {
                sf::Vector2i newPosition{mousePosition.x, mousePosition.y};
                if (selectedSquare.x >= 0) {
                    if (editMode) {
                        auto pieceFrom = board.pieceAt(selectedSquare.x, selectedSquare.y);
                        board.setPiece(selectedSquare.x, selectedSquare.y, std::nullopt);
                        board.setPiece(newPosition.x, newPosition.y, pieceFrom);
                        selectedSquare = newPosition;
                    } else {
                        moveTo = newPosition;
                    }
                } else {
                    selectedSquare = newPosition;
                }
            }
        } else if (clicked) {
            selectedSquare = {-1, -1};
        }

        if (selectedSquare.x >= 0) {
            drawArrow(selectedSquare.x, selectedSquare.y, mousePosition.x, mousePosition.y, sf::Color::Blue, sf::Color::Red);

            sf::Vector2f position = colRowToRect(selectedSquare.x, selectedSquare.y);
            highlightSquare.setPosition(position);

            highlightSquare.setOutlineColor(sf::Color(255, 0, 0, 200));
            window.draw(highlightSquare);

            Chess::Move mv;

            list.forEachMoveFrom(selectedSquare.x, selectedSquare.y, [&](const Chess::Move& move) {
                auto [col, row] = move.colRowToPosition();
                if (moveTo.has_value() && col == moveTo->x && row == moveTo->y) {
                    mv = move;
                }

                if (move.flag == Chess::Move::Flag::None) {
                    highlightSquare.setOutlineColor(sf::Color(0, 255, 0, 200));
                } else if (move.flag == Chess::Move::Flag::Castling) {
                    // cheaty hack to move position one into the board
                    col ^= 1u;
                    highlightSquare.setOutlineColor(sf::Color(0, 255, 255, 200));
                } else {
                    highlightSquare.setOutlineColor(sf::Color(0, 255, 255, 200));
                }
                sf::Vector2f position = colRowToRect(col, row);
                highlightSquare.setPosition(position);
                window.draw(highlightSquare);
            });

            if (moveTo.has_value() && mv.fromPosition != mv.toPosition) {
                lastMove = mv;
                board.makeMove(mv);
            }

        }

        if (moveTo.has_value()) {
            selectedSquare = {-1, -1};
        }

        if (lastMove.fromPosition != lastMove.toPosition) {
            static sf::Color lastColor {184, 15, 10, 100};
            auto [colFrom, rowFrom] = lastMove.colRowFromPosition();
            auto [colTo, rowTo] = lastMove.colRowToPosition();
            drawArrow(colFrom, rowFrom, colTo, rowTo, lastColor, lastColor);
        }

//        drawArrow(0, 0, mousePosition.x, mousePosition.y);

        text.setString(std::to_string(mousePosition.x) + "," + std::to_string(mousePosition.y) + (board.colorToMove() == Chess::Color::White ? " W" : " B"));
        window.draw(text);
        if (editMode) {
            window.draw(edit);
        }



        ImGui::SFML::Render(window);
        window.display();
    }


    ImGui::SFML::Shutdown();
}
