/*
   Copyright 2009-2010 Satoshi Nakamoto
   Copyright 2009-2013 The Bitcoin Core developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <algorithm>

#include <gsl/gsl_util>

#include <core/abi/netmessage.hpp>
#include <core/common/misc.hpp>

namespace zenpp::abi {

void NetMessageHeader::reset() noexcept {
    network_magic.fill('0');
    command.fill('0');
    payload_length = 0;
    payload_checksum.fill('0');
    message_type_ = NetMessageType::kMissingOrUnknown;
}

bool NetMessageHeader::pristine() const noexcept {
    return std::all_of(network_magic.begin(), network_magic.end(), [](const auto b) { return b == 0; }) &&
           std::all_of(command.begin(), command.end(), [](const auto b) { return b == 0; }) &&
           std::all_of(payload_checksum.begin(), payload_checksum.end(), [](const auto b) { return b == 0; }) &&
           message_type_ == NetMessageType::kMissingOrUnknown && payload_length == 0;
}

void NetMessageHeader::set_type(const NetMessageType type) noexcept {
    if (!pristine()) return;
    const auto& message_definition{kMessageDefinitions[static_cast<size_t>(type)]};
    const auto command_bytes{strnlen_s(message_definition.command, command.size())};
    memcpy(command.data(), message_definition.command, command_bytes);
    message_type_ = type;
}

serialization::Error NetMessageHeader::serialization(serialization::SDataStream& stream, serialization::Action action) {
    using namespace serialization;
    using enum Error;
    Error err{Error::kSuccess};
    if (!err) err = stream.bind(network_magic, action);
    if (!err) err = stream.bind(command, action);
    if (!err) err = stream.bind(payload_length, action);
    if (!err) err = stream.bind(payload_checksum, action);
    if (!err && action == Action::kDeserialize) err = validate();
    return err;
}

serialization::Error NetMessageHeader::validate() noexcept {
    using namespace serialization;
    using enum Error;
    if (payload_length > kMaxProtocolMessageLength) return kMessageHeaderOversizedPayload;

    // Check the command string is made of printable characters
    // eventually right padded to 12 bytes with NUL (0x00) characters.
    bool null_terminator_matched{false};
    size_t got_command_len{0};
    for (const auto c : command) {
        if (null_terminator_matched && c != 0) return kMessageHeaderMalformedCommand;
        if (c == 0) {
            null_terminator_matched = true;
            continue;
        }
        if (c < 32 || c > 126) return kMessageHeaderMalformedCommand;
        ++got_command_len;
    }
    if (!got_command_len) return kMessageHeaderEmptyCommand;

    // Identify the command amongst the known ones
    int id{0};
    for (const auto& msg_def : kMessageDefinitions) {
        const auto def_command_len{strnlen_s(msg_def.command, command.size())};
        if (got_command_len == def_command_len && memcmp(msg_def.command, command.data(), def_command_len) == 0) {
            message_type_ = static_cast<NetMessageType>(id);
            break;
        }
        ++id;
    }

    if (message_type_ == NetMessageType::kMissingOrUnknown) return kMessageHeaderUnknownCommand;

    const auto& message_definition{get_definition()};
    if (message_definition.min_payload_length.has_value() && payload_length < *message_definition.min_payload_length) {
        return kMessageHeaderUndersizedPayload;
    }
    if (message_definition.max_payload_length.has_value() && payload_length > *message_definition.max_payload_length) {
        return kMessageHeaderOversizedPayload;
    }

    if (payload_length == 0) /* Hash of empty payload is already known */
    {
        auto empty_payload_hash{crypto::Hash256::kEmptyHash()};
        if (memcmp(payload_checksum.data(), empty_payload_hash.data(), payload_checksum.size()) != 0)
            return kMessageHeaderInvalidChecksum;
    }

    return kSuccess;
}

