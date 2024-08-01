#ifndef RISC_V_SIMULATOR_CIRCULARQUEUE_H
#define RISC_V_SIMULATOR_CIRCULARQUEUE_H

#include <array>
#include <cassert>

namespace bubble {

template<class T, int capacity>
class CircularQueue {
 public:
  CircularQueue() = default;
  CircularQueue(const std::array<T, capacity + 1> &arr, int front, int rear);
  CircularQueue &operator=(const CircularQueue &other) = default;

  bool IsEmpty() const;
  bool IsFull() const;
  void Enqueue(const T &x);
  void Dequeue();
  T &Front();
  const T &Front() const;
  T &Back();
  const T &Back() const;
  int BeginId() const;
  int EndId() const;
  T &operator[](int index);
  const T &operator[](int index) const;
  void Clear();

 private:
  T data_[capacity + 1];
  int front_, rear_; // [front_, rear_)
};

template<class T, int capacity>
CircularQueue<T, capacity>::CircularQueue(const std::array<T, capacity + 1> &arr, int front, int rear) :
    front_(front), rear_(rear), data_() {
  for (int i = front; i != rear; i = (i + 1) % (capacity + 1)) {
    data_[i] = arr[i];
  }
}

template<class T, int capacity>
bool CircularQueue<T, capacity>::IsEmpty() const {
  return front_ == rear_;
}

template<class T, int capacity>
bool CircularQueue<T, capacity>::IsFull() const {
  return front_ == (rear_ + 1) % (capacity + 1);
}

template<class T, int capacity>
void CircularQueue<T, capacity>::Enqueue(const T &x) {
  assert(!IsFull());
  data_[rear_] = x;
  rear_ = (rear_ + 1) % (capacity + 1);
}

template<class T, int capacity>
void CircularQueue<T, capacity>::Dequeue() {
  assert(!IsEmpty());
  front_ = (front_ + 1) % (capacity + 1);
}

template<class T, int capacity>
T &CircularQueue<T, capacity>::Front() {
  return data_[front_];
}

template<class T, int capacity>
const T &CircularQueue<T, capacity>::Front() const {
  return data_[front_];
}

template<class T, int capacity>
T &CircularQueue<T, capacity>::Back() {
  return data_[rear_ == 0 ? capacity : rear_ - 1];
}

template<class T, int capacity>
const T &CircularQueue<T, capacity>::Back() const {
  return data_[rear_ == 0 ? capacity : rear_ - 1];
}

template<class T, int capacity>
int CircularQueue<T, capacity>::BeginId() const {
  return front_;
}

template<class T, int capacity>
int CircularQueue<T, capacity>::EndId() const {
  return rear_;
}

template<class T, int capacity>
T &CircularQueue<T, capacity>::operator[](int index) {
  return data_[index];
}

template<class T, int capacity>
const T &CircularQueue<T, capacity>::operator[](int index) const {
  return data_[index];
}

template<class T, int capacity>
void CircularQueue<T, capacity>::Clear() {
  rear_ = front_;
}

}

#endif //RISC_V_SIMULATOR_CIRCULARQUEUE_H
