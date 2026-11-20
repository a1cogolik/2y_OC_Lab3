// memlab2.cpp — опыт 2 «Матрица по строкам и по столбцам».
// Сборка: g++ -O2 -o memlab2 memlab2.cpp
#include <chrono>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using Clock = std::chrono::steady_clock;

constexpr std::size_t kN = 8192;

class Matrix {
 public:
  Matrix(std::size_t rows, std::size_t cols)
      : rows_(rows), cols_(cols), data_(rows * cols, 0) {}

  int32_t& At(std::size_t i, std::size_t j) {
    return data_[i * cols_ + j];
  }
  int32_t At(std::size_t i, std::size_t j) const {
    return data_[i * cols_ + j];
  }

  std::size_t rows() const { return rows_; }
  std::size_t cols() const { return cols_; }

 private:
  std::size_t rows_;
  std::size_t cols_;
  std::vector<int32_t> data_;
};

Matrix GenerateMatrix(std::size_t rows, std::size_t cols, unsigned int seed) {
  Matrix m(rows, cols);
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> dist(0, 9);
  for (std::size_t i = 0; i < rows; ++i) {
    for (std::size_t j = 0; j < cols; ++j) {
      m.At(i, j) = dist(gen);
    }
  }
  return m;
}

uint64_t SumByRows(const Matrix& m) {
  uint64_t sum = 0;
  // По строкам
  for (std::size_t i = 0; i < m.rows(); ++i) {
    for (std::size_t j = 0; j < m.cols(); ++j) {
      sum += m.At(i, j);
    }
  }
  return sum;
}

uint64_t SumByColumns(const Matrix& m) {
  uint64_t sum = 0;
  // По столбцам
  for (std::size_t j = 0; j < m.cols(); ++j) {
    for (std::size_t i = 0; i < m.rows(); ++i) {
      sum += m.At(i, j);
    }
  }
  return sum;
}

double ElapsedMs(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start)
      .count();
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "RU");
  if (argc < 2) {
    std::printf("Использование: %s <сид>\n", argv[0]);
    return 1;
  }
  unsigned int seed =
      static_cast<unsigned int>(std::strtoul(argv[1], nullptr, 10));

  std::printf("N=%zu  размер=%zu МиБ  сид=%u\n", kN,
              (kN * kN * sizeof(int32_t)) / (1024 * 1024), seed);

  Matrix m = GenerateMatrix(kN, kN, seed);

  auto start = Clock::now();
  uint64_t sum_rows = SumByRows(m);
  double t_rows = ElapsedMs(start);

  start = Clock::now();
  uint64_t sum_cols = SumByColumns(m);
  double t_cols = ElapsedMs(start);

  std::printf("По строкам:  сумма=%llu  время=%.1f мс\n",
              (unsigned long long)sum_rows, t_rows);
  std::printf("По столбцам: сумма=%llu  время=%.1f мс\n",
              (unsigned long long)sum_cols, t_cols);
  std::printf("Суммы совпали: %s\n", sum_rows == sum_cols ? "да" : "НЕТ");
  std::printf("Разница: %.1fx\n", t_cols / t_rows);
  return 0;
}