serialization::Error NetMessage::validate() noexcept {
    using enum serialization::Error;

    if (ser_stream_.size() < kMessageHeaderLength) return kMessageHeaderIncomplete;
    if (ser_stream_.size() > kMaxProtocolMessageLength) return kMessageHeaderOversizedPayload;

    const auto& message_definition(header_.get_definition());
    if (message_definition.message_type == NetMessageType::kMissingOrUnknown) return kMessageHeaderUnknownCommand;

    if (ser_stream_.size() < kMessageHeaderLength) return kMessageHeaderIncomplete;
    if (ser_stream_.size() < kMessageHeaderLength + header_.payload_length) return kMessageBodyIncomplete;
    if (ser_stream_.size() > kMessageHeaderLength + header_.payload_length) return kMessageMismatchingPayloadLength;

    // From here on ensure we return to the beginning of the payload
    const auto data_to_payload{gsl::finally([this] { ser_stream_.seekg(kMessageHeaderLength); })};

    // Validate payload : length and checksum
    ser_stream_.seekg(kMessageHeaderLength);  // Important : skip the header !!!
    if (ser_stream_.avail() != header_.payload_length) return kMessageMismatchingPayloadLength;
    auto payload_view{ser_stream_.read()};
    if (!payload_view) return payload_view.error();
    if (auto error{validate_checksum()}; error != kSuccess) return error;

    // For specific messages the vectorized data size can be known in advance
    // e.g. inventory messages are made of 36 bytes elements hence, after the initial
    // read of the vector size the payload size can be checked against the expected size
    if (message_definition.is_vectorized) {
        ser_stream_.seekg(kMessageHeaderLength);
        const auto vector_size{serialization::read_compact(ser_stream_)};
        if (!vector_size) return vector_size.error();
        if (*vector_size == 0) return kMessagePayloadEmptyVector;  // MUST have at least 1 element
        if (*vector_size > message_definition.max_vector_items.value_or(UINT32_MAX))
            return kMessagePayloadOversizedVector;
        if (message_definition.vector_item_size.has_value()) {
            const auto expected_vector_size{*vector_size * *message_definition.vector_item_size};
            if (ser_stream_.avail() != expected_vector_size) return kMessagePayloadMismatchesVectorSize;
            // Look for duplicates
            payload_view = ser_stream_.read();
            ASSERT(payload_view);
            if (const auto duplicate_count{count_duplicate_data_chunks(
                    *payload_view, *message_definition.vector_item_size, 1 /* one is enough */)};
                duplicate_count > 0) {
                return kMessagePayloadDuplicateVectorItems;
            }
        }
    }

    return kSuccess;
}

serialization::Error NetMessage::parse(ByteView& input_data, ByteView network_magic) noexcept {
    using namespace serialization;
    using enum Error;

    Error ret{kSuccess};
    while (!input_data.empty()) {
        const bool header_mode(ser_stream_.tellg() < kMessageHeaderLength);
        auto bytes_to_read(header_mode ? kMessageHeaderLength - ser_stream_.avail()
                                       : header_.payload_length - ser_stream_.avail());
        if (bytes_to_read > input_data.size()) bytes_to_read = input_data.size();
        ser_stream_.write(input_data.substr(0, bytes_to_read));
        input_data.remove_prefix(bytes_to_read);

        if (header_mode) {
            if (ser_stream_.avail() < kMessageHeaderLength) {
                ret = kMessageHeaderIncomplete;  // Not enough data for a full header
                break;                           // All data consumed nevertheless
            }

            ret = header_.deserialize(ser_stream_);
            if (ret == kSuccess) {

                if (!network_magic.empty()) {
                    REQUIRES(header_.network_magic.size() == network_magic.size());
                    if (memcmp(header_.network_magic.data(), network_magic.data(), network_magic.size()) != 0) {
                        ret = kMessageHeaderMagicMismatch;
                    }
                }

                const auto& message_definition{header_.get_definition()};
                if (message_definition.min_protocol_version.has_value() &&
                    ser_stream_.get_version() < *message_definition.min_protocol_version) {
                    ret = kUnsupportedMessageTypeForProtocolVersion;
                }
                if (message_definition.max_protocol_version.has_value() &&
                    ser_stream_.get_version() > *message_definition.max_protocol_version) {
                    ret = kDeprecatedMessageTypeForProtocolVersion;
                }
            }
            if (ret == kSuccess) ret = header_.validate(/* TODO Network magic here */);
            if (ret == kSuccess) {
                if (header_.payload_length == 0) return validate_checksum();  // No payload to read
                continue;                                                     // Keep reading the body payload - if any
            }
            break;  // Exit on any error - here are all fatal

        } else {
            if (ser_stream_.avail() < header_.payload_length) {
                ret = kMessageBodyIncomplete;  // Not enough data for a full body
                break;                         // All data consumed nevertheless
            }
            ret = validate();  // Validate the whole payload of the message
            break;             // Exit anyway as either there is an or we have consumed all input data
        }
    }

    return ret;
}

