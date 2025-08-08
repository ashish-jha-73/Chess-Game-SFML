#include "game.hpp"
#include <iostream>

// —————————————————————————————————————————————————————————
// Constructor: initialize boardLogic, UI grid, highlights, textures
// —————————————————————————————————————————————————————————
Game::Game() : mWindow({800, 800}, "Chess by Gemini") {
    // 1. Logical start position
    const char init[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            boardLogic[y][x] = init[y][x];

    // 2. Build 8×8 square grid
    mWindow.setFramerateLimit(60);
    float sq = 800.f / 8;
    for (int i = 0; i < 64; ++i) {
        board[i].setSize({sq, sq});
        int r = i / 8, c = i % 8;
        board[i].setPosition(c * sq, r * sq);
        board[i].setFillColor((r + c) % 2 ? sf::Color(181, 136, 99) : sf::Color(240, 217, 181));
    }

    // 3. Highlight square setup
    highlight.setSize({sq, sq});
    highlight.setFillColor({50, 200, 50, 100});

    // 4. Load assets
    loadTextures();
    if (!mFont.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Failed to load font!\n";
    }

    // 5. Setup Game Over Text
    mGameOverText.setFont(mFont);
    mGameOverText.setCharacterSize(60);
    mGameOverText.setFillColor(sf::Color::Red);
    mGameOverText.setStyle(sf::Text::Bold);

    initSprites();
}

// —————————————————————————————————————————————————————————
// Main loop
// —————————————————————————————————————————————————————————
void Game::run() {
    while (mWindow.isOpen()) {
        processEvent();
        update();
        render();
    }
}

// —————————————————————————————————————————————————————————
// Load piece textures from `assets/`
// —————————————————————————————————————————————————————————
void Game::loadTextures() {
    std::vector<std::string> codes = {
        "wp", "wr", "wn", "wb", "wq", "wk",
        "bp", "br", "bn", "bb", "bq", "bk"};
    for (auto& c : codes) {
        sf::Texture t;
        if (!t.loadFromFile("assets/" + c + ".png"))
            std::cerr << "Failed to load " << c << "\n";
        pieceTextures[c] = std::move(t);
    }
}

// —————————————————————————————————————————————————————————
// Initialize sprites to match boardLogic
// —————————————————————————————————————————————————————————
void Game::initSprites() {
    float sq = 800.f / 8;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char p = boardLogic[y][x];
            if (p == ' ') continue;
            std::string code = isWhite(p) ? "w" : "b";
            code += tolower(p);
            auto& spr = pieceSprites[y][x];
            spr.setTexture(pieceTextures[code]);
            auto& tx = pieceTextures[code];
            spr.setScale(sq / tx.getSize().x, sq / tx.getSize().y);
            spr.setPosition(x * sq, y * sq);
        }
    }
}

// —————————————————————————————————————————————————————————
// Process input: clicks, dragging, undo (U key)
// —————————————————————————————————————————————————————————
void Game::processEvent() {
    sf::Event ev;
    while (mWindow.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) mWindow.close();

        // Undo last move on 'U'
        if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::U) undoMove();

        // --- Handle input only when game is actively being played ---
        if (mGameState == PLAYING) {
            // Mouse press: pick up piece
            if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                int x = ev.mouseButton.x / 100, y = ev.mouseButton.y / 100;
                char p = boardLogic[y][x];
                if (p != ' ' && ((whiteToMove && isWhite(p)) || (!whiteToMove && isBlack(p)))) {
                    selectedSquare = {x, y};
                    isPieceSelected = isDragging = true;
                    // Prepare dragged sprite
                    std::string code = (isWhite(p) ? "w" : "b") + std::string(1, tolower(p));
                    draggedSprite.setTexture(pieceTextures[code]);
                    float sq = 100.f;
                    auto& tx = pieceTextures[code];
                    draggedSprite.setScale(sq / tx.getSize().x, sq / tx.getSize().y);
                    
                    // Highlight legal destinations
                    lastLegalMoves.clear();
                    for (auto& mv : legalMoves()) {
                        if (mv[0] == x && mv[1] == y)
                            lastLegalMoves.insert(mv);
                    }
                }
            }

            // Mouse release: drop & move
            if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left && isPieceSelected) {
                int x = ev.mouseButton.x / 100, y = ev.mouseButton.y / 100;
                if (inBounds(x,y) && (x != selectedSquare.x || y != selectedSquare.y)) {
                    handleMove(selectedSquare.x, selectedSquare.y, x, y);
                }
                isPieceSelected = isDragging = false;
                lastLegalMoves.clear();
            }
        }
    }
}

