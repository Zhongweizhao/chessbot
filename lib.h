#pragma once

#include <cstdint>
#include <cstdlib>

#include "./chess.h"

using namespace chess;

bool AreRooksNotConnected(const Board& board, Color color) {
  Bitboard bitboard = board.us(color);
  Bitboard rooks = board.pieces(PieceType::ROOK, color);

  if (!rooks) return false;
  auto r1 = rooks.pop();
  if (!rooks) return false;
  auto r2 = rooks.pop();
  if (rooks) return false;

  auto rook1 = Square(r1);
  auto rook2 = Square(r2);

  auto target_rank = (color == Color::WHITE) ? Rank::RANK_1 : Rank::RANK_8;

  if (rook1.rank() != target_rank || rook2.rank() != target_rank) return false;

  auto min_file = std::min(rook1.file(), rook2.file());
  auto max_file = std::max(rook1.file(), rook2.file());

  for (auto file = min_file + 1; file < max_file; ++file) {
    auto sq = Square(file, target_rank);
    if (bitboard.check(sq.index())) return true;
  }
  return false;
}

bool AreRooksConnected(const Board& board, Color color) {
  Bitboard bitboard = board.us(color);
  Bitboard rooks = board.pieces(PieceType::ROOK, color);

  if (!rooks) return false;
  auto r1 = rooks.pop();
  if (!rooks) return false;
  auto r2 = rooks.pop();
  if (rooks) return false;

  auto rook1 = Square(r1);
  auto rook2 = Square(r2);

  auto target_rank = (color == Color::WHITE) ? Rank::RANK_1 : Rank::RANK_8;

  if (rook1.rank() != target_rank || rook2.rank() != target_rank) return false;

  auto min_file = std::min(rook1.file(), rook2.file());
  auto max_file = std::max(rook1.file(), rook2.file());

  for (auto file = min_file + 1; file < max_file; ++file) {
    auto sq = Square(file, target_rank);
    if (bitboard.check(sq.index())) return false;
  }
  return true;
}

bool IsPawnOccupyingCenter(const Board& board, const Move& move, Color color) {
  Rank min_rank;
  Rank max_rank;
  if (color == Color::WHITE) {
    min_rank = Rank::RANK_2;
    max_rank = Rank::RANK_5;
  } else {
    max_rank = Rank::RANK_7;
    min_rank = Rank::RANK_4;
  }

  auto from = move.from();
  auto to = move.to();
  if (from.file() != to.file()) return false;
  if (from.file() < File::FILE_D || from.file() > File::FILE_E) return false;

  if (from.rank() < min_rank || from.rank() > max_rank) return false;
  if (to.rank() < min_rank || to.rank() > max_rank) return false;

  if (board.at<PieceType>(from) != PieceType::PAWN) return false;

  return true;
}

float IsKnightDevelopment(const Board& board, const Move& move, Color color) {
  auto from = move.from();
  if (board.at<PieceType>(from) != PieceType::KNIGHT) return 0;

  if (!Rank::back_rank(from.rank(), color)) return 0;

  static float middle_file =
      ((float)static_cast<int>(File::FILE_D) + static_cast<int>(File::FILE_E)) /
      2;
  static float middle_rank =
      ((float)static_cast<int>(Rank::RANK_4) + static_cast<int>(Rank::RANK_5)) /
      2;

  auto score_square = [&](const Square& sq) -> float {
    return std::abs(static_cast<int>(sq.file()) - middle_file) +
           std::abs(static_cast<int>(sq.rank()) - middle_rank);
  };

  float score_before = score_square(from);
  float score_after = score_square(move.to());
  return score_before - score_after;
}

bool IsBishopDevelopment(const Board& board, const Move& move, Color color) {
  auto from = move.from();
  if (board.at<PieceType>(from) != PieceType::BISHOP) return false;

  if (Rank::back_rank(from.rank(), color)) return true;
  return false;
}

// Assigns a heuristic score to the move if the move were to be applied on the
// board.
void ScoreMove(Board& board, Move& move) {
  std::int16_t score = 0;

  bool color = board.sideToMove();
  bool rooks_not_connected_before = AreRooksNotConnected(board, color);
  bool is_capture = board.isCapture(move);
  bool is_pawn_occupying_center = IsPawnOccupyingCenter(board, move, color);
  float is_knight_development = IsKnightDevelopment(board, move, color);
  bool is_bishop_development = IsBishopDevelopment(board, move, color);
  board.makeMove(move);
  bool is_check = board.inCheck();
  bool rooks_connected_now = AreRooksConnected(board, color);
  board.unmakeMove(move);

  // Check
  if (is_check) score += 30;
  // Captures
  if (is_capture) score += 30;
  // Pawn occupy center
  if (is_pawn_occupying_center) {
    auto distance = std::abs(move.from().rank() - move.to().rank());
    if (distance > 1) {
      score += 30;
    } else {
      score += 20;
    }
  }
  // Knight development
  if (is_knight_development > 0) {
    /* std::cout << std::endl */
    /*           << board.getFen() << std::endl */
    /*           << uci::moveToSan(board, move) << " is knight development" */
    /*           << std::endl; */
    score += is_knight_development * 10;
  }
  // Bishop Development
  if (is_bishop_development) score += 25;
  // Short Castle
  // Long Castle
  bool is_castle = (move.typeOf() == Move::CASTLING);
  if (is_castle) score += 70;
  // Connect Rooks
  if (rooks_not_connected_before && rooks_connected_now) {
    score += 30;
  }
  move.setScore(score);
}
