#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if _MSC_VER

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <boost/intrusive_ptr.hpp>


namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <typename T>
using intrusive_ptr_ = boost::intrusive_ptr<T>;



template <typename T>
void print(T t){
  std::cout << t << std::endl;
}

template <typename T1, typename T2>
void print(T1 t1, T2 t2){
  std::cout << t1 << "  " << t2 << std::endl;
}

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
