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
        
        virtual void move(Board* board, const std::pair<int, int>& to){}
        virtual bool canMoveTo(Board* board, const std::pair<int, int>& to) { return false; }
};

class Pawn : public Piece{
    public:
        Pawn(bool W) : Piece(W) {}
        void move(Board* board, const std::pair<int, int>& to) override {
            if (canMoveTo(board, to)) {
                std::pair<int, int> currentPos = board->findPieceCoordinates(this);
                if (board->isOccupied(to)) {
                    board->removePiece(to);
                }
                board->placePiece(this, to);
                board->removePiece(currentPos);
                board->moveHistory.push_back({currentPos, to, this});
            } else {
                std::cout << "Invalid move for Pawn." << std::endl;
            }
        }
        bool canMoveTo(Board* board, const std::pair<int, int>& to) override {
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

            }
            return false;
        }
};

class Knight : public Piece{
    public:
        Knight(bool W) : Piece(W) {}
        void move(Board* board, const std::pair<int, int>& to) override {
            if (canMoveTo(board, to)) {
                std::pair<int, int> currentPos = board->findPieceCoordinates(this);
                if (board->isOccupied(to)) {
                    board->removePiece(to);
                }
                board->placePiece(this, to);
                board->removePiece(currentPos);
            } else {
                std::cout << "Invalid move for Knight." << std::endl;
            }
        }
        bool canMoveTo(Board* board, const std::pair<int, int>& to) override {
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

int main(){
    
    return 0;
}