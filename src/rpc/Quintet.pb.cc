// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Quintet.proto

#include "rpc/Quintet.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace quintet {
namespace rpc {
class PbExternalMessageDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<PbExternalMessage>
      _instance;
} _PbExternalMessage_default_instance_;
class PbExternalReplyDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<PbExternalReply>
      _instance;
} _PbExternalReply_default_instance_;
}  // namespace rpc
}  // namespace quintet
namespace protobuf_Quintet_2eproto {
static void InitDefaultsPbExternalMessage() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::quintet::rpc::_PbExternalMessage_default_instance_;
    new (ptr) ::quintet::rpc::PbExternalMessage();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::quintet::rpc::PbExternalMessage::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<0> scc_info_PbExternalMessage =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsPbExternalMessage}, {}};

static void InitDefaultsPbExternalReply() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::quintet::rpc::_PbExternalReply_default_instance_;
    new (ptr) ::quintet::rpc::PbExternalReply();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::quintet::rpc::PbExternalReply::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<0> scc_info_PbExternalReply =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsPbExternalReply}, {}};

void InitDefaults() {
  ::google::protobuf::internal::InitSCC(&scc_info_PbExternalMessage.base);
  ::google::protobuf::internal::InitSCC(&scc_info_PbExternalReply.base);
}

::google::protobuf::Metadata file_level_metadata[2];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalMessage, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalMessage, opname_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalMessage, args_),
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalReply, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalReply, leaderid_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::quintet::rpc::PbExternalReply, ret_),
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::quintet::rpc::PbExternalMessage)},
  { 7, -1, sizeof(::quintet::rpc::PbExternalReply)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::quintet::rpc::_PbExternalMessage_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&::quintet::rpc::_PbExternalReply_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  AssignDescriptors(
      "Quintet.proto", schemas, file_default_instances, TableStruct::offsets,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 2);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\rQuintet.proto\022\013quintet.rpc\"1\n\021PbExtern"
      "alMessage\022\016\n\006opName\030\001 \001(\t\022\014\n\004args\030\002 \001(\t\""
      "0\n\017PbExternalReply\022\020\n\010LeaderId\030\001 \001(\t\022\013\n\003"
      "ret\030\002 \001(\t2R\n\010External\022F\n\004call\022\036.quintet."
      "rpc.PbExternalMessage\032\034.quintet.rpc.PbEx"
      "ternalReply\"\000b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 221);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "Quintet.proto", &protobuf_RegisterTypes);
}

void AddDescriptors() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_Quintet_2eproto
namespace quintet {
namespace rpc {

// ===================================================================

void PbExternalMessage::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int PbExternalMessage::kOpNameFieldNumber;
const int PbExternalMessage::kArgsFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

PbExternalMessage::PbExternalMessage()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_Quintet_2eproto::scc_info_PbExternalMessage.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:quintet.rpc.PbExternalMessage)
}
PbExternalMessage::PbExternalMessage(const PbExternalMessage& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  opname_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.opname().size() > 0) {
    opname_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.opname_);
  }
  args_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.args().size() > 0) {
    args_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.args_);
  }
  // @@protoc_insertion_point(copy_constructor:quintet.rpc.PbExternalMessage)
}

void PbExternalMessage::SharedCtor() {
  opname_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  args_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

PbExternalMessage::~PbExternalMessage() {
  // @@protoc_insertion_point(destructor:quintet.rpc.PbExternalMessage)
  SharedDtor();
}

void PbExternalMessage::SharedDtor() {
  opname_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  args_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void PbExternalMessage::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ::google::protobuf::Descriptor* PbExternalMessage::descriptor() {
  ::protobuf_Quintet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_Quintet_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const PbExternalMessage& PbExternalMessage::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_Quintet_2eproto::scc_info_PbExternalMessage.base);
  return *internal_default_instance();
}


void PbExternalMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:quintet.rpc.PbExternalMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  opname_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  args_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  _internal_metadata_.Clear();
}

