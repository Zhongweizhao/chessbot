#include <iostream>
#include <list>
#include <string>

#include "./chess.h"

using namespace chess;

constexpr int INF = 9999999;
constexpr int NINF = -INF;

static constexpr int PAWN_SQ_VALUE[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

static constexpr int KNIGHT_SQ_VALUE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
};

static constexpr int BISHOP_SQ_VALUE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
};

static constexpr int ROOK_SQ_VALUE[64] = {
    0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0};

static constexpr int QUEEN_SQ_VALUE[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};

static constexpr int KING_SQ_VALUE[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

int Evaluate(const Board &board) {
  int eval = 0;
  for (auto c : {Color::WHITE, Color::BLACK}) {
    int color_eval = 0;
    auto king = board.pieces(PieceType::KING, c);
    auto queens = board.pieces(PieceType::QUEEN, c);
    auto rooks = board.pieces(PieceType::ROOK, c);
    auto bishops = board.pieces(PieceType::BISHOP, c);
    auto knights = board.pieces(PieceType::KNIGHT, c);
    auto pawns = board.pieces(PieceType::PAWN, c);

    auto f = [&color_eval, &c](auto &pieces, auto sq_value, int value) {
      while (pieces) {
        auto sq = pieces.pop();
        if (c == Color::WHITE) sq = 63 - sq;
        color_eval += value + sq_value[sq];
      }
    };
    f(queens, QUEEN_SQ_VALUE, 900);
    f(rooks, ROOK_SQ_VALUE, 500);
    f(bishops, BISHOP_SQ_VALUE, 330);
    f(knights, KNIGHT_SQ_VALUE, 320);
    f(pawns, PAWN_SQ_VALUE, 100);
    f(king, KING_SQ_VALUE, 20000);

    color_eval *= (c == Color::WHITE) ? 1 : -1;
    eval += color_eval;
  }
  return eval;
}

std::pair<int, std::list<Move>> minimax(Board &board, int depth, int alpha,
                                        int beta, bool maximizing_player) {
  if (depth == 0) {
    return {Evaluate(board), {}};
  }

  Movelist moves;
  movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);

  if (moves.empty()) {
    Color color = board.sideToMove();
    // Lose
    if (board.inCheck()) return {(color == Color::WHITE) ? NINF : INF, {}};
    // Draw
    return {0, {}};
  }

  std::list<Move> best_moves;
  if (maximizing_player) {
    int max_eval = NINF;
    for (const auto &move : moves) {
      /* if (depth == 2) */
      /*   std::cout << "evaluating move " << uci::moveToSan(board, move) << " "
       */
      /*             << move.score() << std::endl; */
      board.makeMove(move);
      auto [eval, next_moves] = minimax(board, depth - 1, alpha, beta, false);
      eval += move.score();
      board.unmakeMove(move);
      /* if (depth == 2) { */
      /*   std::cout << eval; */
      /*   for (auto next_move : next_moves) { */
      /*     std::cout << " " << uci::moveToUci(next_move) << " " */
      /*               << next_move.score(); */
      /*   } */
      /*   std::cout << std::endl; */
      /* } */
      if (eval > max_eval) {
        max_eval = eval;
        next_moves.push_front(move);
        best_moves = next_moves;
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha) break;
    }
    return {max_eval, best_moves};
  } else {
    int min_eval = INF;
    for (const auto &move : moves) {
      board.makeMove(move);
      auto [eval, next_moves] = minimax(board, depth - 1, alpha, beta, true);
      eval -= move.score();
      board.unmakeMove(move);
      if (eval < min_eval) {
        min_eval = eval;
        next_moves.push_front(move);
        best_moves = next_moves;
      }
      beta = std::min(beta, eval);
      if (beta <= alpha) break;
    }
    return {min_eval, best_moves};
  }
}

int main(int argc, char **argv) {
  int depth = std::stoi(std::string(argv[1]));

  for (;;) {
    std::string fen;
    std::getline(std::cin, fen);
    Board board = Board(fen);

    auto [eval, move] = minimax(board, depth, NINF, INF, true);

    if (!move.empty()) {
      std::cout << uci::moveToUci(move.front()) << std::endl;
    } else {
      std::cout << "error" << std::endl;
    }
  }

  return 0;
}
