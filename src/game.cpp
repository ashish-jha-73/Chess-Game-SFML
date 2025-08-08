#include "game.hpp"
#include <iostream>

Game::Game()
    : mWindow({WINDOW_WIDTH, WINDOW_HEIGHT}, "Chess Game") {
    newGame();
    
    if (!font.loadFromFile("./assets/arial.ttf")) {
        std::cerr << "Failed to load font\n";
    }
    setupUI();
}

void Game::setupUI() {
    menuBar.setSize({static_cast<float>(WINDOW_WIDTH), static_cast<float>(MENU_BAR_HEIGHT)});
    menuBar.setPosition(0, 0);
    menuBar.setFillColor(sf::Color(50, 50, 100));
    
    titleText.setFont(font);
    titleText.setString("Chess Game");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(20, 20);
    
    statusText.setFont(font);
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(300, 30);
    
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(200, 350);
    gameOverText.setString("");
    
    newGameButton.setSize({150, 40});
    newGameButton.setPosition(600, 20);
    newGameButton.setFillColor(sf::Color(70, 130, 180));
    newGameButton.setOutlineColor(sf::Color::White);
    newGameButton.setOutlineThickness(2);
    
    undoButton.setSize({100, 40});
    undoButton.setPosition(600, 70);
    undoButton.setFillColor(sf::Color(70, 130, 180));
    undoButton.setOutlineColor(sf::Color::White);
    undoButton.setOutlineThickness(2);
    
    exitButton.setSize({100, 40});
    exitButton.setPosition(450, 70);
    exitButton.setFillColor(sf::Color(178, 34, 34));
    exitButton.setOutlineColor(sf::Color::White);
    exitButton.setOutlineThickness(2);
    
    newGameText.setFont(font);
    newGameText.setString("New Game");
    newGameText.setCharacterSize(20);
    newGameText.setFillColor(sf::Color::White);
    newGameText.setPosition(610, 25);
    
    undoText.setFont(font);
    undoText.setString("Undo");
    undoText.setCharacterSize(20);
    undoText.setFillColor(sf::Color::White);
    undoText.setPosition(625, 75);
    
    exitText.setFont(font);
    exitText.setString("Exit");
    exitText.setCharacterSize(20);
    exitText.setFillColor(sf::Color::White);
    exitText.setPosition(475, 75);
}

void Game::newGame() {
    gameState = GameState::Playing;
    whiteToMove = true;
    canCastleK[0] = canCastleK[1] = true;
    canCastleQ[0] = canCastleQ[1] = true;
    enPassantTarget = {-1, -1};
    moveHistory.clear();
    lastLegalMoves.clear();
    isPieceSelected = isDragging = false;
    
    const char init[BOARD_SIZE][BOARD_SIZE] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};
        
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            boardLogic[y][x] = init[y][x];

    float px = 0, py = MENU_BAR_HEIGHT;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
        board[i].setSize({static_cast<float>(SQUARE_SIZE), static_cast<float>(SQUARE_SIZE)});
        board[i].setPosition(px, py);
        int r = i / BOARD_SIZE, c = i % BOARD_SIZE;
        board[i].setFillColor((r + c) % 2 ? sf::Color(165, 42, 42) : sf::Color::White);
        board[i].setOutlineColor(sf::Color(100, 100, 100));
        board[i].setOutlineThickness(1.f);
        px += SQUARE_SIZE;
        if (c == BOARD_SIZE - 1) {
            px = 0;
            py += SQUARE_SIZE;
        }
    }

    highlight.setSize({static_cast<float>(SQUARE_SIZE), static_cast<float>(SQUARE_SIZE)});
    highlight.setFillColor({50, 200, 50, 100});

    loadTextures();
    initSprites();
}

void Game::run() {
    while (mWindow.isOpen()) {
        processEvent();
        update();
        render();
    }
}

void Game::loadTextures() {
    std::vector<std::string> codes = {
        "wp", "wr", "wn", "wb", "wq", "wk",
        "bp", "br", "bn", "bb", "bq", "bk"};
    for (auto &c : codes) {
        sf::Texture t;
        if (!t.loadFromFile("assets/" + c + ".png"))
            std::cerr << "Failed to load " << c << "\n";
        pieceTextures[c] = std::move(t);
    }
}

