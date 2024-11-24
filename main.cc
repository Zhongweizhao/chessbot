#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "./chess.h"

using namespace chess;

constexpr int INF = 9999999;
constexpr int NINF = -INF;

static constexpr uint8_t WHITE_SQ_INDEX[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,  //
    48, 49, 50, 51, 52, 53, 54, 55,  //
    40, 41, 42, 43, 44, 45, 46, 47,  //
    32, 33, 34, 35, 36, 37, 38, 39,  //
    24, 25, 26, 27, 28, 29, 30, 31,  //
    16, 17, 18, 19, 20, 21, 22, 23,  //
    8,  9,  10, 11, 12, 13, 14, 15,  //
    0,  1,  2,  3,  4,  5,  6,  7,
};

static constexpr int BLACK_SQ_INDEX[64] = {
    0,  1,  2,  3,  4,  5,  6,  7,   //
    8,  9,  10, 11, 12, 13, 14, 15,  //
    16, 17, 18, 19, 20, 21, 22, 23,  //
    24, 25, 26, 27, 28, 29, 30, 31,  //
    32, 33, 34, 35, 36, 37, 38, 39,  //
    40, 41, 42, 43, 44, 45, 46, 47,  //
    48, 49, 50, 51, 52, 53, 54, 55,  //
    56, 57, 58, 59, 60, 61, 62, 63,
};

static constexpr int PAWN_OPENING_SQ_VALUE[64] = {
    0,  0,  0,   0,   0,   0,   0,  0,   //
    50, 50, 50,  50,  50,  50,  50, 50,  //
    10, 10, 20,  30,  30,  20,  10, 10,  //
    5,  5,  10,  25,  25,  10,  5,  5,   //
    0,  0,  0,   20,  22,  0,   0,  0,   //
    5,  -5, -10, 0,   0,   -10, -5, 5,   //
    5,  10, 10,  -40, -40, 10,  10, 5,   //
    0,  0,  0,   0,   0,   0,   0,  0};

static constexpr int PAWN_ENDGAME_SQ_VALUE[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,    //
    178, 173, 158, 134, 147, 132, 165, 187,  //
    94,  100, 85,  67,  56,  53,  82,  84,   //
    32,  24,  13,  5,   -2,  4,   17,  17,   //
    13,  9,   -3,  -7,  -7,  -8,  3,   -1,   //
    4,   7,   -6,  1,   0,   -5,  -1,  -8,   //
    13,  8,   8,   10,  13,  0,   2,   -7,   //
    0,   0,   0,   0,   0,   0,   0,   0};

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
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -40, -40, -40, -40, -40, -40, -40, -40,  //
    -20, -20, -20, -20, -20, -20, -20, -20,  //
    0,   20,  40,  -20, 0,   -20, 40,  20};

static constexpr int KING_ENDGAME_SQ_VALUE[64] = {
    0,  10, 20, 30, 30, 20, 10, 0,   //
    10, 20, 30, 40, 40, 30, 20, 10,  //
    20, 30, 40, 50, 50, 40, 30, 20,  //
    30, 40, 50, 60, 60, 50, 40, 30,  //
    30, 40, 50, 60, 60, 50, 40, 30,  //
    20, 30, 40, 50, 50, 40, 30, 20,  //
    10, 20, 30, 40, 40, 30, 20, 10,  //
    0,  10, 20, 30, 30, 20, 10, 0};

