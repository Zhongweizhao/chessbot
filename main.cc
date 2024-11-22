#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "./chess.h"

using namespace chess;

constexpr int INF = 9999999;
constexpr int NINF = -INF;

static constexpr int PAWN_SQ_VALUE[64] = {
    0,  0,  0,   0,   0,   0,   0,  0,   //
    50, 50, 50,  50,  50,  50,  50, 50,  //
    10, 10, 20,  30,  30,  20,  10, 10,  //
    5,  5,  10,  25,  25,  10,  5,  5,   //
    0,  0,  0,   20,  20,  0,   0,  0,   //
    5,  -5, -10, 0,   0,   -10, -5, 5,   //
    5,  10, 10,  -40, -40, 10,  10, 5,   //
    0,  0,  0,   0,   0,   0,   0,  0};

static constexpr int KNIGHT_SQ_VALUE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,  //
    -40, -20, 0,   0,   0,   0,   -20, -40,  //
    -30, 0,   10,  15,  15,  10,  0,   -30,  //
    -30, 5,   15,  20,  20,  15,  5,   -30,  //
    -30, 0,   15,  20,  20,  15,  0,   -30,  //
    -30, 5,   10,  15,  15,  10,  5,   -30,  //
    -40, -20, 0,   5,   5,   0,   -20, -40,  //
    -50, -40, -30, -30, -30, -30, -40, -50,
};

static constexpr int BISHOP_SQ_VALUE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,  //
    -10, 0,   0,   0,   0,   0,   0,   -10,  //
    -10, 0,   5,   10,  10,  5,   0,   -10,  //
    -10, 5,   5,   10,  10,  5,   5,   -10,  //
    -10, 0,   10,  10,  10,  10,  0,   -10,  //
    -10, 10,  10,  10,  10,  10,  10,  -10,  //
    -10, 5,   0,   0,   0,   0,   5,   -10,  //
    -20, -10, -10, -10, -10, -10, -10, -20,
};

static constexpr int ROOK_SQ_VALUE[64] = {0,  0,  0,  0,  0,  0,  0,  0,   //
                                          5,  10, 10, 10, 10, 10, 10, 5,   //
                                          -5, 0,  0,  0,  0,  0,  0,  -5,  //
                                          -5, 0,  0,  0,  0,  0,  0,  -5,  //
                                          -5, 0,  0,  0,  0,  0,  0,  -5,  //
                                          -5, 0,  0,  0,  0,  0,  0,  -5,  //
                                          -5, 0,  0,  0,  0,  0,  0,  -5,  //
                                          0,  0,  0,  5,  5,  0,  0,  0};

static constexpr int QUEEN_SQ_VALUE[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,  //
    -10, 0,   0,   0,  0,  0,   0,   -10,  //
    -10, 0,   5,   5,  5,  5,   0,   -10,  //
    -5,  0,   5,   5,  5,  5,   0,   -5,   //
    0,   0,   5,   5,  5,  5,   0,   -5,   //
    -10, 5,   5,   5,  5,  5,   0,   -10,  //
    -10, 0,   5,   0,  0,  0,   0,   -10,  //
    -20, -10, -10, -5, -5, -10, -10, -20};

static constexpr int KING_OPENING_SQ_VALUE[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,  //
    -30, -40, -40, -50, -50, -40, -40, -30,  //
    -30, -40, -40, -50, -50, -40, -40, -30,  //
    -30, -40, -40, -50, -50, -40, -40, -30,  //
    -20, -30, -30, -40, -40, -30, -30, -20,  //
    -10, -20, -20, -20, -20, -20, -20, -10,  //
    20,  20,  0,   0,   0,   0,   20,  20,   //
    20,  30,  10,  0,   0,   10,  30,  20};

static constexpr int KING_ENDGAME_SQ_VALUE[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,  //
    -30, -20, -10, 0,   0,   -10, -20, -30,  //
    -30, -10, 20,  30,  30,  20,  -10, -30,  //
    -30, -10, 30,  40,  40,  30,  -10, -30,  //
    -30, -10, 30,  40,  40,  30,  -10, -30,  //
    -30, -10, 20,  30,  30,  20,  -10, -30,  //
    -30, -30, 0,   0,   0,   0,   -30, -30,  //
    -50, -30, -30, -30, -30, -30, -30, -50};

static std::map<PackedBoard, int> board_repetition = {};

// Move score from attacker to victim
// PAWN KNIGHT BISHOP ROOK QUEEN KING
static constexpr int MVV_LVA[6][6] = {{105, 205, 305, 405, 505, 605},  //
                                      {104, 204, 304, 404, 504, 604},  //
                                      {103, 203, 303, 403, 503, 603},  //
                                      {102, 202, 302, 402, 502, 602},  //
                                      {101, 201, 301, 401, 501, 601},  //
                                      {100, 200, 300, 400, 500, 600}};

void ScoreMove(const Board &board, Move &move) {
  if (board.isCapture(move)) {
    auto attacker_type = board.at<PieceType>(move.from());
    auto victim_type = board.at<PieceType>(move.to());
    move.setScore(MVV_LVA[attacker_type][victim_type]);
  } else {
    move.setScore(0);
  }
}

