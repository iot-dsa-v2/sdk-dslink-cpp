#ifndef DSA_SDK_NODE_MODEL_H
#define DSA_SDK_NODE_MODEL_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "model_base.h"

#include <unordered_map>

namespace dsa {
class StorageBucket;

class NodeModel : public NodeModelBase {
 private:
  PermissionLevel _set_value_require_permission = PermissionLevel::NEVER;
  void set_value_require_permission(PermissionLevel permission_level);

 protected:
  ref_<NodeModel> _profile;
  std::unordered_map<string_, VarBytesRef> _metas;
  std::unordered_map<string_, VarBytesRef> _attributes;
  std::unordered_map<string_, ref_<NodeModelBase>> _list_children;

  VarBytesRef _summary;

  void destroy_impl() override;

  // initialize will be called when NodeModel is attached to NodeState
  void initialize() override;

 public:
  NodeModel(LinkStrandRef &&strand,
            PermissionLevel write_require_permission = PermissionLevel::NEVER);
  NodeModel(LinkStrandRef &&strand, ref_<NodeModel> &&profile,
            PermissionLevel write_require_permission = PermissionLevel::NEVER);

  void on_list(BaseOutgoingListStream &stream, bool first_request) override;

  void update_property(const string_ &field, VarBytesRef &&value);
  void update_property(const string_ &field, Var &&value) {
    update_property(field, make_ref_<VarBytes>(std::move(value)));
  }
  ref_<NodeModelBase> add_list_child(const string_ &name,
                                     ref_<NodeModelBase> &&model);
  void remove_list_child(const string_ &name);

  VarBytesRef &get_summary() override;

 protected:
  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override;

  void send_props_list(BaseOutgoingListStream &stream);
  void send_children_list(BaseOutgoingListStream &stream);

  void set(ref_<OutgoingSetStream> &&stream) override;
  virtual MessageStatus on_set_value(MessageValue &&value);
  virtual MessageStatus on_set_attribute(const string_ &field, Var &&value);

 public:  // serialization logic
  void save(StorageBucket &storage, const string_ &storage_path = "",
            bool recursive = false, bool user_json = false) const;
  void load(VarMap &map);

 protected:  // serialization implementation
  // return true if a metadata should be saved
  virtual bool save_meta(const string_ &name) const { return false; }
  // return true if a attribute should be saved
  virtual bool save_attribute(const string_ &name) const { return true; }
  // return true if a child should be saved
  virtual bool save_child(const string_ &name) const { return false; }
  // handler of children loading
  // usually this should create an instance of child and waiting for the
  // storage loader to call load() on that instance
  virtual void on_load_child(const string_ &name, VarMap &map){};

  // extra data that's not stored in metadata or attribute
  // prefix these properties with ? so they won't conflict with children name
  virtual void save_extra(VarMap &map) const {};
  virtual void load_extra(VarMap &map){};
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H