void Game::initSprites() {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            char p = boardLogic[y][x];
            if (p == ' ') continue;
            std::string code = isWhite(p) ? "w" : "b";
            code += tolower(p);
            auto &spr = pieceSprites[y][x];
            spr.setTexture(pieceTextures[code]);
            auto &tx = pieceTextures[code];
            spr.setScale(static_cast<float>(SQUARE_SIZE) / tx.getSize().x, 
                         static_cast<float>(SQUARE_SIZE) / tx.getSize().y);
            spr.setPosition(x * SQUARE_SIZE, y * SQUARE_SIZE + MENU_BAR_HEIGHT);
        }
    }
}

void Game::processEvent() {
    sf::Event ev;
    while (mWindow.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed)
            mWindow.close();

        if (ev.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(mWindow);
            
            if (newGameButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                newGame();
                return;
            }
            
            if (undoButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                undoMove();
                return;
            }
            
            if (exitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                mWindow.close();
                return;
            }
            
            if (gameState == GameState::Menu && 
                newGameButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                newGame();
                return;
            }
        }

        if (gameState == GameState::Playing) {
            if (ev.type == sf::Event::MouseButtonPressed && 
                ev.mouseButton.button == sf::Mouse::Left) {
                int x = ev.mouseButton.x / SQUARE_SIZE,
                    y = (ev.mouseButton.y - MENU_BAR_HEIGHT) / SQUARE_SIZE;
                    
                if (y >= 0 && y < BOARD_SIZE) {
                    char p = boardLogic[y][x];
                    if (p != ' ' && ((whiteToMove && isWhite(p)) || (!whiteToMove && isBlack(p)))) {
                        selectedSquare = {x, y};
                        isPieceSelected = isDragging = true;
                        std::string code = (isWhite(p) ? "w" : "b") + std::string(1, tolower(p));
                        draggedSprite.setTexture(pieceTextures[code]);
                        auto &tx = pieceTextures[code];
                        draggedSprite.setScale(static_cast<float>(SQUARE_SIZE) / tx.getSize().x, 
                                              static_cast<float>(SQUARE_SIZE) / tx.getSize().y);
                    }
                    lastLegalMoves.clear();
                    for (auto &mv : legalMoves()) {
                        if (mv[0] == x && mv[1] == y)
                            lastLegalMoves.insert(mv);
                    }
                }
            }

            if (ev.type == sf::Event::MouseButtonReleased && 
                ev.mouseButton.button == sf::Mouse::Left && 
                isPieceSelected) {
                int x = sf::Mouse::getPosition(mWindow).x / SQUARE_SIZE,
                    y = (sf::Mouse::getPosition(mWindow).y - MENU_BAR_HEIGHT) / SQUARE_SIZE;
                    
                if (y >= 0 && y < BOARD_SIZE) {
                    if (x != selectedSquare.x || y != selectedSquare.y)
                        handleMoves(selectedSquare.x, selectedSquare.y, x, y);
                }
                isPieceSelected = isDragging = false;
                lastLegalMoves.clear();
            }
        }
    }
}

void Game::update() {
    if (gameState == GameState::Playing) {
        std::string status = whiteToMove ? "White's turn" : "Black's turn";
        if (inCheck(whiteToMove)) {
            status += " (CHECK!)";
        }
        statusText.setString(status);
    }
    else if (gameState == GameState::Menu) {
        statusText.setString("Main Menu");
    }
}

void Game::checkGameState() {
    auto moves = legalMoves();
    if (moves.empty()) {
        gameState = GameState::GameOver;
        if (inCheck(whiteToMove)) {
            gameOverText.setString(whiteToMove ? "Black Wins!\nCheckmate" : "White Wins!\nCheckmate");
        } else {
            gameOverText.setString("Stalemate!\nDraw Game");
        }
    }
}

