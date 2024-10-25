//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_RESULT_H_
#define CPPCOROUTINES_04_TASK_RESULT_H_

#include <exception>

template<typename T>
struct Result {

  explicit Result() = default;

  explicit Result(T &&value) : _value(std::forward<T>(value)) {}

  explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(std::move(exception_ptr)) {}

  T get_or_throw() {
    if (_exception_ptr) {
      std::rethrow_exception(std::move(_exception_ptr));
    }
    return _value;
  }

 private:
  T _value{};
  std::exception_ptr _exception_ptr;
};

template<>
struct Result<int> {

    explicit Result() = default;

    explicit Result(int value) : _value(value) {}

    // 或者保持右值引用版本
    explicit Result(std::exception_ptr&& exception_ptr)
            : _exception_ptr(std::move(exception_ptr)) {}

    int get_or_throw() {
        if (_exception_ptr) {
            std::rethrow_exception(std::move(_exception_ptr));
        }
        return _value;
    }

private:
    int _value{};
    std::exception_ptr _exception_ptr;
};


template<>
struct Result<void> {

  explicit Result() = default;

  explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(std::move(exception_ptr)) {}

  void get_or_throw() {
    if (_exception_ptr) {
      std::rethrow_exception(std::move(_exception_ptr));
    }
  }

 private:
  std::exception_ptr _exception_ptr;
};

#endif //CPPCOROUTINES_04_TASK_RESULT_H_