// —————————————————————————————————————————————————————————
// (No per-frame simulation needed here)
// —————————————————————————————————————————————————————————
void Game::update() {}

// —————————————————————————————————————————————————————————
// Draw everything: board, highlights, pieces, dragged sprite
// —————————————————————————————————————————————————————————
void Game::render() {
    mWindow.clear();
    // 1) Board squares
    for (int i = 0; i < 64; ++i) mWindow.draw(board[i]);

    // 2) Highlights for legal moves
    for (auto& mv : lastLegalMoves) {
        highlight.setPosition(mv[2] * 100.f, mv[3] * 100.f);
        mWindow.draw(highlight);
    }
    
    // 3) Pieces on the board
    initSprites();
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            if (isDragging && x == selectedSquare.x && y == selectedSquare.y) continue;
            if (boardLogic[y][x] != ' ') mWindow.draw(pieceSprites[y][x]);
        }

    // 4) Dragged piece under cursor
    if (isDragging) {
        auto mp = sf::Mouse::getPosition(mWindow);
        draggedSprite.setPosition(mp.x - 50.f, mp.y - 50.f); // center on cursor
        mWindow.draw(draggedSprite);
    }
    
    // 5) Game Over Message
    if (mGameState != PLAYING) {
        mWindow.draw(mGameOverText);
    }
    
    mWindow.display();
}

// —————————————————————————————————————————————————————————
// Is a specific square attacked by the opponent?
// —————————————————————————————————————————————————————————
bool Game::isSquareAttacked(int x, int y, bool byWhite) const {
    auto oppMoves = pseudoLegalMoves(byWhite);
    for (auto& mv : oppMoves) {
        if (mv[2] == x && mv[3] == y) return true;
    }
    return false;
}

// —————————————————————————————————————————————————————————
// Generate pseudo-legal moves (ignores check)
// —————————————————————————————————————————————————————————
std::set<std::vector<int>> Game::pseudoLegalMoves(bool w) const {
    std::set<std::vector<int>> M;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char p = boardLogic[y][x];
            if (p == ' ' || (w != isWhite(p))) continue;

            int dir = isWhite(p) ? -1 : 1;

            switch (tolower(p)) {
            case 'p': { // PAWN
                int ny = y + dir;
                // Single push
                if (isEmpty(x, ny)) M.insert({x, y, x, ny});
                // Double push from start rank
                if (((isWhite(p) && y == 6) || (isBlack(p) && y == 1))) {
                    int ny2 = y + 2 * dir;
                    if (isEmpty(x, ny) && isEmpty(x, ny2)) M.insert({x, y, x, ny2});
                }
                // Captures
                if (isOpponent(x - 1, ny, w)) M.insert({x, y, x - 1, ny});
                if (isOpponent(x + 1, ny, w)) M.insert({x, y, x + 1, ny});
                // En passant
                int ex = enPassantTarget.first, ey = enPassantTarget.second;
                if (ex != -1 && ey == y + dir && abs(ex - x) == 1) {
                    M.insert({x, y, ex, ey});
                }
                break;
            }
            case 'n': { // KNIGHT
                const int DX[8] = {1, 2, 2, 1, -1, -2, -2, -1};
                const int DY[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
                for (int i = 0; i < 8; ++i) {
                    int nx = x + DX[i], ny = y + DY[i];
                    if (inBounds(nx, ny) && !sameColor(p, boardLogic[ny][nx])) M.insert({x, y, nx, ny});
                }
                break;
            }
            case 'b': // BISHOP
            case 'r': // ROOK
            case 'q': { // QUEEN
                const int DIRS[8][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}, {-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                int start = (tolower(p) == 'b') ? 0 : (tolower(p) == 'r') ? 4 : 0;
                int end = (tolower(p) == 'b') ? 4 : (tolower(p) == 'r') ? 8 : 8;
                for (int i = start; i < end; ++i) {
                    for (int j = 1; j < 8; ++j) {
                        int nx = x + DIRS[i][0] * j, ny = y + DIRS[i][1] * j;
                        if (!inBounds(nx, ny)) break;
                        if (boardLogic[ny][nx] == ' ') {
                            M.insert({x, y, nx, ny});
                        } else {
                            if (isOpponent(nx, ny, w)) M.insert({x, y, nx, ny});
                            break;
                        }
                    }
                }
                break;
            }
            case 'k': { // KING
                for (int dx = -1; dx <= 1; ++dx)
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (inBounds(nx, ny) && !sameColor(p, boardLogic[ny][nx])) M.insert({x, y, nx, ny});
                    }
                // Castling
                int row = w ? 7 : 0;
                if (canCastleK[w] && isEmpty(5, row) && isEmpty(6, row)) M.insert({4, row, 6, row});
                if (canCastleQ[w] && isEmpty(3, row) && isEmpty(2, row) && isEmpty(1, row)) M.insert({4, row, 2, row});
                break;
            }
            }
        }
    }
    return M;
}

