/*
   Copyright 2009-2010 Satoshi Nakamoto
   Copyright 2009-2014 The Bitcoin Core developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <tl/expected.hpp>

#include <zen/core/common/base.hpp>
#include <zen/core/common/secure_bytes.hpp>
#include <zen/core/serialization/base.hpp>

namespace zen::ser {

class DataStream {
  public:
    using size_type = typename SecureBytes::size_type;

    DataStream(Scope scope, int version) : scope_{scope}, version_{version} {};

    [[nodiscard]] Scope scope() const noexcept;
    [[nodiscard]] int version() const noexcept;

    //! \brief Appends provided data to internal buffer
    void write(ByteView data);

    //! \brief Appends provided data to internal buffer
    void write(uint8_t* ptr, size_type count);

    //! \brief Appends a single byte to internal buffer
    void push_back(uint8_t byte);

    //! \brief Returns a view of requested bytes count from the actual read position
    //! \remarks After the view is returned the read position is advanced by count
    [[nodiscard]] tl::expected<ByteView, DeserializationError> read(size_t count);

    //! \brief Advances the read position by count
    //! \remarks If the count of bytes to be skipped exceeds the boundary of the buffer the read position
    //! is moved at the end and eof() will return true
    void skip(size_type count) noexcept;

    //! \brief Whether the end of stream's data has been reached
    [[nodiscard]] bool eof() const noexcept;

    //! \brief Returns the size of the contained data
    [[nodiscard]] size_type size() const noexcept;

    //! \brief Returns the size of yet-to-be-consumed data
    [[nodiscard]] size_type avail() const noexcept;

    //! \brief Clears data and moves the read position to the beginning
    //! \remarks After this operation eof() == true
    void clear() noexcept;

    //! \brief Returns the current read position
    [[nodiscard]] size_type tellp() const noexcept;

    //! \brief Removes the portion of already consumed data (up to read pos)
    void shrink();

    //! \brief Returns the hexed representation of the data buffer
    [[nodiscard]] std::string to_string() const;

  private:
    SecureBytes buffer_{};        // Data buffer
    size_type read_position_{0};  // Current read position;

    Scope scope_;
    int version_{0};
};

}  // namespace zen::ser