static std::map<PackedBoard, int> board_repetition = {};
int time_remaining_ms = 0;
int total_time_used_ms = 0;

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
    // pieces bitboard is copied
    auto f = [&num_pieces, &c, &sign](int &target, auto pieces, auto sq_value,
                                      int value) {
      while (pieces) {
        num_pieces++;
        auto sq = pieces.pop();
        if (c == Color::WHITE)
          sq = WHITE_SQ_INDEX[sq];
        else
          sq = BLACK_SQ_INDEX[sq];
        target += (value + sq_value[sq]) * sign;
      }
    };
    f(base_eval, queens, QUEEN_SQ_VALUE, 900);
    f(base_eval, rooks, ROOK_SQ_VALUE, 500);
    f(base_eval, bishops, BISHOP_SQ_VALUE, 330);
    f(base_eval, knights, KNIGHT_SQ_VALUE, 320);
    f(opening_eval, pawns, PAWN_OPENING_SQ_VALUE, 100);
    f(endgame_eval, pawns, PAWN_ENDGAME_SQ_VALUE, 100);
    f(opening_eval, king, KING_OPENING_SQ_VALUE, 20000);
    f(endgame_eval, king, KING_ENDGAME_SQ_VALUE, 20000);
  }
  float phase = num_pieces / 32.0f;
  return base_eval + opening_eval * phase + (1 - phase) * endgame_eval;
}

Move killer_moves[2][64];
int history_moves_score[12][64];
Move pv_table[2][64][64];

Move (*cur_pv_table)[64][64];
Move (*prev_pv_table)[64][64];

int ply = 0;
int nodes;
bool follow_pv;

void PrevPvToStderr() {
  for (int i = 0; i < 64; ++i) {
    const Move &move = (*prev_pv_table)[0][i];
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
    if (move == (*cur_pv_table)[0][ply]) pv_move = &move;
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

int quiescence(Board &board, int alpha, int beta,
               const std::chrono::time_point<std::chrono::high_resolution_clock>
                   &deadline) {
  if (std::chrono::high_resolution_clock::now() > deadline) {
    throw "Deadline passed";
  }

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
  ScoreMoves(board, moves);
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.score() > b.score(); });

  for (const auto &move : moves) {
    board.makeMove(move);
    ply++;
    eval = -quiescence(board, -beta, -alpha, deadline);
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

int negamax(Board &board, int depth, int alpha, int beta,
            const std::chrono::time_point<std::chrono::high_resolution_clock>
                &deadline) {
  constexpr static int FULL_DEPTH_MOVE = 4;
  constexpr static int REDUCTION_LIMIT = 3;

  if (std::chrono::high_resolution_clock::now() > deadline) {
    throw "Deadline passed";
  }

  if (IsThreeFoldRepetition(board)) {
    return 0;
  }
  if (depth == 0) {
    return quiescence(board, alpha, beta, deadline);
  }

  if (ply > 63) {
    return (board.sideToMove() == Color::WHITE) ? Evaluate(board)
                                                : -Evaluate(board);
  }

  nodes++;

  bool in_check = board.inCheck();

  // Null move reduction
  if (depth >= 3 && !in_check && ply > 0) {
    board.makeNullMove();
    int R = 2;
    int score = -negamax(board, depth - 1 - R, -beta, -beta + 1, deadline);
    board.unmakeNullMove();
    if (score >= beta) {
      return beta;
    }
  }

  // Score moves for better pruning.
  Movelist moves;
  movegen::legalmoves<movegen::MoveGenType::ALL>(moves, board);
  ScoreMoves(board, moves);
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.score() > b.score(); });

  if (moves.empty()) {
    // Lose
    if (in_check) return -999999 + ply;
    // Draw
    return 0;
  }

  bool found_pv = false;
  int moves_searched = 0;

  for (const auto &move : moves) {
    board.makeMove(move);
    Seen(board);
    ply++;

    // first move
    int eval = 0;
    if (moves_searched == 0) {
      eval = -negamax(board, depth - 1, -beta, -alpha, deadline);
    } else {
      auto full_depth_search = [&]() {
        // Principal variation search, mixed with last move reduction
        eval = -negamax(board, depth - 1, -alpha - 1, -alpha, deadline);
        if (eval > alpha && eval < beta) {
          eval = -negamax(board, depth - 1, -beta, -alpha, deadline);
        }
      };
      // late move reduction
      if (moves_searched >= FULL_DEPTH_MOVE && depth >= REDUCTION_LIMIT &&
          !in_check) {
        eval = -negamax(board, depth - 2, -alpha - 1, -alpha, deadline);
        if (eval > alpha) full_depth_search();
      } else {
        full_depth_search();
      }
    }

    ply--;
    Unseen(board);
    board.unmakeMove(move);
    moves_searched++;
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

      (*cur_pv_table)[ply][ply] = move;
      for (int next_ply = ply + 1; next_ply < 64; next_ply++) {
        auto next = (*cur_pv_table)[ply + 1][next_ply];
        if (next == Move::NO_MOVE) break;
        (*cur_pv_table)[ply][next_ply] = next;
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
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 64; ++j) {
      std::fill(pv_table[i][j], pv_table[i][j] + 64, Move::NO_MOVE);
    }
  }
  follow_pv = false;
  cur_pv_table = &pv_table[0];
  prev_pv_table = &pv_table[1];
}

