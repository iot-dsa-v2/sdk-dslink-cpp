#ifndef DSA_SDK_LOCAL_NODE_H
#define DSA_SDK_LOCAL_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "../node_model.h"

namespace dsa {

//class LocalNode : public NodeModel {
// protected:
//  MessageValue _value;
//  bool _value_ready = false;
//
//  std::unordered_map<std::string, ref_<LocalNode>> _children;
//
// public:
//  const Path path;
//
//  void on_subscribe(const SubscribeOptions &options) override;
//};
}

#endif  // DSA_SDK_LOCAL_NODE_H
