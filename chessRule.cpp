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
        
        // Store complete board states after each move for accurate position comparison
        struct BoardState {
            Piece* board[8][8];
            // Additional game state information for threefold repetition
            bool whiteCanCastleKingside;
            bool whiteCanCastleQueenside;
            bool blackCanCastleKingside;
            bool blackCanCastleQueenside;
            std::pair<int, int> enPassantTarget; // {-1, -1} if no en passant possible
        };
        std::vector<BoardState> boardHistory;
              
        bool isOccupied(const std::pair<int, int>& pos) const;
        bool isOccupiedByWhite(const std::pair<int, int>& pos) const;
        void placePiece(Piece* piece, const std::pair<int, int>& pos);
        void removePiece(const std::pair<int, int>& pos);        
        void movePiece(const std::pair<int, int>& from, const std::pair<int, int>& to);
        void promotePawn(const std::pair<int, int>& pos, char pieceType);
        bool isCheck(bool white) const;
        bool isLegal(const std::pair<int, int>& from, const std::pair<int, int>& to);
        bool isCheckmate(bool white);
        bool isDrawByStalemate(bool white);        
        bool isDrawByRepetition() const;
        bool isDrawByFiftyMoves() const; 
        bool isDrawByInsufficientMaterial() const; 
        std::pair<int, int> findPieceCoordinates(const Piece* target) const;
        bool hasPieceMoved(const Piece* piece) const;
        void saveBoardState();
        BoardState getCurrentBoardState() const;
        void initializeBoardHistory(); // Save initial board state
    private:
        bool isSamePosition(int moveIndex1, int moveIndex2) const;
};