void Game::render() {
    mWindow.clear(sf::Color(40, 40, 40));
    
    mWindow.draw(menuBar);
    mWindow.draw(titleText);
    mWindow.draw(statusText);
    
    if (gameState == GameState::Menu) {
        newGameButton.setSize({200, 80});
        newGameButton.setPosition(300, 350);
        newGameButton.setFillColor(sf::Color(70, 130, 180));
        mWindow.draw(newGameButton);
        
        newGameText.setString("Play Chess");
        newGameText.setCharacterSize(36);
        newGameText.setPosition(325, 365);
        mWindow.draw(newGameText);
        
        sf::Text menuTitle = titleText;
        menuTitle.setCharacterSize(72);
        menuTitle.setPosition(200, 200);
        mWindow.draw(menuTitle);
    }
    else {
        for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i)
            mWindow.draw(board[i]);

        for (auto &mv : lastLegalMoves) {
            highlight.setPosition(mv[2] * SQUARE_SIZE, mv[3] * SQUARE_SIZE + MENU_BAR_HEIGHT);
            mWindow.draw(highlight);
        }

        initSprites();
        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                if (isDragging && x == selectedSquare.x && y == selectedSquare.y)
                    continue;
                if (boardLogic[y][x] != ' ')
                    mWindow.draw(pieceSprites[y][x]);
            }
        }

        if (isDragging) {
            auto mp = sf::Mouse::getPosition(mWindow);
            draggedSprite.setPosition(mp.x - 50, mp.y - 50);
            mWindow.draw(draggedSprite);
        }
        
        if (gameState == GameState::GameOver) {
            sf::RectangleShape overlay({static_cast<float>(WINDOW_WIDTH), 
                                       static_cast<float>(WINDOW_HEIGHT - MENU_BAR_HEIGHT)});
            overlay.setPosition(0, MENU_BAR_HEIGHT);
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            mWindow.draw(overlay);
            mWindow.draw(gameOverText);
            
            newGameButton.setSize({200, 60});
            newGameButton.setPosition(300, 500);
            newGameButton.setFillColor(sf::Color(70, 130, 180));
            mWindow.draw(newGameButton);
            
            newGameText.setString("Click On New Game");
            newGameText.setCharacterSize(15);
            newGameText.setPosition(335, 515);
            mWindow.draw(newGameText);
        }
        
        newGameButton.setSize({150, 40});
        newGameButton.setPosition(600, 20);
        mWindow.draw(newGameButton);
        mWindow.draw(undoButton);
        mWindow.draw(exitButton);
        
        newGameText.setString("New Game");
        newGameText.setCharacterSize(20);
        newGameText.setPosition(610, 25);
        mWindow.draw(newGameText);
        mWindow.draw(undoText);
        mWindow.draw(exitText);
    }
    
    mWindow.display();
}

