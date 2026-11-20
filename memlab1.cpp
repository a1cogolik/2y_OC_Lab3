// memlab1.cpp — опыт 1 «Выделить — не значит получить».
// Сборка: g++ -O2 -o memlab1 memlab1.cpp
#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

using Clock = std::chrono::steady_clock;

// Читает значение в килобайтах из /proc/self/status по ключу
// ("VmSize:" или "VmRSS:"). Возвращает -1, если ключ не найден.
long ReadStatusKb(const char* key) {
  std::ifstream status_file("/proc/self/status");
  std::string line;
  while (std::getline(status_file, line)) {
    if (line.compare(0, std::strlen(key), key) == 0) {
      return std::stol(line.substr(std::strlen(key)));
    }
  }
  return -1;
}

// Читает счётчик minor page faults из /proc/self/stat. Это 8-е поле
// после последней ')'; нумерация полей с нуля — индекс 7.
long ReadMinorFaults() {
  std::ifstream stat_file("/proc/self/stat");
  std::string stat_line;
  std::getline(stat_file, stat_line);
  const char* cursor = stat_line.c_str() + stat_line.rfind(')') + 1;
  for (int field_index = 0; field_index < 7; ++field_index) {
    cursor = std::strchr(cursor + 1, ' ');
  }
  return std::strtol(cursor + 1, nullptr, 10);
}

// Печатает снимок VmSize/VmRSS/minor faults с подписью этапа.
void ReportMemory(const char* stage_label) {
  std::printf("%-22s VmSize=%6.1fM  RSS=%6.1fM  minflt=%ld\n",
              stage_label,
              ReadStatusKb("VmSize:") / 1024.0,
              ReadStatusKb("VmRSS:") / 1024.0,
              ReadMinorFaults());
}

// Возвращает число миллисекунд, прошедших с момента start_time.
double ElapsedMs(Clock::time_point start_time) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start_time)
      .count();
}

int main() {
  const size_t page_size = sysconf(_SC_PAGESIZE); // размер страницы в байтах 4096
  const size_t page_count = (256UL << 20) / page_size; //  256MiB / 4096 = 65536 - это кол-во страниц в 256MiB
  ReportMemory("start");

  // volatile — защита от -O2
  volatile uint8_t* block = static_cast<volatile uint8_t*>(
      mmap(nullptr, page_count * page_size, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  ReportMemory("after mmap");

  // Первое чтение: по одному байту с каждой страницы.
  uint64_t checksum = 0;
  auto start_time = Clock::now();
  for (size_t page_index = 0; page_index < page_count; ++page_index) {
    checksum += block[page_index * page_size];
  }
  std::printf("read    sum=%lu   t=%.1fms\n",
              static_cast<unsigned long>(checksum), ElapsedMs(start_time));
  ReportMemory("after read");

  // Первая запись '1'
  start_time = Clock::now();
  for (size_t page_index = 0; page_index < page_count; ++page_index) {
    block[page_index * page_size] = 1;
  }
  std::printf("write1  t=%.1fms\n", ElapsedMs(start_time));
  ReportMemory("after write1");

  // Повторная запись '2'
  start_time = Clock::now();
  for (size_t page_index = 0; page_index < page_count; ++page_index) {
    block[page_index * page_size] = 2;
  }
  std::printf("write2  t=%.1fms\n", ElapsedMs(start_time));
  ReportMemory("after write2");

  // Проверка
  std::printf("verify: %u %u %u %u\n",
              block[0],
              block[page_size],
              block[2 * page_size],
              block[(page_count - 1) * page_size]);

  munmap(const_cast<uint8_t*>(block), page_count * page_size);
  ReportMemory("after munmap");
  return 0;
}
