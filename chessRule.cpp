#include <iostream>
#include <vector>

// Forward declaration
class Piece;

class Board{
    public:
        Piece* board[8][8] = {{nullptr}};

        struct Move {
            std::pair<int, int> from;
            std::pair<int, int> to;
            Piece* piece;
        };
        std::vector<Move> moveHistory;        
        bool isOccupied(const std::pair<int, int>& pos) const;
        bool isOccupiedByWhite(const std::pair<int, int>& pos) const;
        void placePiece(Piece* piece, const std::pair<int, int>& pos);
        void removePiece(const std::pair<int, int>& pos);
        bool checkCheck(bool white);
        bool isLegal(const std::pair<int, int>& from, const std::pair<int, int>& to);
        bool checkCheckmate(bool white);
        bool checkStalemate(bool white);
        std::pair<int, int> findPieceCoordinates(const Piece* target) const;
};

class Piece{
    public:
        Piece(bool W) : white(W) {}
        bool white;
        
        virtual void move(Board* board, const std::pair<int, int>& to);
        virtual bool canMoveTo(const Board* board, const std::pair<int, int>& to);
};

class Pawn : public Piece{
    public:
        Pawn(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

class Knight : public Piece{
    public:
        Knight(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

class Bishop : public Piece{
    public:
        Bishop(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

class Rook : public Piece{
    public:
        Rook(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

class Queen : public Piece{
    public:
        Queen(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

class King : public Piece{
    public:
        King(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override;
};

// Board method implementations
bool Board::checkCheck(bool white) {
    // Find the king of the specified color
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            Piece* piece = board[y][x];
            if(piece != nullptr && piece->white == white && dynamic_cast<King*>(piece)) {
                std::pair<int, int> kingPos = {x, y};
                
                // Check if any enemy piece can attack the king
                for(int ey = 0; ey < 8; ey++) {
                    for(int ex = 0; ex < 8; ex++) {
                        Piece* enemyPiece = board[ey][ex];
                        if(enemyPiece != nullptr && enemyPiece->white != white) {
                            if(enemyPiece->canMoveTo(this, kingPos)) {
                                return true; // King is in check
                            }
                        }
                    }
                }
                return false; // King is safe
            }
        }
    }
    return false; // King not found (shouldn't happen in valid game)
}

bool Board::isLegal(const std::pair<int, int>& from, const std::pair<int, int>& to) {
    // Check if move is within bounds
    if(from.first < 0 || from.first > 7 || from.second < 0 || from.second > 7 ||
       to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false;
    }
    
    Piece* piece = board[from.second][from.first];
    if(piece == nullptr) {
        return false;
    }
    
    // First check if the piece can actually make this move
    if(!piece->canMoveTo(this, to)) {
        return false;
    }
    
    // Make a temporary move to check if it leaves king in check
    Piece* captured = board[to.second][to.first];
    board[to.second][to.first] = piece;
    board[from.second][from.first] = nullptr;
    
    bool kingInCheck = checkCheck(piece->white);
    
    // Restore the board
    board[from.second][from.first] = piece;
    board[to.second][to.first] = captured;
    
    return !kingInCheck; // Move is legal if king is not in check after move
}

bool Board::checkCheckmate(bool white) {
    // If king is not in check, it's not checkmate
    if(!checkCheck(white)) {
        return false;
    }
    
    // Check if any piece of this color has a legal move
    for(int fy = 0; fy < 8; fy++) {
        for(int fx = 0; fx < 8; fx++) {
            Piece* piece = board[fy][fx];
            if(piece != nullptr && piece->white == white) {
                // Try all possible destination squares
                for(int ty = 0; ty < 8; ty++) {
                    for(int tx = 0; tx < 8; tx++) {
                        if(isLegal({fx, fy}, {tx, ty})) {
                            return false; // Found a legal move, not checkmate
                        }
                    }
                }
            }
        }
    }
    return true; // No legal moves found, it's checkmate
}

bool Board::checkStalemate(bool white) {
    // If king is in check, it's not stalemate (could be checkmate)
    if(checkCheck(white)) {
        return false;
    }
    
    // Check if any piece of this color has a legal move
    for(int fy = 0; fy < 8; fy++) {
        for(int fx = 0; fx < 8; fx++) {
            Piece* piece = board[fy][fx];
            if(piece != nullptr && piece->white == white) {
                // Try all possible destination squares
                for(int ty = 0; ty < 8; ty++) {
                    for(int tx = 0; tx < 8; tx++) {
                        if(isLegal({fx, fy}, {tx, ty})) {
                            return false; // Found a legal move, not stalemate
                        }
                    }
                }
            }
        }
    }
    return true; // No legal moves found and not in check, it's stalemate
}

// Board method implementations
bool Board::isOccupied(const std::pair<int, int>& pos) const {
    return board[pos.second][pos.first] != nullptr;
}

bool Board::isOccupiedByWhite(const std::pair<int, int>& pos) const {
    return board[pos.second][pos.first] != nullptr && board[pos.second][pos.first]->white;
}

void Board::placePiece(Piece* piece, const std::pair<int, int>& pos) {
    board[pos.second][pos.first] = piece;
}

void Board::removePiece(const std::pair<int, int>& pos) {
    board[pos.second][pos.first] = nullptr;
}

std::pair<int, int> Board::findPieceCoordinates(const Piece* target) const {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            if (board[y][x] == target) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

// Piece method implementations
void Piece::move(Board* board, const std::pair<int, int>& to) {
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (board->isLegal(currentPos, to)) {
        if (board->isOccupied(to)) {
            board->removePiece(to);
        }
        board->placePiece(this, to);
        board->removePiece(currentPos);
        board->moveHistory.push_back({currentPos, to, this});
    } else {
        std::cout << "Invalid move." << std::endl;
    }
}

bool Piece::canMoveTo(const Board* board, const std::pair<int, int>& to) { 
    return false; 
}

// Pawn method implementation
bool Pawn::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }
    // Pawn forward move
    if(currentPos.first == to.first) {
        if(to.second == currentPos.second + (white ? 1 : -1)) {
            if (!board->isOccupied(to)) {
                return true;
            }
        }
        if (to.second == currentPos.second + (white ? 2 : -2) && currentPos.second == (white ? 1 : 6)) {
            if (!board->isOccupied(to) && !board->isOccupied({currentPos.first, currentPos.second + (white ? 1 : -1)})) {
                return true;
            }
        }
    }
    // Pawn capture
    if ((to.first == currentPos.first + 1 || to.first == currentPos.first - 1) &&
        to.second == currentPos.second + (white ? 1 : -1)) {
        if (board->isOccupied(to) && board->isOccupiedByWhite(to) != white) {
            return true;
        }
        //En passant
        if (dynamic_cast<Pawn*> (board->moveHistory.back().piece)&&
            board->moveHistory.back().from == std::make_pair(to.first, to.second + (white ? 1 : -1)) &&
            board->moveHistory.back().to == std::make_pair(to.first, to.second - (white ? 1 : -1))){
            return true;
        }
    }
    return false;
}

// Knight method implementation
bool Knight::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }    
    if ((abs(to.first - currentPos.first) == 2 && abs(to.second - currentPos.second) == 1) ||
        (abs(to.first - currentPos.first) == 1 && abs(to.second - currentPos.second) == 2)) {
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    return false;
}

// Bishop method implementation
bool Bishop::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }
    if ((abs(to.first - currentPos.first) == abs(to.second - currentPos.second)) &&
        (to.first != currentPos.first || to.second != currentPos.second)) {
        int xStep = (to.first - currentPos.first > 0) ? 1 : -1;
        int yStep = (to.second - currentPos.second > 0) ? 1 : -1;
        for (int i = 1; i < abs(to.first - currentPos.first); ++i) {
            if (board->isOccupied({currentPos.first + i * xStep, currentPos.second + i * yStep})) {
                return false; // Path is blocked
            }
        }
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    return false;
}

// Rook method implementation
bool Rook::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }
    if ((to.first == currentPos.first) != (to.second == currentPos.second)) {
        int xStep = (to.first - currentPos.first > 0) ? 1 : (to.first - currentPos.first < 0 ? -1 : 0);
        int yStep = (to.second - currentPos.second > 0) ? 1 : (to.second - currentPos.second < 0 ? -1 : 0);
        for (int i = 1; i < std::max(abs(to.first - currentPos.first), abs(to.second - currentPos.second)); ++i) {
            if (board->isOccupied({currentPos.first + i * xStep, currentPos.second + i * yStep})) {
                return false; // Path is blocked
            }
        }
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    return false;
}

// Queen method implementation
bool Queen::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }
    if (((abs(to.first - currentPos.first) == abs(to.second - currentPos.second)) ||
        (to.first == currentPos.first || to.second == currentPos.second)) &&
        (to.first != currentPos.first || to.second != currentPos.second)) {
        int xStep = (to.first - currentPos.first > 0) ? 1 : (to.first - currentPos.first < 0 ? -1 : 0);
        int yStep = (to.second - currentPos.second > 0) ? 1 : (to.second - currentPos.second < 0 ? -1 : 0);
        for (int i = 1; i < std::max(abs(to.first - currentPos.first), abs(to.second - currentPos.second)); ++i) {
            if (board->isOccupied({currentPos.first + i * xStep, currentPos.second + i * yStep})) {
                return false; // Path is blocked
            }
        }
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    return false;
}

// King method implementation
bool King::canMoveTo(const Board* board, const std::pair<int, int>& to) {
    if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
        return false; 
    }
    std::pair<int, int> currentPos = board->findPieceCoordinates(this);
    if (currentPos.first == -1) {
        return false;
    }
    // King can move one square in any direction
    if (abs(to.first - currentPos.first) <= 1 && abs(to.second - currentPos.second) <= 1 &&
        (to.first != currentPos.first || to.second != currentPos.second)) {
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    return false;
}

int main(){
    
    return 0;
}