std::set<std::vector<int>> Game::pseudoLegalMoves(bool w) const {
    std::set<std::vector<int>> M;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            char p = boardLogic[y][x];
            if (p == ' ' || (w != isWhite(p)))
                continue;

            int dir = isWhite(p) ? -1 : 1;

            switch (tolower(p)) {
                case 'p': {
                    int ny = y + dir;
                    if (inBounds(x, ny) && isEmpty(x, ny))
                        M.insert({x, y, x, ny});
                    if (((isWhite(p) && y == WHITE_PAWN_START_ROW) || 
                         (isBlack(p) && y == BLACK_PAWN_START_ROW))) {
                        int ny2 = y + 2 * dir;
                        if (inBounds(x, ny2) && isEmpty(x, ny) && isEmpty(x, ny2))
                            M.insert({x, y, x, ny2});
                    }
                    if (isOpponent(x - 1, ny, w))
                        M.insert({x, y, x - 1, ny});
                    if (isOpponent(x + 1, ny, w))
                        M.insert({x, y, x + 1, ny});
                    int ex = enPassantTarget.first, ey = enPassantTarget.second;
                    if (inBounds(ex, ey) && ey == y + dir && abs(ex - x) == 1 && boardLogic[y][ex] != ' ') {
                        M.insert({x, y, ex, ey});
                    }
                    break;
                }
                case 'n': {
                    const int DX[8] = {1, 2, 2, 1, -1, -2, -2, -1};
                    const int DY[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
                    for (int i = 0; i < 8; ++i) {
                        int nx = x + DX[i], ny = y + DY[i];
                        if (inBounds(nx, ny) && !sameColor(p, boardLogic[ny][nx]))
                            M.insert({x, y, nx, ny});
                    }
                    break;
                }
                case 'b': {
                    for (int dx : {-1, 1}) {
                        for (int dy : {-1, 1}) {
                            for (int i = 1; i < BOARD_SIZE; ++i) {
                                int nx = x + dx * i, ny = y + dy * i;
                                if (!inBounds(nx, ny)) break;
                                if (boardLogic[ny][nx] == ' ') {
                                    M.insert({x, y, nx, ny});
                                } else {
                                    if (isOpponent(nx, ny, w))
                                        M.insert({x, y, nx, ny});
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case 'r': {
                    for (int d : {-1, 1}) {
                        for (int i = 1; i < BOARD_SIZE; ++i) {
                            int nx = x + d * i;
                            if (!inBounds(nx, y)) break;
                            if (boardLogic[y][nx] == ' ') {
                                M.insert({x, y, nx, y});
                            } else {
                                if (isOpponent(nx, y, w))
                                    M.insert({x, y, nx, y});
                                break;
                            }
                        }
                        for (int i = 1; i < BOARD_SIZE; ++i) {
                            int ny = y + d * i;
                            if (!inBounds(x, ny)) break;
                            if (boardLogic[ny][x] == ' ') {
                                M.insert({x, y, x, ny});
                            } else {
                                if (isOpponent(x, ny, w))
                                    M.insert({x, y, x, ny});
                                break;
                            }
                        }
                    }
                    break;
                }
                case 'q': {
                    for (int dx : {-1, 1}) {
                        for (int dy : {-1, 1}) {
                            for (int i = 1; i < BOARD_SIZE; ++i) {
                                int nx = x + dx * i, ny = y + dy * i;
                                if (!inBounds(nx, ny)) break;
                                if (boardLogic[ny][nx] == ' ') {
                                    M.insert({x, y, nx, ny});
                                } else {
                                    if (isOpponent(nx, ny, w))
                                        M.insert({x, y, nx, ny});
                                    break;
                                }
                            }
                        }
                    }
                    for (int d : {-1, 1}) {
                        for (int i = 1; i < BOARD_SIZE; ++i) {
                            int nx = x + d * i;
                            if (!inBounds(nx, y)) break;
                            if (boardLogic[y][nx] == ' ') {
                                M.insert({x, y, nx, y});
                            } else {
                                if (isOpponent(nx, y, w))
                                    M.insert({x, y, nx, y});
                                break;
                            }
                        }
                        for (int i = 1; i < BOARD_SIZE; ++i) {
                            int ny = y + d * i;
                            if (!inBounds(x, ny)) break;
                            if (boardLogic[ny][x] == ' ') {
                                M.insert({x, y, x, ny});
                            } else {
                                if (isOpponent(x, ny, w))
                                    M.insert({x, y, x, ny});
                                break;
                            }
                        }
                    }
                    break;
                }
                case 'k': {
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (inBounds(nx, ny) && !sameColor(p, boardLogic[ny][nx]))
                                M.insert({x, y, nx, ny});
                        }
                    }
                    int row = w ? WHITE_BACK_ROW : BLACK_BACK_ROW;
                    if (canCastleK[w] && boardLogic[row][5] == ' ' && boardLogic[row][6] == ' ') {
                        M.insert({4, row, 6, row});
                    }
                    if (canCastleQ[w] && boardLogic[row][3] == ' ' && boardLogic[row][2] == ' ' && boardLogic[row][1] == ' ') {
                        M.insert({4, row, 2, row});
                    }
                    break;
                }
            }
        }
    }
    return M;
}

std::set<std::vector<int>> Game::legalMoves() {
    auto P = pseudoLegalMoves(whiteToMove);
    std::set<std::vector<int>> L;
    for (auto &mv : P) {
        int x1 = mv[0], y1 = mv[1], x2 = mv[2], y2 = mv[3];
        char pc = boardLogic[y1][x1], cap = boardLogic[y2][x2];
        bool isKingMove = (tolower(pc) == 'k');
        bool isCastling = (isKingMove && abs(x2 - x1) == 2);

        if (isCastling) {
            if (inCheck(whiteToMove))
                continue;
            int passX = (x1 + x2) / 2;
            if (isSquareAttacked(passX, y1, !whiteToMove))
                continue;
            if (isSquareAttacked(x2, y2, !whiteToMove))
                continue;
        }

        boardLogic[y2][x2] = pc;
        boardLogic[y1][x1] = ' ';
        bool ep = (tolower(pc) == 'p' && x2 != x1 && cap == ' ');
        char epCap = ' ';
        if (ep) {
            epCap = boardLogic[y1][x2];
            boardLogic[y1][x2] = ' ';
        }

        bool safe = !inCheck(whiteToMove);
        boardLogic[y1][x1] = pc;
        boardLogic[y2][x2] = cap;
        if (ep)
            boardLogic[y1][x2] = epCap;

        if (safe)
            L.insert(mv);
    }
    return L;
}

bool Game::inCheck(bool white) const {
    int kx = -1, ky = -1;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (boardLogic[y][x] == (white ? 'K' : 'k')) {
                kx = x;
                ky = y;
            }
        }
    }
    if (kx == -1) return false;
    return isSquareAttacked(kx, ky, !white);
}

bool Game::isSquareAttacked(int x, int y, bool byWhite) const {
    auto oppMoves = pseudoLegalMoves(byWhite);
    for (auto &mv : oppMoves) {
        if (mv[2] == x && mv[3] == y)
            return true;
    }
    return false;
}

