// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: testing_service.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_testing_5fservice_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_testing_5fservice_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3019000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3019002 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_testing_5fservice_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_testing_5fservice_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_testing_5fservice_2eproto;
namespace ni {
namespace grpc_json_client {
class UnaryUnaryEchoMessage;
struct UnaryUnaryEchoMessageDefaultTypeInternal;
extern UnaryUnaryEchoMessageDefaultTypeInternal _UnaryUnaryEchoMessage_default_instance_;
}  // namespace grpc_json_client
}  // namespace ni
PROTOBUF_NAMESPACE_OPEN
template<> ::ni::grpc_json_client::UnaryUnaryEchoMessage* Arena::CreateMaybeMessage<::ni::grpc_json_client::UnaryUnaryEchoMessage>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace ni {
namespace grpc_json_client {

// ===================================================================

class UnaryUnaryEchoMessage final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:ni.grpc_json_client.UnaryUnaryEchoMessage) */ {
 public:
  inline UnaryUnaryEchoMessage() : UnaryUnaryEchoMessage(nullptr) {}
  ~UnaryUnaryEchoMessage() override;
  explicit constexpr UnaryUnaryEchoMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  UnaryUnaryEchoMessage(const UnaryUnaryEchoMessage& from);
  UnaryUnaryEchoMessage(UnaryUnaryEchoMessage&& from) noexcept
    : UnaryUnaryEchoMessage() {
    *this = ::std::move(from);
  }

  inline UnaryUnaryEchoMessage& operator=(const UnaryUnaryEchoMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline UnaryUnaryEchoMessage& operator=(UnaryUnaryEchoMessage&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const UnaryUnaryEchoMessage& default_instance() {
    return *internal_default_instance();
  }
  enum FieldTypesCase {
    kStringField = 2,
    kInt32Field = 3,
    kDoubleField = 4,
    FIELD_TYPES_NOT_SET = 0,
  };

  static inline const UnaryUnaryEchoMessage* internal_default_instance() {
    return reinterpret_cast<const UnaryUnaryEchoMessage*>(
               &_UnaryUnaryEchoMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(UnaryUnaryEchoMessage& a, UnaryUnaryEchoMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(UnaryUnaryEchoMessage* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(UnaryUnaryEchoMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  UnaryUnaryEchoMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<UnaryUnaryEchoMessage>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const UnaryUnaryEchoMessage& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const UnaryUnaryEchoMessage& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to, const ::PROTOBUF_NAMESPACE_ID::Message& from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(UnaryUnaryEchoMessage* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "ni.grpc_json_client.UnaryUnaryEchoMessage";
  }
  protected:
  explicit UnaryUnaryEchoMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kDelayFieldNumber = 1,
    kStringFieldFieldNumber = 2,
    kInt32FieldFieldNumber = 3,
    kDoubleFieldFieldNumber = 4,
  };
  // int32 delay = 1;
  void clear_delay();
  int32_t delay() const;
  void set_delay(int32_t value);
  private:
  int32_t _internal_delay() const;
  void _internal_set_delay(int32_t value);
  public:

  // string string_field = 2;
  bool has_string_field() const;
  private:
  bool _internal_has_string_field() const;
  public:
  void clear_string_field();
  const std::string& string_field() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_string_field(ArgT0&& arg0, ArgT... args);
  std::string* mutable_string_field();
  PROTOBUF_NODISCARD std::string* release_string_field();
  void set_allocated_string_field(std::string* string_field);
  private:
  const std::string& _internal_string_field() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_string_field(const std::string& value);
  std::string* _internal_mutable_string_field();
  public:

  // int32 int32_field = 3;
  bool has_int32_field() const;
  private:
  bool _internal_has_int32_field() const;
  public:
  void clear_int32_field();
  int32_t int32_field() const;
  void set_int32_field(int32_t value);
  private:
  int32_t _internal_int32_field() const;
  void _internal_set_int32_field(int32_t value);
  public:

  // double double_field = 4;
  bool has_double_field() const;
  private:
  bool _internal_has_double_field() const;
  public:
  void clear_double_field();
  double double_field() const;
  void set_double_field(double value);
  private:
  double _internal_double_field() const;
  void _internal_set_double_field(double value);
  public:

  void clear_field_types();
  FieldTypesCase field_types_case() const;
  // @@protoc_insertion_point(class_scope:ni.grpc_json_client.UnaryUnaryEchoMessage)
 private:
  class _Internal;
  void set_has_string_field();
  void set_has_int32_field();
  void set_has_double_field();

  inline bool has_field_types() const;
  inline void clear_has_field_types();

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  int32_t delay_;
  union FieldTypesUnion {
    constexpr FieldTypesUnion() : _constinit_{} {}
      ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized _constinit_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr string_field_;
    int32_t int32_field_;
    double double_field_;
  } field_types_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  uint32_t _oneof_case_[1];

  friend struct ::TableStruct_testing_5fservice_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// UnaryUnaryEchoMessage

// int32 delay = 1;
inline void UnaryUnaryEchoMessage::clear_delay() {
  delay_ = 0;
}
inline int32_t UnaryUnaryEchoMessage::_internal_delay() const {
  return delay_;
}
inline int32_t UnaryUnaryEchoMessage::delay() const {
  // @@protoc_insertion_point(field_get:ni.grpc_json_client.UnaryUnaryEchoMessage.delay)
  return _internal_delay();
}
inline void UnaryUnaryEchoMessage::_internal_set_delay(int32_t value) {
  
  delay_ = value;
}
inline void UnaryUnaryEchoMessage::set_delay(int32_t value) {
  _internal_set_delay(value);
  // @@protoc_insertion_point(field_set:ni.grpc_json_client.UnaryUnaryEchoMessage.delay)
}

// string string_field = 2;
inline bool UnaryUnaryEchoMessage::_internal_has_string_field() const {
  return field_types_case() == kStringField;
}
inline bool UnaryUnaryEchoMessage::has_string_field() const {
  return _internal_has_string_field();
}
inline void UnaryUnaryEchoMessage::set_has_string_field() {
  _oneof_case_[0] = kStringField;
}
inline void UnaryUnaryEchoMessage::clear_string_field() {
  if (_internal_has_string_field()) {
    field_types_.string_field_.Destroy(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
    clear_has_field_types();
  }
}
inline const std::string& UnaryUnaryEchoMessage::string_field() const {
  // @@protoc_insertion_point(field_get:ni.grpc_json_client.UnaryUnaryEchoMessage.string_field)
  return _internal_string_field();
}
template <typename ArgT0, typename... ArgT>
inline void UnaryUnaryEchoMessage::set_string_field(ArgT0&& arg0, ArgT... args) {
  if (!_internal_has_string_field()) {
    clear_field_types();
    set_has_string_field();
    field_types_.string_field_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  }
  field_types_.string_field_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:ni.grpc_json_client.UnaryUnaryEchoMessage.string_field)
}
inline std::string* UnaryUnaryEchoMessage::mutable_string_field() {
  std::string* _s = _internal_mutable_string_field();
  // @@protoc_insertion_point(field_mutable:ni.grpc_json_client.UnaryUnaryEchoMessage.string_field)
  return _s;
}
inline const std::string& UnaryUnaryEchoMessage::_internal_string_field() const {
  if (_internal_has_string_field()) {
    return field_types_.string_field_.Get();
  }
  return ::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited();
}
inline void UnaryUnaryEchoMessage::_internal_set_string_field(const std::string& value) {
  if (!_internal_has_string_field()) {
    clear_field_types();
    set_has_string_field();
    field_types_.string_field_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  }
  field_types_.string_field_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* UnaryUnaryEchoMessage::_internal_mutable_string_field() {
  if (!_internal_has_string_field()) {
    clear_field_types();
    set_has_string_field();
    field_types_.string_field_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  }
  return field_types_.string_field_.Mutable(
      ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* UnaryUnaryEchoMessage::release_string_field() {
  // @@protoc_insertion_point(field_release:ni.grpc_json_client.UnaryUnaryEchoMessage.string_field)
  if (_internal_has_string_field()) {
    clear_has_field_types();
    return field_types_.string_field_.ReleaseNonDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
  } else {
    return nullptr;
  }
}
inline void UnaryUnaryEchoMessage::set_allocated_string_field(std::string* string_field) {
  if (has_field_types()) {
    clear_field_types();
  }
  if (string_field != nullptr) {
    set_has_string_field();
    field_types_.string_field_.UnsafeSetDefault(string_field);
    ::PROTOBUF_NAMESPACE_ID::Arena* arena = GetArenaForAllocation();
    if (arena != nullptr) {
      arena->Own(string_field);
    }
  }
  // @@protoc_insertion_point(field_set_allocated:ni.grpc_json_client.UnaryUnaryEchoMessage.string_field)
}

// int32 int32_field = 3;
inline bool UnaryUnaryEchoMessage::_internal_has_int32_field() const {
  return field_types_case() == kInt32Field;
}
inline bool UnaryUnaryEchoMessage::has_int32_field() const {
  return _internal_has_int32_field();
}
inline void UnaryUnaryEchoMessage::set_has_int32_field() {
  _oneof_case_[0] = kInt32Field;
}
inline void UnaryUnaryEchoMessage::clear_int32_field() {
  if (_internal_has_int32_field()) {
    field_types_.int32_field_ = 0;
    clear_has_field_types();
  }
}
inline int32_t UnaryUnaryEchoMessage::_internal_int32_field() const {
  if (_internal_has_int32_field()) {
    return field_types_.int32_field_;
  }
  return 0;
}
inline void UnaryUnaryEchoMessage::_internal_set_int32_field(int32_t value) {
  if (!_internal_has_int32_field()) {
    clear_field_types();
    set_has_int32_field();
  }
  field_types_.int32_field_ = value;
}
inline int32_t UnaryUnaryEchoMessage::int32_field() const {
  // @@protoc_insertion_point(field_get:ni.grpc_json_client.UnaryUnaryEchoMessage.int32_field)
  return _internal_int32_field();
}
inline void UnaryUnaryEchoMessage::set_int32_field(int32_t value) {
  _internal_set_int32_field(value);
  // @@protoc_insertion_point(field_set:ni.grpc_json_client.UnaryUnaryEchoMessage.int32_field)
}

// double double_field = 4;
inline bool UnaryUnaryEchoMessage::_internal_has_double_field() const {
  return field_types_case() == kDoubleField;
}
inline bool UnaryUnaryEchoMessage::has_double_field() const {
  return _internal_has_double_field();
}
inline void UnaryUnaryEchoMessage::set_has_double_field() {
  _oneof_case_[0] = kDoubleField;
}
inline void UnaryUnaryEchoMessage::clear_double_field() {
  if (_internal_has_double_field()) {
    field_types_.double_field_ = 0;
    clear_has_field_types();
  }
}
inline double UnaryUnaryEchoMessage::_internal_double_field() const {
  if (_internal_has_double_field()) {
    return field_types_.double_field_;
  }
  return 0;
}
inline void UnaryUnaryEchoMessage::_internal_set_double_field(double value) {
  if (!_internal_has_double_field()) {
    clear_field_types();
    set_has_double_field();
  }
  field_types_.double_field_ = value;
}
inline double UnaryUnaryEchoMessage::double_field() const {
  // @@protoc_insertion_point(field_get:ni.grpc_json_client.UnaryUnaryEchoMessage.double_field)
  return _internal_double_field();
}
inline void UnaryUnaryEchoMessage::set_double_field(double value) {
  _internal_set_double_field(value);
  // @@protoc_insertion_point(field_set:ni.grpc_json_client.UnaryUnaryEchoMessage.double_field)
}

inline bool UnaryUnaryEchoMessage::has_field_types() const {
  return field_types_case() != FIELD_TYPES_NOT_SET;
}
inline void UnaryUnaryEchoMessage::clear_has_field_types() {
  _oneof_case_[0] = FIELD_TYPES_NOT_SET;
}
inline UnaryUnaryEchoMessage::FieldTypesCase UnaryUnaryEchoMessage::field_types_case() const {
  return UnaryUnaryEchoMessage::FieldTypesCase(_oneof_case_[0]);
}
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace grpc_json_client
}  // namespace ni

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_testing_5fservice_2eproto