bool PbExternalMessage::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:quintet.rpc.PbExternalMessage)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // string opName = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_opname()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->opname().data(), static_cast<int>(this->opname().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "quintet.rpc.PbExternalMessage.opName"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // string args = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_args()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->args().data(), static_cast<int>(this->args().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "quintet.rpc.PbExternalMessage.args"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:quintet.rpc.PbExternalMessage)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:quintet.rpc.PbExternalMessage)
  return false;
#undef DO_
}

void PbExternalMessage::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:quintet.rpc.PbExternalMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string opName = 1;
  if (this->opname().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->opname().data(), static_cast<int>(this->opname().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalMessage.opName");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->opname(), output);
  }

  // string args = 2;
  if (this->args().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->args().data(), static_cast<int>(this->args().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalMessage.args");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->args(), output);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), output);
  }
  // @@protoc_insertion_point(serialize_end:quintet.rpc.PbExternalMessage)
}

::google::protobuf::uint8* PbExternalMessage::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:quintet.rpc.PbExternalMessage)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string opName = 1;
  if (this->opname().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->opname().data(), static_cast<int>(this->opname().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalMessage.opName");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->opname(), target);
  }

  // string args = 2;
  if (this->args().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->args().data(), static_cast<int>(this->args().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalMessage.args");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->args(), target);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:quintet.rpc.PbExternalMessage)
  return target;
}

size_t PbExternalMessage::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:quintet.rpc.PbExternalMessage)
  size_t total_size = 0;

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()));
  }
  // string opName = 1;
  if (this->opname().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->opname());
  }

  // string args = 2;
  if (this->args().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->args());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void PbExternalMessage::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:quintet.rpc.PbExternalMessage)
  GOOGLE_DCHECK_NE(&from, this);
  const PbExternalMessage* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const PbExternalMessage>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:quintet.rpc.PbExternalMessage)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:quintet.rpc.PbExternalMessage)
    MergeFrom(*source);
  }
}

void PbExternalMessage::MergeFrom(const PbExternalMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:quintet.rpc.PbExternalMessage)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.opname().size() > 0) {

    opname_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.opname_);
  }
  if (from.args().size() > 0) {

    args_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.args_);
  }
}

void PbExternalMessage::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:quintet.rpc.PbExternalMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PbExternalMessage::CopyFrom(const PbExternalMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:quintet.rpc.PbExternalMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PbExternalMessage::IsInitialized() const {
  return true;
}

void PbExternalMessage::Swap(PbExternalMessage* other) {
  if (other == this) return;
  InternalSwap(other);
}
void PbExternalMessage::InternalSwap(PbExternalMessage* other) {
  using std::swap;
  opname_.Swap(&other->opname_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  args_.Swap(&other->args_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::google::protobuf::Metadata PbExternalMessage::GetMetadata() const {
  protobuf_Quintet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_Quintet_2eproto::file_level_metadata[kIndexInFileMessages];
}


// ===================================================================

void PbExternalReply::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int PbExternalReply::kLeaderIdFieldNumber;
const int PbExternalReply::kRetFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

PbExternalReply::PbExternalReply()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_Quintet_2eproto::scc_info_PbExternalReply.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:quintet.rpc.PbExternalReply)
}
PbExternalReply::PbExternalReply(const PbExternalReply& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  leaderid_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.leaderid().size() > 0) {
    leaderid_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.leaderid_);
  }
  ret_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.ret().size() > 0) {
    ret_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.ret_);
  }
  // @@protoc_insertion_point(copy_constructor:quintet.rpc.PbExternalReply)
}