void Seen(const Board &board) {
  auto key = Board::Compact::encode(board);
  auto it = board_repetition.find(key);
  if (it == board_repetition.end()) {
    board_repetition[key] = 1;
    return;
  }
  (it->second)++;
}

void Unseen(const Board &board) {
  auto key = Board::Compact::encode(board);
  auto it = board_repetition.find(key);
  if (it == board_repetition.end()) {
    return;
  }
  if (it->second == 1) {
    board_repetition.erase(key);
    return;
  }
  (it->second)--;
}

// Returns true if when the board is reached, it's a three fold repetition.
bool IsThreeFoldRepetition(const Board &board) {
  auto key = Board::Compact::encode(board);
  auto it = board_repetition.find(key);
  if (it == board_repetition.end()) {
    return false;
  }
  if (it->second >= 3) return true;
  return false;
}

int Evaluate(const Board &board) {
  int opening_eval = 0;
  int endgame_eval = 0;
  int base_eval = 0;
  int num_pieces = 0;
  for (auto c : {Color::WHITE, Color::BLACK}) {
    auto king = board.pieces(PieceType::KING, c);
    auto queens = board.pieces(PieceType::QUEEN, c);
    auto rooks = board.pieces(PieceType::ROOK, c);
    auto bishops = board.pieces(PieceType::BISHOP, c);
    auto knights = board.pieces(PieceType::KNIGHT, c);
    auto pawns = board.pieces(PieceType::PAWN, c);

    int sign = (c == Color::WHITE) ? 1 : -1;
    auto f = [&num_pieces, &c, &sign](int &target, auto &pieces, auto sq_value,
                                      int value) {
      while (pieces) {
        num_pieces++;
        auto sq = pieces.pop();
        if (c == Color::WHITE) sq = 63 - sq;
        target += (value + sq_value[sq]) * sign;
      }
    };
    f(base_eval, queens, QUEEN_SQ_VALUE, 900);
    f(base_eval, rooks, ROOK_SQ_VALUE, 500);
    f(base_eval, bishops, BISHOP_SQ_VALUE, 330);
    f(base_eval, knights, KNIGHT_SQ_VALUE, 320);
    f(base_eval, pawns, PAWN_SQ_VALUE, 100);
    f(opening_eval, king, KING_OPENING_SQ_VALUE, 20000);
    f(endgame_eval, king, KING_ENDGAME_SQ_VALUE, 20000);
  }
  float phase = num_pieces / 32.0f;
  return base_eval + opening_eval * phase + (1 - phase) * endgame_eval;
}

Move best_move = Move::NO_MOVE;
int ply = 0;
int node;

int negamax(Board &board, int depth, int alpha, int beta) {
  node++;
  if (depth == 0) {
    if (IsThreeFoldRepetition(board)) {
      return 0;
    }
    return (board.sideToMove() == Color::WHITE) ? Evaluate(board)
                                                : -Evaluate(board);
  }

  // Score moves for better pruning.
  Movelist moves;
  movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);
  for (auto &move : moves) {
    ScoreMove(board, move);
  }
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.score() > b.score(); });

  if (moves.empty()) {
    Color color = board.sideToMove();
    // Lose
    if (board.inCheck()) return -999999 + ply;
    // Draw
    return 0;
  }

  Move local_best = Move::NO_MOVE;
  int old_alpha = alpha;
  for (const auto &move : moves) {
    board.makeMove(move);
    Seen(board);
    ply++;
    auto eval = -negamax(board, depth - 1, -beta, -alpha);
    ply--;
    Unseen(board);
    board.unmakeMove(move);
    if (eval > alpha) {
      alpha = eval;
      local_best = move;
    }
    alpha = std::max(alpha, eval);
    if (beta <= alpha) break;
  }
  best_move = local_best;
  return alpha;
}

int main(int argc, char **argv) {
  int depth = std::stoi(std::string(argv[1]));

  for (;;) {
    node = 0;
    best_move = Move::NO_MOVE;
    ply = 0;

    std::string fen;
    std::getline(std::cin, fen);

    auto start = std::chrono::high_resolution_clock::now();
    Board board = Board(fen);

    Seen(board);

    int num_pieces = 0;
    auto pieces = board.all();
    while (pieces) {
      num_pieces++;
      (void)pieces.pop();
    }

    // Max for white and min for black.
    bool maximizing_player = board.sideToMove() == Color::WHITE;
    auto eval = negamax(board, depth, NINF, INF);

    if (best_move != Move::NO_MOVE) {
      std::cout << uci::moveToUci(best_move) << std::endl;
      board.makeMove(best_move);
      Seen(board);
    } else {
      std::cout << "error" << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cerr << "Operation of depth " << depth << " took " << duration.count()
              << " milliseconds, node: " << node << std::endl;
    if (duration.count() < 20) {
      depth++;
      // std::cerr << "increase adjustment to " << depth << std::endl;
    } else if (duration.count() > 400) {
      depth--;
      // std::cerr << "decrease adjustment to " << depth << std::endl;
    }
    depth = std::max(2, depth);
  }

  return 0;
}