serialization::Error NetMessage::validate_checksum() noexcept {
    using enum serialization::Error;
    const auto current_pos{ser_stream_.tellg()};
    if (ser_stream_.seekg(kMessageHeaderLength) != kMessageHeaderLength) return kMessageHeaderIncomplete;
    const auto data_to_payload{gsl::finally([this, current_pos] { std::ignore = ser_stream_.seekg(current_pos); })};

    const auto payload_view{ser_stream_.read()};
    if (!payload_view) return payload_view.error();

    serialization::Error ret{kSuccess};
    crypto::Hash256 payload_digest(*payload_view);
    if (auto payload_hash{payload_digest.finalize()};
        memcmp(payload_hash.data(), header_.payload_checksum.data(), header_.payload_checksum.size()) != 0) {
        ret = kMessageHeaderInvalidChecksum;
    }
    return ret;
}

void NetMessage::set_version(int version) noexcept { ser_stream_.set_version(version); }

int NetMessage::get_version() const noexcept { return ser_stream_.get_version(); }

serialization::Error NetMessage::push(const NetMessageType message_type, serialization::Serializable& payload,
                                      ByteView magic) noexcept {
    using namespace serialization;
    using enum Error;

    if (message_type == NetMessageType::kMissingOrUnknown) return kMessageHeaderUnknownCommand;
    if (magic.size() != header_.network_magic.size()) return kMessageHeaderMagicMismatch;
    if (!header_.pristine()) return kInvalidMessageState;
    header_.set_type(message_type);
    std::memcpy(header_.network_magic.data(), magic.data(), header_.network_magic.size());

    ser_stream_.clear();
    auto err{header_.serialize(ser_stream_)};
    if (!!err) return err;
    ASSERT(ser_stream_.size() == kMessageHeaderLength);

    err = payload.serialize(ser_stream_);
    if (!!err) return err;

    header_.payload_length = static_cast<uint32_t>(ser_stream_.size() - kMessageHeaderLength);

    // Compute the checksum
    ser_stream_.seekg(kMessageHeaderLength);  // Move at the beginning of the payload
    const auto payload_view{ser_stream_.read()};
    if (!payload_view) return payload_view.error();
    crypto::Hash256 payload_digest(*payload_view);
    auto payload_hash{payload_digest.finalize()};
    std::memcpy(header_.payload_checksum.data(), payload_hash.data(), header_.payload_checksum.size());

    // Now copy the lazily computed size and checksum into the datastream
    memcpy(&ser_stream_[16], &header_.payload_length, sizeof(header_.payload_length));
    memcpy(&ser_stream_[20], &header_.payload_checksum, sizeof(header_.payload_checksum));

    return validate();  // Ensure the message is valid also when we push it
}

serialization::Error NetMessage::push(NetMessageType message_type, serialization::Serializable& payload,
                                      std::array<uint8_t, 4>& magic) noexcept {
    ByteView magic_view{magic.data(), magic.size()};
    return push(message_type, payload, magic_view);
}
}  // namespace zenpp::abi