class Piece{
    public:
        Piece(bool W) : white(W) {}
        bool white;
        
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
bool Board::isCheck(bool white) const {
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
    
    bool kingInCheck = isCheck(piece->white);
    
    // Restore the board
    board[from.second][from.first] = piece;
    board[to.second][to.first] = captured;
    
    return !kingInCheck; // Move is legal if king is not in check after move
}

bool Board::isCheckmate(bool white) {
    // If king is not in check, it's not checkmate
    if(!isCheck(white)) {
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

bool Board::isDrawByStalemate(bool white) {
    // If king is in check, it's not stalemate (could be checkmate)
    if(isCheck(white)) {
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

bool Board::isDrawByRepetition() const {
    if (boardHistory.size() < 8) { // Need at least 4 full moves (8 half-moves) for threefold repetition
        return false;
    }
    
    // Count how many times the current position has occurred
    int repetitionCount = 1; // Current position counts as 1
    int currentBoardIndex = boardHistory.size() - 1;
    
    // Helper function to check if a move is a king castling move
    auto isKingCastlingMove = [&](int idx) -> bool {
        if (idx >= moveHistory.size()) return false;
        const Move& move = moveHistory[idx];
        return dynamic_cast<King*>(move.piece) && abs(move.to.first - move.from.first) == 2;
    };
    
    // Function to count actual player moves (considering castling as one move)
    auto countPlayerMoves = [&](int fromIndex, int toIndex) -> int {
        int playerMoves = 0;
        for (int i = fromIndex; i <= toIndex; i++) {
            if (isKingCastlingMove(i)) {
                playerMoves++; // Castling counts as one player move
                i++; // Skip the rook move that follows
            } else {
                playerMoves++; // Regular move
            }
        }
        return playerMoves;
    };
    
    // Current position player moves count
    int currentPlayerMoves = countPlayerMoves(0, moveHistory.size() - 1);
    
    // Start from previous board states and go back, looking for positions with same player to move
    for (int i = boardHistory.size() - 2; i >= 0; i--) {
        // Count player moves up to position i
        int playerMovesToI = countPlayerMoves(0, i);
        
        // Check if we have the same player to move by comparing parity
        if (currentPlayerMoves % 2 == playerMovesToI % 2) {
            if (isSamePosition(currentBoardIndex, i)) {
                repetitionCount++;
                if (repetitionCount >= 3) {
                    return true; // Threefold repetition
                }
            }
        }
    }
    
    return false;
}

bool Board::isSamePosition(int moveIndex1, int moveIndex2) const {
    if (moveIndex1 < 0 || moveIndex2 < 0 || 
        moveIndex1 >= boardHistory.size() || moveIndex2 >= boardHistory.size()) {
        return false;
    }
    
    // If both indices point to the same position, they're obviously the same
    if (moveIndex1 == moveIndex2) {
        return true;
    }
    
    const BoardState& state1 = boardHistory[moveIndex1];
    const BoardState& state2 = boardHistory[moveIndex2];
    
    // Compare piece positions on all squares
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Piece* piece1 = state1.board[y][x];
            Piece* piece2 = state2.board[y][x];
            
            // Compare pieces at this position
            if (piece1 == nullptr && piece2 == nullptr) {
                continue; // Both empty, ok
            }
            
            if (piece1 == nullptr || piece2 == nullptr) {
                return false; // One empty, one occupied
            }
            
            // Both positions have pieces - compare their types and colors
            if (piece1->white != piece2->white) {
                return false; // Different colors
            }
            
            // Check piece types by comparing dynamic casts
            if ((dynamic_cast<Pawn*>(piece1) != nullptr) != (dynamic_cast<Pawn*>(piece2) != nullptr) ||
                (dynamic_cast<Knight*>(piece1) != nullptr) != (dynamic_cast<Knight*>(piece2) != nullptr) ||
                (dynamic_cast<Bishop*>(piece1) != nullptr) != (dynamic_cast<Bishop*>(piece2) != nullptr) ||
                (dynamic_cast<Rook*>(piece1) != nullptr) != (dynamic_cast<Rook*>(piece2) != nullptr) ||
                (dynamic_cast<Queen*>(piece1) != nullptr) != (dynamic_cast<Queen*>(piece2) != nullptr) ||
                (dynamic_cast<King*>(piece1) != nullptr) != (dynamic_cast<King*>(piece2) != nullptr)) {
                return false; // Different piece types
            }
        }
    }
    
    // Compare castling rights (essential for threefold repetition)
    if (state1.whiteCanCastleKingside != state2.whiteCanCastleKingside ||
        state1.whiteCanCastleQueenside != state2.whiteCanCastleQueenside ||
        state1.blackCanCastleKingside != state2.blackCanCastleKingside ||
        state1.blackCanCastleQueenside != state2.blackCanCastleQueenside) {
        return false; // Different castling rights
    }
    
    // Compare en passant target squares
    if (state1.enPassantTarget != state2.enPassantTarget) {
        return false; // Different en passant possibilities
    }
    
    return true; // Positions are identical
}

bool Board::hasPieceMoved(const Piece* piece) const {
    // Check if the piece appears in move history
    for (const Move& move : moveHistory) {
        if (move.piece == piece) {
            return true;
        }
    }
    return false;
}

bool Board::isDrawByFiftyMoves() const {
    if (moveHistory.size() < 100) { // 50 moves by each side = 100 half-moves
        return false;
    }
    
    // Helper function to check if a move is a king castling move
    auto isKingCastlingMove = [&](int idx) -> bool {
        if (idx >= moveHistory.size() || idx < 0) return false;
        const Move& move = moveHistory[idx];
        return dynamic_cast<King*>(move.piece) && abs(move.to.first - move.from.first) == 2;
    };
    
    // Helper function to count pieces on a board state
    auto countPieces = [](const BoardState& state) -> int {
        int count = 0;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (state.board[y][x] != nullptr) {
                    count++;
                }
            }
        }
        return count;
    };
    
    // Count moves since last pawn move or capture
    int movesSinceLastPawnMoveOrCapture = 0;
    
    // Go through move history from most recent backwards
    for (int i = moveHistory.size() - 1; i >= 0; i--) {
        const Move& move = moveHistory[i];
        
        // Check if this was a pawn move
        if (dynamic_cast<Pawn*>(move.piece)) {
            break; // Found pawn move, reset counter
        }
        
        // Check if this was a capture by comparing piece counts
        // boardHistory[i] is the state before move i, boardHistory[i+1] is after move i
        if (i + 1 < boardHistory.size() && i >= 0) {
            int piecesBefore = countPieces(boardHistory[i]);
            int piecesAfter = countPieces(boardHistory[i + 1]);
            
            if (piecesBefore > piecesAfter) {
                break; // Found capture, reset counter
            }
        }        
        // Handle castling properly - count as one move, not two
        // When going backwards through move history, castling appears as:
        // - Rook move (at higher index, encountered first when going backwards)
        // - King move (at lower index, encountered second when going backwards)
        
        if (isKingCastlingMove(i - 1)) {
            movesSinceLastPawnMoveOrCapture++;
            i--; // Skip the rook move that follows castling
        } else {
            movesSinceLastPawnMoveOrCapture++;
        }
        
        // If we've counted 100 half-moves (50 full moves) without finding a pawn move or capture,
        // it's a draw by fifty-move rule
        if (movesSinceLastPawnMoveOrCapture >= 100) {
            return true;
        }
    }
    
    return false;
}

bool Board::isDrawByInsufficientMaterial() const {
    // Count pieces on the board
    int whiteKnights = 0, blackKnights = 0;
    int whiteBishops = 0, blackBishops = 0;
    int whiteRooks = 0, blackRooks = 0;
    int whiteQueens = 0, blackQueens = 0;
    int whitePawns = 0, blackPawns = 0;
    bool whiteKing = false, blackKing = false;
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Piece* piece = board[y][x];
            if (piece != nullptr) {
                if (dynamic_cast<King*>(piece)) {
                    if (piece->white) whiteKing = true;
                    else blackKing = true;
                } else if (dynamic_cast<Queen*>(piece)) {
                    if (piece->white) whiteQueens++;
                    else blackQueens++;
                } else if (dynamic_cast<Rook*>(piece)) {
                    if (piece->white) whiteRooks++;
                    else blackRooks++;
                } else if (dynamic_cast<Bishop*>(piece)) {
                    if (piece->white) whiteBishops++;
                    else blackBishops++;
                } else if (dynamic_cast<Knight*>(piece)) {
                    if (piece->white) whiteKnights++;
                    else blackKnights++;
                } else if (dynamic_cast<Pawn*>(piece)) {
                    if (piece->white) whitePawns++;
                    else blackPawns++;
                }
            }
        }
    }
    
    // Check for insufficient material combinations
    
    // King vs King
    if (whiteQueens == 0 && blackQueens == 0 && 
        whiteRooks == 0 && blackRooks == 0 && 
        whitePawns == 0 && blackPawns == 0 &&
        whiteBishops == 0 && blackBishops == 0 &&
        whiteKnights == 0 && blackKnights == 0) {
        return true;
    }
    
    // King and Bishop vs King
    if (whiteQueens == 0 && blackQueens == 0 && 
        whiteRooks == 0 && blackRooks == 0 && 
        whitePawns == 0 && blackPawns == 0 &&
        whiteKnights == 0 && blackKnights == 0 &&
        ((whiteBishops == 1 && blackBishops == 0) || 
         (whiteBishops == 0 && blackBishops == 1))) {
        return true;
    }
    
    // King and Knight vs King
    if (whiteQueens == 0 && blackQueens == 0 && 
        whiteRooks == 0 && blackRooks == 0 && 
        whitePawns == 0 && blackPawns == 0 &&
        whiteBishops == 0 && blackBishops == 0 &&
        ((whiteKnights == 1 && blackKnights == 0) || 
         (whiteKnights == 0 && blackKnights == 1))) {
        return true;
    }
    
    // King and Bishop vs King and Bishop (same color squares)
    if (whiteQueens == 0 && blackQueens == 0 && 
        whiteRooks == 0 && blackRooks == 0 && 
        whitePawns == 0 && blackPawns == 0 &&
        whiteKnights == 0 && blackKnights == 0 &&
        whiteBishops == 1 && blackBishops == 1) {
        
        // Find the bishops and check if they're on same color squares
        std::pair<int, int> whiteBishopPos = {-1, -1};
        std::pair<int, int> blackBishopPos = {-1, -1};
        
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Piece* piece = board[y][x];
                if (piece != nullptr && dynamic_cast<Bishop*>(piece)) {
                    if (piece->white) {
                        whiteBishopPos = {x, y};
                    } else {
                        blackBishopPos = {x, y};
                    }
                }
            }
        }
        
        // Check if bishops are on squares of the same color
        // A square is light if (x + y) is even, dark if odd
        if (whiteBishopPos.first != -1 && blackBishopPos.first != -1) {
            bool whiteBishopOnLight = (whiteBishopPos.first + whiteBishopPos.second) % 2 == 0;
            bool blackBishopOnLight = (blackBishopPos.first + blackBishopPos.second) % 2 == 0;
            
            if (whiteBishopOnLight == blackBishopOnLight) {
                return true; // Both bishops on same color squares
            }
        }
    }
    
    return false; // Material is sufficient for checkmate
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