void Game::handleMoves(int x1, int y1, int x2, int y2) {
    char pc = boardLogic[y1][x1], tgt = boardLogic[y2][x2];
    if (pc == ' ' || sameColor(pc, tgt))
        return;
    auto LM = legalMoves();
    if (!LM.count({x1, y1, x2, y2}))
        return;

    Move m;
    m.oldCastleK[0] = canCastleK[0];
    m.oldCastleK[1] = canCastleK[1];
    m.oldCastleQ[0] = canCastleQ[0];
    m.oldCastleQ[1] = canCastleQ[1];
    m.oldEnPassant = enPassantTarget;

    m.x1 = x1;
    m.y1 = y1;
    m.x2 = x2;
    m.y2 = y2;
    m.moved = pc;
    m.captured = tgt;

    m.wasEnPassant = (tolower(pc) == 'p' && x1 != x2 && tgt == ' ');
    if (m.wasEnPassant) {
        m.captured = boardLogic[y1][x2];
        boardLogic[y1][x2] = ' ';
    }

    if (tolower(m.captured) == 'r') {
        int ry = whiteToMove ? BLACK_BACK_ROW : WHITE_BACK_ROW;
        int capY = m.wasEnPassant ? y1 : y2;
        if (capY == ry) {
            if (x2 == 0) {
                canCastleQ[!whiteToMove] = false;
            } else if (x2 == 7) {
                canCastleK[!whiteToMove] = false;
            }
        }
    }

    if (tolower(pc) == 'k') {
        canCastleK[whiteToMove] = canCastleQ[whiteToMove] = false;
        if (abs(x2 - x1) == 2) {
            m.wasCastling = true;
            int ry = whiteToMove ? WHITE_BACK_ROW : BLACK_BACK_ROW;
            if (x2 > x1) {
                m.rookX1 = 7;
                m.rookY1 = ry;
                m.rookX2 = x2 - 1;
                m.rookY2 = ry;
            } else {
                m.rookX1 = 0;
                m.rookY1 = ry;
                m.rookX2 = x2 + 1;
                m.rookY2 = ry;
            }
            char r = boardLogic[m.rookY1][m.rookX1];
            boardLogic[m.rookY1][m.rookX1] = ' ';
            boardLogic[m.rookY2][m.rookX2] = r;
        }
    }

    if (tolower(pc) == 'r') {
        int ry = whiteToMove ? WHITE_BACK_ROW : BLACK_BACK_ROW;
        if (y1 == ry) {
            if (x1 == 0) canCastleQ[whiteToMove] = false;
            if (x1 == 7) canCastleK[whiteToMove] = false;
        }
    }

    boardLogic[y2][x2] = pc;
    boardLogic[y1][x1] = ' ';

    m.wasPromotion = false;
    if (tolower(pc) == 'p' && (y2 == BLACK_BACK_ROW || y2 == WHITE_BACK_ROW)) {
        m.wasPromotion = true;
        boardLogic[y2][x2] = whiteToMove ? 'Q' : 'q';
    }

    int dir = isWhite(pc) ? -1 : +1;
    if (tolower(pc) == 'p' && abs(y2 - y1) == 2) {
        enPassantTarget = {x2, y1 + dir};
    } else {
        enPassantTarget = {-1, -1};
    }

    moveHistory.push_back(m);
    whiteToMove = !whiteToMove;
    checkGameState();
}

void Game::undoMove() {
    if (moveHistory.empty())
        return;

    Move m = moveHistory.back();
    moveHistory.pop_back();
    whiteToMove = !whiteToMove;
    gameState = GameState::Playing;
    gameOverText.setString("");

    if (m.wasEnPassant) {
        boardLogic[m.y1][m.x1] = m.moved;
        boardLogic[m.y2][m.x2] = ' ';
        boardLogic[m.y1][m.x2] = m.captured;
    } else {
        boardLogic[m.y1][m.x1] = m.moved;
        boardLogic[m.y2][m.x2] = m.captured;
    }

    if (m.wasCastling) {
        char r = boardLogic[m.rookY2][m.rookX2];
        boardLogic[m.rookY2][m.rookX2] = ' ';
        boardLogic[m.rookY1][m.rookX1] = r;
    }

    if (m.wasPromotion) {
        boardLogic[m.y1][m.x1] = (whiteToMove ? 'P' : 'p');
    }

    canCastleK[0] = m.oldCastleK[0];
    canCastleK[1] = m.oldCastleK[1];
    canCastleQ[0] = m.oldCastleQ[0];
    canCastleQ[1] = m.oldCastleQ[1];
    enPassantTarget = m.oldEnPassant;
}