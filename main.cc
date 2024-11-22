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

// Move best_move = Move::NO_MOVE;
Move killer_moves[2][64];
int history_moves_score[12][64];
Move pv_table[64][64];

int ply = 0;
int nodes;
bool follow_pv;

void PvToStderr() {
  for (int i = 0; i < 64; ++i) {
    const Move &move = pv_table[0][i];
    if (move == Move::NO_MOVE) return;
    if (i != 0) std::cerr << " ";
    std::cerr << uci::moveToUci(move);
  }
}

void ScoreMove(const Board &board, Move &move) {
  auto attacker_type = board.at<PieceType>(move.from());
  auto target_sq = move.to();
  if (board.isCapture(move)) {
    auto victim_type = board.at<PieceType>(target_sq);
    move.setScore(MVV_LVA[attacker_type][victim_type] + 10000);
  } else if (killer_moves[0][ply] == move) {
    move.setScore(9000);
  } else if (killer_moves[1][ply] == move) {
    move.setScore(8000);
  } else {
    int piece_index = board.sideToMove() * 6 + attacker_type;
    move.setScore(history_moves_score[piece_index][target_sq.index()]);
  }
}

void ScoreMoves(const Board &board, Movelist &moves) {
  Move *pv_move = nullptr;
  for (auto &move : moves) {
    ScoreMove(board, move);
    if (move == pv_table[0][ply]) pv_move = &move;
  }
  if (follow_pv) {
    if (pv_move == nullptr) {
      follow_pv = false;
    } else {
      // give pv_move highest sore
      // std::cerr << "current pv move " << *pv_move << " ply " << ply
      //           << std::endl;
      pv_move->setScore(20000);
    }
  }
}

int quiescence(Board &board, int alpha, int beta) {
  nodes++;

  int eval =
      (board.sideToMove() == Color::WHITE) ? Evaluate(board) : -Evaluate(board);
  if (eval >= beta) {
    return beta;
  }
  if (eval > alpha) {
    alpha = eval;
  }

  Movelist moves;
  movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

  for (const auto &move : moves) {
    board.makeMove(move);
    ply++;
    eval = -quiescence(board, -beta, -alpha);
    ply--;
    board.unmakeMove(move);
    if (eval >= beta) {
      return beta;
    }
    if (eval > alpha) {
      alpha = eval;
    }
  }

  return alpha;
}

int negamax(Board &board, int depth, int alpha, int beta) {
  if (IsThreeFoldRepetition(board)) {
    return 0;
  }
  if (depth == 0) {
    return quiescence(board, alpha, beta);
  }

  if (ply > 63) {
    return (board.sideToMove() == Color::WHITE) ? Evaluate(board)
                                                : -Evaluate(board);
  }

  nodes++;

  // Score moves for better pruning.
  Movelist moves;
  movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);
  ScoreMoves(board, moves);
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.score() > b.score(); });

  if (moves.empty()) {
    Color color = board.sideToMove();
    // Lose
    if (board.inCheck()) return -999999 + ply;
    // Draw
    return 0;
  }

  bool found_pv = false;

  for (const auto &move : moves) {
    board.makeMove(move);
    Seen(board);
    ply++;

    // Principal variation search
    int eval = 0;
    if (found_pv) {
      eval = -negamax(board, depth - 1, -alpha - 1, -alpha);
      if (eval > alpha && eval < beta) {
        eval = -negamax(board, depth - 1, -beta, -alpha);
      }
    } else {
      eval = -negamax(board, depth - 1, -beta, -alpha);
    }

    ply--;
    Unseen(board);
    board.unmakeMove(move);
    if (eval >= beta) {
      if (!board.isCapture(move)) {
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = move;
      }
      return beta;
    }
    if (eval > alpha) {
      if (!board.isCapture(move)) {
        auto attacker_type = board.at<PieceType>(move.from());
        int piece_index = board.sideToMove() * 6 + attacker_type;
        history_moves_score[piece_index][move.to().index()] += depth;
      }
      alpha = eval;

      found_pv = true;

      pv_table[ply][ply] = move;
      for (int next_ply = ply + 1; next_ply < 64; next_ply++) {
        auto next = pv_table[ply + 1][next_ply];
        if (next == Move::NO_MOVE) break;
        pv_table[ply][next_ply] = next;
      }
    }
  }
  return alpha;
}

void ResetGlobal() {
  nodes = 0;
  ply = 0;
  for (int i = 0; i < 2; ++i) {
    std::fill(killer_moves[i], killer_moves[i] + 64, Move::NO_MOVE);
  }
  for (int i = 0; i < 12; ++i) {
    std::fill(history_moves_score[i], history_moves_score[i] + 64, 0);
  }
  for (int i = 0; i < 64; ++i) {
    std::fill(pv_table[i], pv_table[i] + 64, Move::NO_MOVE);
  }
  follow_pv = false;
}

void search(std::string &fen, int depth) {
  ResetGlobal();

  Board board = Board(fen);

  // Track the board state after the opponent played, for third fold repetition
  // check.
  Seen(board);

  auto eval = 0;
  for (int current_depth = 1; current_depth <= depth; current_depth++) {
    follow_pv = 1;
    eval = negamax(board, current_depth, NINF, INF);
  }
  std::cerr << "iteration " << depth << " eval " << std::showpos
            << (board.sideToMove() == Color::WHITE ? eval : -eval)
            << std::noshowpos << " pv ";
  PvToStderr();
  std::cerr << " nodes " << nodes << std::endl;

  auto best_move = pv_table[0][0];
  if (best_move != Move::NO_MOVE) {
    std::cout << uci::moveToUci(best_move) << std::endl;

    board.makeMove(best_move);
    Seen(board);
  } else {
    std::cout << "error" << std::endl;
  }
}

int main(int argc, char **argv) {
  int depth = std::stoi(std::string(argv[1]));

  for (;;) {
    std::string fen;
    std::getline(std::cin, fen);

    auto start = std::chrono::high_resolution_clock::now();

    search(fen, depth);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cerr << "depth " << depth << ", time: " << duration.count()
              << " milliseconds" << std::endl;
    if (duration.count() < 50) {
      depth++;
    } else if (duration.count() > 1000) {
      depth -= 2;
    } else if (duration.count() > 400) {
      depth--;
    }
    depth = std::max(2, depth);
  }

  return 0;
}