void Board::movePiece(const std::pair<int, int>& from, const std::pair<int, int>& to) {
    if (isLegal(from, to)) {
        Piece* piece = board[from.second][from.first];
        
        // Check if this is a castling move
        if (dynamic_cast<King*>(piece) && abs(to.first - from.first) == 2) {
            // This is a castling move
            bool isKingside = (to.first == 6);
            int rookFromX = isKingside ? 7 : 0;
            int rookToX = isKingside ? 5 : 3;
            int row = from.second;
            
            // Move the rook
            Piece* rook = board[row][rookFromX];
            board[row][rookToX] = rook;
            board[row][rookFromX] = nullptr;
            
            // Move the king
            board[to.second][to.first] = piece;
            board[from.second][from.first] = nullptr;
              // Record both moves in history
            moveHistory.push_back({from, to, piece});
            moveHistory.push_back({{rookFromX, row}, {rookToX, row}, rook});
            
            // Save board state after castling
            saveBoardState();
        } else {
            // Check for en passant capture before regular move
            bool isEnPassant = false;
            std::pair<int, int> capturedPawnPos = {-1, -1};
            
            if (dynamic_cast<Pawn*>(piece)) {
                // Check if this is an en passant capture
                if ((to.first == from.first + 1 || to.first == from.first - 1) &&
                    to.second == from.second + (piece->white ? 1 : -1) &&
                    !isOccupied(to)) {
                    
                    // Check if the last move was a pawn moving two squares
                    if (!moveHistory.empty()) {
                        const Move& lastMove = moveHistory.back();
                        if (dynamic_cast<Pawn*>(lastMove.piece) &&
                            lastMove.from == std::make_pair(to.first, to.second + (piece->white ? 1 : -1)) &&
                            lastMove.to == std::make_pair(to.first, to.second - (piece->white ? 1 : -1))) {
                            isEnPassant = true;
                            capturedPawnPos = {to.first, to.second - (piece->white ? 1 : -1)};
                        }
                    }
                }
            }
            
            // Regular move or en passant
            if (isEnPassant) {
                // Remove the captured pawn
                board[capturedPawnPos.second][capturedPawnPos.first] = nullptr;
            }
            board[to.second][to.first] = piece;
            board[from.second][from.first] = nullptr;
            moveHistory.push_back({from, to, piece});
            
            // Check for pawn promotion
            if (dynamic_cast<Pawn*>(piece)) {
                // White pawn reaches rank 8 (index 7) or black pawn reaches rank 1 (index 0)
                if ((piece->white && to.second == 7) || (!piece->white && to.second == 0)) {
                    // For now, auto-promote to Queen. In a real game, player would choose.
                    promotePawn(to, 'Q');
                    std::cout << "Pawn promoted to Queen!" << std::endl;
                }
            }
            
            // Save board state after regular move or en passant
            saveBoardState();
        }
    } else {
        std::cout << "Invalid move." << std::endl;    
    }
}

