/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/encoding/hex.hpp>
#include <zen/core/serialization/serialize.hpp>
#include <zen/core/serialization/stream.hpp>

namespace zen::ser {

TEST_CASE("Serialization Sizes", "[serialization]") {
    CHECK(ser_sizeof('a') == 1);
    CHECK(ser_sizeof(uint8_t{0}) == 1);
    CHECK(ser_sizeof(int8_t{0}) == 1);
    CHECK(ser_sizeof(uint16_t{0}) == 2);
    CHECK(ser_sizeof(int16_t{0}) == 2);
    CHECK(ser_sizeof(uint32_t{0}) == 4);
    CHECK(ser_sizeof(int32_t{0}) == 4);
    CHECK(ser_sizeof(uint64_t{0}) == 8);
    CHECK(ser_sizeof(int64_t{0}) == 8);
    CHECK(ser_sizeof(float{0}) == 4);
    CHECK(ser_sizeof(double{0}) == 8);
    CHECK(ser_sizeof(bool{true}) == 1);

    // Compact integral
    CHECK(ser_csizeof(0x00) == 1);
    CHECK(ser_csizeof(0xfffa) == 3);
    CHECK(ser_csizeof(256) == 3);
    CHECK(ser_csizeof(0x10003) == 5);
    CHECK(ser_csizeof(0xffffffff) == 5);
    CHECK(ser_csizeof(UINT64_MAX) == 9);
}

TEST_CASE("Serialization of base types", "[serialization]") {
    SECTION("Write Types", "[serialization]") {
        DataStream stream(Scope::kStorage, 0);

        uint8_t u8{0x10};
        write_data(stream, u8);
        CHECK(stream.size() == ser_sizeof(u8));
        uint16_t u16{0x10};
        write_data(stream, u16);
        CHECK(stream.size() == ser_sizeof(u8) + ser_sizeof(u16));
        uint32_t u32{0x10};
        write_data(stream, u32);
        CHECK(stream.size() == ser_sizeof(u8) + ser_sizeof(u16) + ser_sizeof(u32));
        uint64_t u64{0x10};
        write_data(stream, u64);
        CHECK(stream.size() == ser_sizeof(u8) + ser_sizeof(u16) + ser_sizeof(u32) + ser_sizeof(u64));

        float f{1.05f};
        write_data(stream, f);
        CHECK(stream.size() == ser_sizeof(u8) + ser_sizeof(u16) + ser_sizeof(u32) + ser_sizeof(u64) + ser_sizeof(f));

        double d{f * 2};
        write_data(stream, d);
        CHECK(stream.size() ==
              ser_sizeof(u8) + ser_sizeof(u16) + ser_sizeof(u32) + ser_sizeof(u64) + ser_sizeof(f) + ser_sizeof(d));
    }

    SECTION("Floats serialization", "[serialization]") {
        static const double f64v{19880124.0};
        DataStream stream(Scope::kStorage, 0);
        write_data(stream, f64v);
        CHECK(stream.to_string() == "000000c08bf57241");
    }

    SECTION("Write compact") {
        DataStream stream(Scope::kStorage, 0);
        uint64_t value{0};
        write_compact(stream, value);
        CHECK(stream.size() == 1);

        auto read_bytes(stream.read(stream.avail()));
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "00");
        CHECK(stream.eof());

        stream.clear();
        value = 0xfc;
        write_compact(stream, value);
        CHECK(stream.size() == 1);
        read_bytes = stream.read(1);
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "fc");
        CHECK(stream.eof());

        stream.clear();
        value = 0xfffe;
        write_compact(stream, value);
        CHECK(stream.size() == 3);
        read_bytes = stream.read(1);
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "fd");
        CHECK(stream.tellp() == 1);
        read_bytes = stream.read(stream.avail());
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "feff" /*swapped*/);
        CHECK(stream.eof());

        stream.clear();
        value = 0xfffffffe;
        write_compact(stream, value);
        CHECK(stream.size() == 5);
        read_bytes = stream.read(1);
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "fe");
        CHECK(stream.tellp() == 1);
        read_bytes = stream.read(stream.avail());
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "feffffff" /*swapped*/);
        CHECK(stream.eof());

        stream.clear();
        value = 0xffffffffa0;
        write_compact(stream, value);
        CHECK(stream.size() == 9);
        read_bytes = stream.read(1);
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "ff");
        CHECK(stream.tellp() == 1);
        read_bytes = stream.read(stream.avail());
        CHECK(read_bytes);
        CHECK(hex::encode(*read_bytes) == "a0ffffffff000000" /*swapped*/);

        // Try read more bytes than avail
        stream.clear();
        write_compact(stream, value);
        read_bytes = stream.read(stream.avail() + 1);
        CHECK_FALSE(read_bytes);
        CHECK(read_bytes.error() == DeserializationError::kReadBeyondData);
    }

    SECTION("Read Compact", "[serialization]") {
        DataStream stream(Scope::kStorage, 0);
        DataStream::size_type i;

        for (i = 1; i <= kMaxSerializedCompactSize; i *= 2) {
            write_compact(stream, i - 1);
            write_compact(stream, i);
        }

        for (i = 1; i <= kMaxSerializedCompactSize; i *= 2) {
            auto value{read_compact(stream)};
            CHECK(value);
            CHECK(*value == i - 1);
            value = read_compact(stream);
            CHECK(value);
            CHECK(*value == i);
        }
    }

    SECTION("Non Canonical Compact", "[serialization]") {
        DataStream stream(Scope::kStorage, 0);
        auto value{read_compact(stream)};
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kReadBeyondData);

        Bytes data{0xfd, 0x00, 0x00};  // Zero encoded with 3 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        data.assign({0xfd, 0xfc, 0x00});  // 252 encoded with 3 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        data.assign({0xfd, 0xfd, 0x00});  // 253 encoded with 3 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK(value);

        data.assign({0xfe, 0x00, 0x00, 0x00, 0x00});  // Zero encoded with 5 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        data.assign({0xfe, 0xff, 0xff, 0x00, 0x00});  // 0xffff encoded with 5 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        data.assign({0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});  // Zero encoded with 5 bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        data.assign({0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00});  // 0x01ffffff encoded with nine bytes
        stream.write(data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kNonCanonicalCompactSize);

        const uint64_t too_big_value{kMaxSerializedCompactSize + 1};
        Bytes too_big_data(reinterpret_cast<uint8_t*>(&too_big_data), sizeof(too_big_value));
        too_big_data.insert(too_big_data.begin(), 0xff);  // 9 bytes
        stream.write(too_big_data);
        value = read_compact(stream);
        CHECK_FALSE(value);
        CHECK(value.error() == DeserializationError::kCompactSizeTooBig);
    }
}
}  // namespace zen::ser