void PbExternalReply::SharedCtor() {
  leaderid_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ret_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

PbExternalReply::~PbExternalReply() {
  // @@protoc_insertion_point(destructor:quintet.rpc.PbExternalReply)
  SharedDtor();
}

void PbExternalReply::SharedDtor() {
  leaderid_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ret_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void PbExternalReply::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ::google::protobuf::Descriptor* PbExternalReply::descriptor() {
  ::protobuf_Quintet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_Quintet_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const PbExternalReply& PbExternalReply::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_Quintet_2eproto::scc_info_PbExternalReply.base);
  return *internal_default_instance();
}


void PbExternalReply::Clear() {
// @@protoc_insertion_point(message_clear_start:quintet.rpc.PbExternalReply)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  leaderid_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ret_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  _internal_metadata_.Clear();
}

bool PbExternalReply::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:quintet.rpc.PbExternalReply)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // string LeaderId = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_leaderid()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->leaderid().data(), static_cast<int>(this->leaderid().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "quintet.rpc.PbExternalReply.LeaderId"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // string ret = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_ret()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->ret().data(), static_cast<int>(this->ret().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "quintet.rpc.PbExternalReply.ret"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:quintet.rpc.PbExternalReply)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:quintet.rpc.PbExternalReply)
  return false;
#undef DO_
}

void PbExternalReply::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:quintet.rpc.PbExternalReply)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string LeaderId = 1;
  if (this->leaderid().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->leaderid().data(), static_cast<int>(this->leaderid().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalReply.LeaderId");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->leaderid(), output);
  }

  // string ret = 2;
  if (this->ret().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->ret().data(), static_cast<int>(this->ret().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalReply.ret");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->ret(), output);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), output);
  }
  // @@protoc_insertion_point(serialize_end:quintet.rpc.PbExternalReply)
}

::google::protobuf::uint8* PbExternalReply::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:quintet.rpc.PbExternalReply)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string LeaderId = 1;
  if (this->leaderid().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->leaderid().data(), static_cast<int>(this->leaderid().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalReply.LeaderId");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->leaderid(), target);
  }

  // string ret = 2;
  if (this->ret().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->ret().data(), static_cast<int>(this->ret().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "quintet.rpc.PbExternalReply.ret");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->ret(), target);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:quintet.rpc.PbExternalReply)
  return target;
}

size_t PbExternalReply::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:quintet.rpc.PbExternalReply)
  size_t total_size = 0;

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()));
  }
  // string LeaderId = 1;
  if (this->leaderid().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->leaderid());
  }

  // string ret = 2;
  if (this->ret().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->ret());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void PbExternalReply::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:quintet.rpc.PbExternalReply)
  GOOGLE_DCHECK_NE(&from, this);
  const PbExternalReply* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const PbExternalReply>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:quintet.rpc.PbExternalReply)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:quintet.rpc.PbExternalReply)
    MergeFrom(*source);
  }
}

void PbExternalReply::MergeFrom(const PbExternalReply& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:quintet.rpc.PbExternalReply)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.leaderid().size() > 0) {

    leaderid_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.leaderid_);
  }
  if (from.ret().size() > 0) {

    ret_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.ret_);
  }
}

void PbExternalReply::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:quintet.rpc.PbExternalReply)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void PbExternalReply::CopyFrom(const PbExternalReply& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:quintet.rpc.PbExternalReply)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PbExternalReply::IsInitialized() const {
  return true;
}

void PbExternalReply::Swap(PbExternalReply* other) {
  if (other == this) return;
  InternalSwap(other);
}
void PbExternalReply::InternalSwap(PbExternalReply* other) {
  using std::swap;
  leaderid_.Swap(&other->leaderid_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  ret_.Swap(&other->ret_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::google::protobuf::Metadata PbExternalReply::GetMetadata() const {
  protobuf_Quintet_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_Quintet_2eproto::file_level_metadata[kIndexInFileMessages];
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace rpc
}  // namespace quintet
namespace google {
namespace protobuf {
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::quintet::rpc::PbExternalMessage* Arena::CreateMaybeMessage< ::quintet::rpc::PbExternalMessage >(Arena* arena) {
  return Arena::CreateInternal< ::quintet::rpc::PbExternalMessage >(arena);
}
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::quintet::rpc::PbExternalReply* Arena::CreateMaybeMessage< ::quintet::rpc::PbExternalReply >(Arena* arena) {
  return Arena::CreateInternal< ::quintet::rpc::PbExternalReply >(arena);
}
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
