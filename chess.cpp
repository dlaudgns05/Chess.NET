#include <iostream>
#include <vector>

class Board{
    public:
        Piece* board[8][8] = {{nullptr}};

        struct Move {
            std::pair<int, int> from;
            std::pair<int, int> to;
            Piece* piece;
        };
        std::vector<Move> moveHistory;

        bool isOccupied(const std::pair<int, int>& pos) const {
            return board[pos.second][pos.first] != nullptr;
        }

        bool isOccupiedByWhite(const std::pair<int, int>& pos) const {
            return board[pos.second][pos.first] != nullptr && board[pos.second][pos.first]->white;
        }

        void placePiece(Piece* piece, const std::pair<int, int>& pos) {
            board[pos.second][pos.first] = piece;
        }

        void removePiece(const std::pair<int, int>& pos) {
            board[pos.second][pos.first] = nullptr;
        }

        std::pair<int, int> findPieceCoordinates(const Piece* target) const {
            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    if (board[y][x] == target) {
                        return {x, y};
                    }
                }
            }
            return {-1, -1};
        }
};

class Piece{
    public:
        Piece(bool W) : white(W) {}
        bool white;
        
        virtual void move(Board* board, const std::pair<int, int>& to) {
            if (canMoveTo(board, to)) {
                std::pair<int, int> currentPos = board->findPieceCoordinates(this);
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
        virtual bool canMoveTo(const Board* board, const std::pair<int, int>& to) { return false; }
};

class Pawn : public Piece{
    public:
        Pawn(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
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
};

class Knight : public Piece{
    public:
        Knight(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
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
                if (!(board->isOccupiedByWhite(to) == white)) {
                    return true;
                }
            }
            return false;
        }
};

class Bishop : public Piece{
    public:
        Bishop(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
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
                if (!(board->isOccupiedByWhite(to) == white)) {
                    return true;
                }
            }
            return false;
        }
};

class Rook : public Piece{
    public:
        Rook(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
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
                if (!(board->isOccupiedByWhite(to) == white)) {
                    return true;
                }
            }
            return false;
        }
};

class Queen : public Piece{
    public:
        Queen(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
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
                if (!(board->isOccupiedByWhite(to) == white)) {
                    return true;
                }
            }
            return false;
        }
};

class King : public Piece{
    public:
        King(bool W) : Piece(W) {}
        bool canMoveTo(const Board* board, const std::pair<int, int>& to) override {
            if(to.first < 0 || to.first > 7 || to.second < 0 || to.second > 7) {
                return false; 
            }
            std::pair<int, int> currentPos = board->findPieceCoordinates(this);
            if (currentPos.first == -1) {
                return false;
            }
            return false;
        }
};

int main(){
    
    return 0;
}