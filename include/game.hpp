#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <set>
#include <map>
#include <string>

// Encapsulates every detail needed to undo a move
struct Move {
    int x1, y1, x2, y2;
    char moved, captured;
    // Special flags:
    bool wasEnPassant = false, wasCastling = false, wasPromotion = false;
    // Castling rook move:
    int rookX1, rookY1, rookX2, rookY2;
    // Stored rights and EP square before move:
    bool oldCastleK[2], oldCastleQ[2];
    std::pair<int, int> oldEnPassant;
};

class Game {
public:
    Game();
    void run();
    void undoMove();

private:
    // --- SFML UI ---
    sf::RenderWindow                  mWindow;
    sf::RectangleShape                board[64];
    sf::RectangleShape                highlight;
    std::map<std::string, sf::Texture> pieceTextures;
    sf::Sprite                        pieceSprites[8][8];
    sf::Sprite                        draggedSprite;
    sf::Font                          mFont;
    sf::Text                          mGameOverText;

    // --- Game State ---
    enum GameState { PLAYING, CHECKMATE, STALEMATE };
    GameState   mGameState = PLAYING;
    char        boardLogic[8][8];
    bool        whiteToMove = true;
    bool        canCastleK[2] = {true, true};   // [0]=white, [1]=black
    bool        canCastleQ[2] = {true, true};
    std::pair<int, int> enPassantTarget = {-1, -1};
    std::vector<Move> moveHistory;
    std::set<std::vector<int>> lastLegalMoves;

    // Drag & highlight
    bool         isPieceSelected = false, isDragging = false;
    sf::Vector2i selectedSquare;

    // Core loop
    void processEvent();
    void update();
    void render();

    // Load & draw
    void loadTextures();
    void initSprites();

    // Move generation & execution
    std::set<std::vector<int>> pseudoLegalMoves(bool white) const;
    std::set<std::vector<int>> legalMoves();
    bool inCheck(bool white) const;
    bool isSquareAttacked(int x, int y, bool byWhite) const;
    void handleMove(int x1, int y1, int x2, int y2);
    void checkGameState();

    // Helpers
    inline bool inBounds(int x, int y) const { return x >= 0 && x < 8 && y >= 0 && y < 8; }
    inline bool isEmpty(int x, int y) const {
        return inBounds(x, y) && boardLogic[y][x] == ' ';
    }
    inline bool sameColor(char a, char b) const {
        if (a == ' ' || b == ' ') return false;
        return (isupper(a) && isupper(b)) || (islower(a) && islower(b));
    }
    inline bool isWhite(char p) const { return isupper(p); }
    inline bool isBlack(char p) const { return islower(p); }
    inline bool isOpponent(int x, int y, bool white) const {
        if (!inBounds(x, y)) return false;
        char t = boardLogic[y][x];
        return t != ' ' && (white ? islower(t) : isupper(t));
    }
};