#include <iostream>
#include <vector>
#include <atomic>
#include <string.h>
using namespace std;

void err(const char *msg)
{
        cerr << msg << endl;
}

void message(const char *msg)
{
        cout << msg << endl;
}

struct PacketBuffer {
public:
  static constexpr size_t BUFFER_SIZE = 1500;
  PacketBuffer(){}

  void initialize(int i) {
    char value  = (i % 9) + '0';
    memset(buffer, value, BUFFER_SIZE);
  }

  void print() {
    string s(buffer, buffer + 5);
    cout << s << endl;
  }
private:
  char buffer[BUFFER_SIZE];
};

template<typename T>
class RingBuffer {
public:
  static constexpr size_t FIXED_SIZE = 1 << 3;
  RingBuffer(){}
  
  bool push(T&& t) {

    auto current_tail = write_index.load(memory_order::memory_order_relaxed);
    auto next_tail = (current_tail + 1) % FIXED_SIZE;

    if(next_tail == read_index.load(memory_order::memory_order_acquire)) {

      //  Buffer is full
      return false;
    }

    buffer[current_tail] = std::move(t);
    write_index.store(next_tail, memory_order::memory_order_release);
    return true;
  }

  bool pop(T& t) {
    auto current_head = read_index.load(memory_order::memory_order_relaxed);
    auto next_head = (current_head + 1) % FIXED_SIZE;

    if(next_head == write_index.load(memory_order::memory_order_acquire)) {
      return false;
    }

    t = buffer[current_head];
    read_index.store(next_head, memory_order::memory_order_release);
    return true;
  }

private:
  size_t size;
  std::atomic<size_t> read_index{0};  //head
  std::atomic<size_t> write_index{0}; //tail

  T buffer[FIXED_SIZE];
};

int main() {
  RingBuffer<PacketBuffer> packet_buffer;
  for(int i = 0; i < 10; i++) {
    PacketBuffer packet;
    packet.initialize(i);
    packet_buffer.push(std::move(packet));
  }

  for(int i = 0; i < 8; i++) {
    PacketBuffer packet;
    if(packet_buffer.pop(packet))
    {
      packet.print();
    }
  }

  return 0;
}