void Board::promotePawn(const std::pair<int, int>& pos, char pieceType) {
    Piece* pawn = board[pos.second][pos.first];
    if (pawn && dynamic_cast<Pawn*>(pawn)) {
        bool isWhite = pawn->white;
        
        // Create new piece based on promotion choice
        Piece* newPiece = nullptr;
        switch (pieceType) {
            case 'Q': case 'q':
                newPiece = new Queen(isWhite);
                break;
            case 'R': case 'r':
                newPiece = new Rook(isWhite);
                break;
            case 'B': case 'b':
                newPiece = new Bishop(isWhite);
                break;
            case 'N': case 'n':
                newPiece = new Knight(isWhite);
                break;
            default:
                newPiece = new Queen(isWhite); // Default to Queen
                break;
        }
        
        board[pos.second][pos.first] = newPiece;
    }
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
    
    // Regular king move (one square in any direction)
    if (abs(to.first - currentPos.first) <= 1 && abs(to.second - currentPos.second) <= 1 &&
        (to.first != currentPos.first || to.second != currentPos.second)) {
        // Can move if destination is empty or occupied by opponent
        if (!board->isOccupied(to) || (board->isOccupied(to) && board->isOccupiedByWhite(to) != white)) {
            return true;
        }
    }
    
    // Castling logic
    if (currentPos.second == to.second && abs(to.first - currentPos.first) == 2) {
        // Check if king is on starting position
        int kingStartRow = white ? 0 : 7;
        if (currentPos.second != kingStartRow || currentPos.first != 4) {
            return false; // King not on starting position
        }
        
        // Check if king has moved
        if (board->hasPieceMoved(this)) {
            return false; // King has already moved
        }
        
        // Check if king is in check
        if (board->isCheck(white)) {
            return false; // Cannot castle while in check
        }
        
        // Determine if kingside or queenside castling
        bool isKingside = (to.first == 6);
        int rookX = isKingside ? 7 : 0;
        
        // Check if rook exists and hasn't moved
        Piece* rook = board->board[kingStartRow][rookX];
        if (rook == nullptr || rook->white != white || !dynamic_cast<Rook*>(rook)) {
            return false; // No rook or wrong color
        }
        
        if (board->hasPieceMoved(rook)) {
            return false; // Rook has already moved
        }
        
        // Check if path is clear between king and rook
        int startX = std::min(currentPos.first, rookX);
        int endX = std::max(currentPos.first, rookX);
        for (int x = startX + 1; x < endX; x++) {
            if (board->isOccupied({x, kingStartRow})) {
                return false; // Path is blocked
            }
        }
        
        // Check if king passes through or ends in check
        int direction = isKingside ? 1 : -1;
        for (int step = 1; step <= 2; step++) {
            std::pair<int, int> intermediatePos = {currentPos.first + step * direction, currentPos.second};
            
            // Temporarily move king to check for check
            const_cast<Board*>(board)->board[currentPos.second][currentPos.first] = nullptr;
            const_cast<Board*>(board)->board[intermediatePos.second][intermediatePos.first] = const_cast<King*>(this);
            
            bool inCheck = board->isCheck(white);
            
            // Restore king position
            const_cast<Board*>(board)->board[intermediatePos.second][intermediatePos.first] = nullptr;
            const_cast<Board*>(board)->board[currentPos.second][currentPos.first] = const_cast<King*>(this);
            
            if (inCheck) {
                return false; // King passes through or ends in check
            }
        }
        
        return true; // Castling is legal
    }
    
    return false;
}

