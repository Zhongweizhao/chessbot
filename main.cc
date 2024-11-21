#include <iostream>
#include <list>
#include <string>

#include "./chess.h"
/* #include "./lib.h" */

using namespace chess;

constexpr int INF = 9999999;
constexpr int NINF = -INF;

int Evaluate(const Board &board) {
  int eval = 0;
  for (auto c : {Color::WHITE, Color::BLACK}) {
    int color_eval = 0;
    auto queens = board.pieces(PieceType::QUEEN, c);
    auto rooks = board.pieces(PieceType::ROOK, c);
    auto bishops = board.pieces(PieceType::BISHOP, c);
    auto knights = board.pieces(PieceType::KNIGHT, c);
    auto pawns = board.pieces(PieceType::PAWN, c);

    auto f = [&color_eval](auto &pieces, int value) {
      while (pieces) {
        color_eval += value;
        (void)pieces.pop();
      }
    };
    f(queens, 900);
    f(rooks, 500);
    f(bishops, 300);
    f(knights, 300);
    f(pawns, 100);
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
  /* for (auto &move : moves) { */
  /*   ScoreMove(board, move); */
  /* } */

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