// —————————————————————————————————————————————————————————
// Filter pseudo-legal moves to get only fully legal moves
// —————————————————————————————————————————————————————————
std::set<std::vector<int>> Game::legalMoves() {
    auto P = pseudoLegalMoves(whiteToMove);
    std::set<std::vector<int>> L;

    for (auto& mv : P) {
        int x1 = mv[0], y1 = mv[1], x2 = mv[2], y2 = mv[3];
        char pc = boardLogic[y1][x1], cap = boardLogic[y2][x2];

        bool isKingMove = (tolower(pc) == 'k');
        bool isCastling = (isKingMove && abs(x2 - x1) == 2);

        if (isCastling) {
            if (inCheck(whiteToMove)) continue; // Can't castle out of check
            int passX = (x1 + x2) / 2;
            if (isSquareAttacked(passX, y1, !whiteToMove) || isSquareAttacked(x2, y2, !whiteToMove))
                continue; // Can't castle through or into check
        }

        // Simulate the move
        boardLogic[y2][x2] = pc;
        boardLogic[y1][x1] = ' ';
        
        bool ep = (tolower(pc) == 'p' && x2 != x1 && cap == ' ');
        if (ep) boardLogic[y1][x2] = ' '; // Remove captured pawn in en passant
        
        // If the king is not in check after the move, it's legal
        if (!inCheck(whiteToMove)) L.insert(mv);

        // Undo simulation
        boardLogic[y1][x1] = pc;
        boardLogic[y2][x2] = cap;
        if (ep) boardLogic[y1][x2] = whiteToMove ? 'p' : 'P';
    }
    return L;
}


// —————————————————————————————————————————————————————————
// Is the current player in check?
// —————————————————————————————————————————————————————————
bool Game::inCheck(bool white) const {
    int kx = -1, ky = -1;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            if (boardLogic[y][x] == (white ? 'K' : 'k')) {
                kx = x; ky = y; break;
            }
    return isSquareAttacked(kx, ky, !white);
}

