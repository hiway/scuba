// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef SCUBA_RUNTIME_SERVICE_CONTAINER_H
#define SCUBA_RUNTIME_SERVICE_CONTAINER_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <arpc++/arpc++.h>
#include <flower/protocol/switchboard.ad.h>
#include <google/protobuf/map.h>
#include <google/protobuf/repeated_field.h>
#include <k8s.io/kubernetes/pkg/kubelet/apis/cri/v1alpha1/runtime/api.grpc.pb.h>

namespace scuba {
namespace runtime_service {

class IPAddressLease;

class Container {
 public:
  explicit Container(const runtime::ContainerConfig& config);

  void GetInfo(runtime::Container* info);
  void GetStatus(runtime::ContainerStatus* status);

  bool MatchesFilter(
      std::optional<runtime::ContainerState> state,
      const google::protobuf::Map<std::string, std::string>& labels);

  void Start(
      const runtime::PodSandboxMetadata& pod_metadata,
      const arpc::FileDescriptor& root_directory,
      const arpc::FileDescriptor& image_directory,
      const arpc::FileDescriptor& log_directory,
      flower::protocol::switchboard::Switchboard::Stub* switchboard_servers);
  void Stop(std::int64_t timeout);

  Container(Container&) = delete;
  void operator=(Container) = delete;

 private:
  // Data that should be returned through ContainerStatus.
  const runtime::ContainerMetadata metadata_;
  const runtime::ImageSpec image_;
  const std::chrono::system_clock::time_point creation_time_;
  const google::protobuf::Map<std::string, std::string> labels_;
  const google::protobuf::Map<std::string, std::string> annotations_;
  const google::protobuf::RepeatedPtrField<runtime::Mount> mounts_;
  const std::string log_path_;
  const std::string argdata_;

  runtime::ContainerState GetContainerState_();

  std::unique_ptr<arpc::FileDescriptor> OpenContainerLog_(
      const arpc::FileDescriptor& log_directory);

  runtime::ContainerState container_state_;
  std::optional<arpc::FileDescriptor> child_process_;
  std::chrono::system_clock::time_point start_time_;
  std::chrono::system_clock::time_point finish_time_;
  std::int32_t exit_code_;
};

}  // namespace runtime_service
}  // namespace scuba

#endif