void search(std::string &fen) {
  auto start = std::chrono::high_resolution_clock::now();

  ResetGlobal();

  int allocated_time = 0;
  if (time_remaining_ms >= 4000) {
    allocated_time = 360;
  } else if (time_remaining_ms >= 2000) {
    allocated_time = 180;
  } else {
    allocated_time = 90;
  }
  const std::chrono::time_point<std::chrono::high_resolution_clock> deadline =
      start + std::chrono::milliseconds(allocated_time);

  Board board = Board(fen);
  // Track the board state after the opponent played, for third fold repetition
  // check.
  Seen(board);

  auto eval = 0;
  int completed_depth = 0;
  // Backup three fold repetition tracker
  std::map<PackedBoard, int> board_repetition_cp = board_repetition;

  // Iterative deepening
  try {
    int alpha = NINF;
    int beta = INF;
    for (; completed_depth <= 20;) {
      follow_pv = 1;
      eval = negamax(board, completed_depth + 1, alpha, beta, deadline);

      // Aspiration window
      if (eval <= alpha || eval >= beta) {
        // reset to full width window
        // std::cerr << "aspiration window failed" << std::endl;
        alpha = NINF;
        beta = INF;
        continue;
      }
      // Setup window for next depth.
      static int ASPIRATION_WINDOW = 50;
      alpha = eval - ASPIRATION_WINDOW;
      beta = eval + ASPIRATION_WINDOW;

      // Increment depth
      ++completed_depth;
      // Store completed depth pv_table, and prepare new one for next depth.
      auto *temp = prev_pv_table;
      prev_pv_table = cur_pv_table;
      cur_pv_table = temp;
    }
  } catch (const char *msg) {
    // Resetting the board, because of exception, board could have been in an
    // inconsistent state
    board = Board(fen);
    // Restore board_repetition.
    board_repetition = board_repetition_cp;
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto best_move = (*prev_pv_table)[0][0];
  if (best_move != Move::NO_MOVE) {
    std::cout << uci::moveToUci(best_move) << std::endl;

    board.makeMove(best_move);
    Seen(board);
  } else {
    std::cout << "error" << std::endl;
  }

  auto duration_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  total_time_used_ms += duration_ms;

  std::cerr << "iteration " << completed_depth << " eval "
            << std::showpos
            // We have made a move and the board is for the opponent so the eval
            // sign is flipped.
            << (board.sideToMove() == Color::WHITE ? -eval : eval)
            << std::noshowpos << " pv ";
  PrevPvToStderr();
  std::cerr << " nodes " << nodes << " time " << duration_ms
            << " milliseconds total_time " << total_time_used_ms << std::endl;
}

int main(int argc, char **argv) {
  std::ios::sync_with_stdio(false);
  std::cerr << "start" << std::endl;

  total_time_used_ms = 0;
  for (;;) {
    std::string fen;
    std::getline(std::cin, fen);
    std::string remaining_overage_time;
    std::getline(std::cin, remaining_overage_time);
    time_remaining_ms = std::stof(remaining_overage_time) * 1000;
    // std::cerr << "time_remaining_ms: " << time_remaining_ms << std::endl;

    search(fen);
  }

  return 0;
}