// —————————————————————————————————————————————————————————
// Execute a move, handling all special rules + record history
// —————————————————————————————————————————————————————————
void Game::handleMove(int x1, int y1, int x2, int y2) {
    if (!legalMoves().count({x1, y1, x2, y2})) return;

    Move m;
    m.x1 = x1; m.y1 = y1; m.x2 = x2; m.y2 = y2;
    m.moved = boardLogic[y1][x1]; m.captured = boardLogic[y2][y2];
    m.oldCastleK[0] = canCastleK[0]; m.oldCastleK[1] = canCastleK[1];
    m.oldCastleQ[0] = canCastleQ[0]; m.oldCastleQ[1] = canCastleQ[1];
    m.oldEnPassant = enPassantTarget;

    char pc = m.moved;

    // --- Special Move Logic ---
    // En Passant
    if (tolower(pc) == 'p' && x1 != x2 && m.captured == ' ') {
        m.wasEnPassant = true;
        int capY = whiteToMove ? y2 + 1 : y2 - 1;
        m.captured = boardLogic[capY][x2];
        boardLogic[capY][x2] = ' ';
    }
    // Castling
    if (tolower(pc) == 'k' && abs(x2 - x1) == 2) {
        m.wasCastling = true;
        int ry = y1;
        if (x2 > x1) { // Kingside
            m.rookX1 = 7; m.rookY1 = ry; m.rookX2 = 5; m.rookY2 = ry;
        } else { // Queenside
            m.rookX1 = 0; m.rookY1 = ry; m.rookX2 = 3; m.rookY2 = ry;
        }
        char r = boardLogic[m.rookY1][m.rookX1];
        boardLogic[m.rookY1][m.rookX1] = ' ';
        boardLogic[m.rookY2][m.rookX2] = r;
    }
    
    // --- Update Board and State ---
    boardLogic[y2][x2] = pc;
    boardLogic[y1][x1] = ' ';

    // Pawn Promotion (auto-queen)
    if (tolower(pc) == 'p' && (y2 == 0 || y2 == 7)) {
        m.wasPromotion = true;
        boardLogic[y2][x2] = whiteToMove ? 'Q' : 'q';
    }

    // Set new En Passant target
    enPassantTarget = {-1, -1};
    if (tolower(pc) == 'p' && abs(y2 - y1) == 2) {
        enPassantTarget = {x1, (y1 + y2) / 2};
    }

    // Update Castling Rights
    if (tolower(pc) == 'k') { canCastleK[whiteToMove] = canCastleQ[whiteToMove] = false; }
    if (pc == 'R' && x1 == 7 && y1 == 7) canCastleK[0] = false;
    if (pc == 'R' && x1 == 0 && y1 == 7) canCastleQ[0] = false;
    if (pc == 'r' && x1 == 7 && y1 == 0) canCastleK[1] = false;
    if (pc == 'r' && x1 == 0 && y1 == 0) canCastleQ[1] = false;
    // Revoke if rook is captured
    if (m.captured == 'R') {
        if (x2 == 7 && y2 == 7) canCastleK[0] = false;
        if (x2 == 0 && y2 == 7) canCastleQ[0] = false;
    }
    if (m.captured == 'r') {
        if (x2 == 7 && y2 == 0) canCastleK[1] = false;
        if (x2 == 0 && y2 == 0) canCastleQ[1] = false;
    }

    // --- Finalize ---
    moveHistory.push_back(m);
    whiteToMove = !whiteToMove;
    checkGameState();
}

// —————————————————————————————————————————————————————————
// Undo last move, fully restoring rights & EP
// —————————————————————————————————————————————————————————
void Game::undoMove() {
    if (moveHistory.empty()) return;
    
    Move m = moveHistory.back();
    moveHistory.pop_back();
    whiteToMove = !whiteToMove;

    char movedPiece = m.moved;
    if (m.wasPromotion) {
        movedPiece = whiteToMove ? 'P' : 'p';
    }

    boardLogic[m.y1][m.x1] = movedPiece;
    boardLogic[m.y2][m.x2] = m.captured; // will be ' ' if not a capture

    if (m.wasEnPassant) {
        boardLogic[m.y2][m.x2] = ' '; // The landing square becomes empty again
        int capY = whiteToMove ? m.y2 + 1 : m.y2 - 1;
        boardLogic[capY][m.x2] = m.captured;
    }

    if (m.wasCastling) {
        char rook = boardLogic[m.rookY2][m.rookX2];
        boardLogic[m.rookY2][m.rookX2] = ' ';
        boardLogic[m.rookY1][m.rookX1] = rook;
    }

    // Restore castling rights & EP target
    canCastleK[0] = m.oldCastleK[0]; canCastleK[1] = m.oldCastleK[1];
    canCastleQ[0] = m.oldCastleQ[0]; canCastleQ[1] = m.oldCastleQ[1];
    enPassantTarget = m.oldEnPassant;
    mGameState = PLAYING; // Game is no longer over after undo
}

// —————————————————————————————————————————————————————————
// Check for Checkmate or Stalemate
// —————————————————————————————————————————————————————————
void Game::checkGameState() {
    if (legalMoves().empty()) {
        if (inCheck(whiteToMove)) {
            mGameState = CHECKMATE;
            mGameOverText.setString(whiteToMove ? "Checkmate! Black wins." : "Checkmate! White wins.");
        } else {
            mGameState = STALEMATE;
            mGameOverText.setString("Stalemate! It's a draw.");
        }
        // Center the text
        sf::FloatRect textRect = mGameOverText.getLocalBounds();
        mGameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        mGameOverText.setPosition(mWindow.getSize().x / 2.0f, mWindow.getSize().y / 2.0f);
    }
}