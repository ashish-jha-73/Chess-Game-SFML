#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <set>
#include <map>
#include <string>

// Constants for game configuration
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 950;
constexpr int MENU_BAR_HEIGHT = 150;
constexpr int BOARD_SIZE = 8;
constexpr int SQUARE_SIZE = WINDOW_WIDTH / BOARD_SIZE;
constexpr int WHITE_PAWN_START_ROW = 6;
constexpr int BLACK_PAWN_START_ROW = 1;
constexpr int WHITE_BACK_ROW = 7;
constexpr int BLACK_BACK_ROW = 0;

enum class GameState {
    Menu,
    Playing,
    GameOver
};

struct Move {
    int x1, y1, x2, y2;
    char moved, captured;
    bool wasEnPassant = false, wasCastling = false, wasPromotion = false;
    int rookX1, rookY1, rookX2, rookY2;
    bool oldCastleK[2], oldCastleQ[2];
    std::pair<int, int> oldEnPassant;
};

class Game {
public:
    Game();
    void run();
    void undoMove();
    void newGame();

private:
    sf::RenderWindow mWindow;
    sf::RectangleShape board[BOARD_SIZE * BOARD_SIZE];
    sf::RectangleShape highlight;
    std::map<std::string, sf::Texture> pieceTextures;
    sf::Sprite pieceSprites[BOARD_SIZE][BOARD_SIZE];
    sf::Sprite draggedSprite;

    // UI Elements
    sf::Font font;
    sf::Text titleText;
    sf::Text statusText;
    sf::Text gameOverText;
    sf::RectangleShape menuBar;
    sf::RectangleShape newGameButton;
    sf::RectangleShape undoButton;
    sf::RectangleShape exitButton;
    sf::Text newGameText;
    sf::Text undoText;
    sf::Text exitText;

    char boardLogic[BOARD_SIZE][BOARD_SIZE];
    bool whiteToMove = true;
    bool canCastleK[2] = {true, true};
    bool canCastleQ[2] = {true, true};
    std::pair<int, int> enPassantTarget = {-1, -1};
    std::vector<Move> moveHistory;
    std::set<std::vector<int>> lastLegalMoves;
    bool isPieceSelected = false, isDragging = false;
    sf::Vector2i selectedSquare;
    GameState gameState = GameState::Menu;

    void processEvent();
    void update();
    void render();
    void loadTextures();
    void initSprites();
    std::set<std::vector<int>> pseudoLegalMoves(bool white) const;
    std::set<std::vector<int>> legalMoves();
    bool inCheck(bool white) const;
    bool isSquareAttacked(int x, int y, bool byWhite) const;
    void handleMoves(int x1, int y1, int x2, int y2);
    void setupUI();
    void checkGameState();

    inline bool inBounds(int x, int y) const { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }
    inline bool isEmpty(int x, int y) const { return inBounds(x, y) && boardLogic[y][x] == ' '; }
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