void Board::saveBoardState() {
    boardHistory.push_back(getCurrentBoardState());
}

Board::BoardState Board::getCurrentBoardState() const {
    BoardState state;
    
    // Copy current board position
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            state.board[y][x] = board[y][x];
        }
    }
    
    // Determine castling rights based on whether kings and rooks have moved
    // Find kings and rooks in their starting positions
    Piece* whiteKing = board[0][4];
    Piece* blackKing = board[7][4];
    Piece* whiteKingsideRook = board[0][7];
    Piece* whiteQueensideRook = board[0][0];
    Piece* blackKingsideRook = board[7][7];
    Piece* blackQueensideRook = board[7][0];
    
    // Check if pieces are in original positions and haven't moved
    state.whiteCanCastleKingside = (whiteKing && dynamic_cast<King*>(whiteKing) && whiteKing->white &&
                                   whiteKingsideRook && dynamic_cast<Rook*>(whiteKingsideRook) && whiteKingsideRook->white &&
                                   !hasPieceMoved(whiteKing) && !hasPieceMoved(whiteKingsideRook));
    
    state.whiteCanCastleQueenside = (whiteKing && dynamic_cast<King*>(whiteKing) && whiteKing->white &&
                                    whiteQueensideRook && dynamic_cast<Rook*>(whiteQueensideRook) && whiteQueensideRook->white &&
                                    !hasPieceMoved(whiteKing) && !hasPieceMoved(whiteQueensideRook));
    
    state.blackCanCastleKingside = (blackKing && dynamic_cast<King*>(blackKing) && !blackKing->white &&
                                   blackKingsideRook && dynamic_cast<Rook*>(blackKingsideRook) && !blackKingsideRook->white &&
                                   !hasPieceMoved(blackKing) && !hasPieceMoved(blackKingsideRook));
    
    state.blackCanCastleQueenside = (blackKing && dynamic_cast<King*>(blackKing) && !blackKing->white &&
                                    blackQueensideRook && dynamic_cast<Rook*>(blackQueensideRook) && !blackQueensideRook->white &&
                                    !hasPieceMoved(blackKing) && !hasPieceMoved(blackQueensideRook));
    
    // Determine en passant target square
    state.enPassantTarget = {-1, -1}; // Default: no en passant
    
    if (!moveHistory.empty()) {
        const Move& lastMove = moveHistory.back();
        
        // Check if last move was a pawn moving two squares
        if (dynamic_cast<Pawn*>(lastMove.piece) && 
            abs(lastMove.to.second - lastMove.from.second) == 2) {
            
            // En passant target is the square the pawn passed over
            int targetY = (lastMove.from.second + lastMove.to.second) / 2;
            state.enPassantTarget = {lastMove.to.first, targetY};
        }
    }
    
    return state;
}

void Board::initializeBoardHistory() {
    // Save the initial board state (should be called after setting up starting position)
    saveBoardState();
}