// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_READER_HPP
#define IPTSD_COMMON_READER_HPP

#include "error.hpp"
#include "types.hpp"

#include <gsl/gsl>

#include <algorithm>

namespace iptsd {
namespace impl {

enum class ReaderError : u8 {
	EndOfData,
	InvalidRead,
	InvalidSeek,
};

inline std::string format_as(ReaderError err)
{
	switch (err) {
	case ReaderError::EndOfData:
		return "common: Tried to read {} bytes but no data left!";
	case ReaderError::InvalidRead:
		return "common: Tried to read {} bytes with only {} bytes available!";
	case ReaderError::InvalidSeek:
		return "common: Tried to seek to position {} when {} is the max!";
	default:
		return "Invalid error code!";
	}
}

} // namespace impl

class Reader {
public:
	using Error = impl::ReaderError;

private:
	std::vector<u8> m_buffer {};
	gsl::span<u8> m_data;

	// The current position in the data.
	usize m_index = 0;

public:
	Reader(const gsl::span<u8> data) : m_data {data} {};
	Reader(std::vector<u8> buffer) : m_buffer {std::move(buffer)}, m_data {m_buffer} {};

	/*!
	 * The current position of the reader inside the data.
	 */
	[[nodiscard]] usize index() const
	{
		return m_index;
	}

	/*!
	 * Changes the current position of the reader inside the data.
	 *
	 * @param[in] index The new position. Must be less or equal to the length of the data.
	 */
	void seek(usize index)
	{
		if (index > m_data.size())
			throw common::Error<Error::InvalidSeek> {index, m_data.size()};

		m_index = index;
	}

	/*!
	 * Fills a buffer with the data at the current position.
	 *
	 * @param[in] dest The destination and size of the data.
	 */
	void read(const gsl::span<u8> dest)
	{
		if (this->size() == 0)
			throw common::Error<Error::EndOfData> {dest.size()};

		if (dest.size() > this->size())
			throw common::Error<Error::InvalidRead> {dest.size(), this->size()};

		const gsl::span<u8> src = this->subspan<u8>(dest.size());
		std::copy(src.begin(), src.end(), dest.begin());
	}

	/*!
	 * Moves the current position forward.
	 *
	 * @param[in] size How many bytes to skip.
	 */
	void skip(const usize size)
	{
		if (this->size() == 0)
			throw common::Error<Error::EndOfData> {size};

		if (size > this->size())
			throw common::Error<Error::InvalidRead> {size, this->size()};

		m_index += size;
	}

	/*!
	 * How many bytes are left in the data.
	 *
	 * @return The amount of bytes that have not been read.
	 */
	[[nodiscard]] usize size() const
	{
		return m_data.size() - m_index;
	}

	/*!
	 * Takes a chunk of bytes from the current position and splits it off.
	 *
	 * @param[in] size How many bytes to take.
	 * @return The raw chunk of data.
	 */
	template <class T>
	gsl::span<T> subspan(const usize size)
	{
		if (this->size() == 0)
			throw common::Error<Error::EndOfData> {size};

		if (size > this->size())
			throw common::Error<Error::InvalidRead> {size, this->size()};

		const usize bytes = size * sizeof(T);
		const gsl::span<u8> sub = m_data.subspan(m_index, bytes);

		this->skip(bytes);

		// We have to break type safety here, since all we have is a bytestream.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		return gsl::span<T> {reinterpret_cast<T *>(sub.data()), size};
	}

	/*!
	 * Takes a chunk of bytes from the current position and splits it off.
	 *
	 * @param[in] size How many bytes to take.
	 * @return A new reader instance for the chunk of data.
	 */
	Reader sub(const usize size)
	{
		return Reader {this->subspan<u8>(size)};
	}

	/*!
	 * Reads an object from the current position.
	 *
	 * @tparam T The type (and size) of the object to read.
	 * @return The object that was read.
	 */
	template <class T>
	T read()
	{
		T value {};

		// We have to break type safety here, since all we have is a bytestream.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		this->read(gsl::span {reinterpret_cast<u8 *>(&value), sizeof(value)});

		return value;
	}
};

} // namespace iptsd

#endif // IPTSD_COMMON_READER_HPP
