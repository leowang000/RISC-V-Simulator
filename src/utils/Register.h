#ifndef RISC_V_SIMULATOR_REGISTER_H
#define RISC_V_SIMULATOR_REGISTER_H

namespace bubble {

template<class T>
class Register {
 public:
  Register() = default;
  explicit Register(const T &data);

  void Update();
  void Write(const T &src);
  T &New();
  const T &New() const;
  const T &GetCur() const;

 private:
  T cur_, new_;
};

template<class T>
Register<T>::Register(const T &data) : cur_(data), new_(data) {}

template<class T>
void Register<T>::Update() {
  cur_ = new_;
}

template<class T>
void Register<T>::Write(const T &src) {
  new_ = src;
}

template<class T>
T &Register<T>::New() {
  return new_;
}

template<class T>
const T &Register<T>::New() const {
  return new_;
}

template<class T>
const T &Register<T>::GetCur() const {
  return cur_;
}

}

#endif //RISC_V_SIMULATOR_REGISTER_H
