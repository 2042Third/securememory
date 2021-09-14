/*
 * Copyright (C) 2015-2021 The ViaDuck Project
 *
 * This file is part of SecureMemory.
 *
 * SecureMemory is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SecureMemory is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SecureMemory.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>

#include <secure_memory/Buffer.h>
#include <secure_memory/BufferRange.h>

Buffer::Buffer(uint32_t reserved) : mData(reserved), mReserved(reserved) { }

Buffer::Buffer(const Buffer &buffer) : mData(buffer.mReserved), mReserved(buffer.mReserved), mUsed(buffer.mUsed), mOffset(0) {
    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(mData().get(), &buffer.mData()[buffer.mOffset], mUsed);
}

Buffer::Buffer(Buffer &&buffer) noexcept : mData(std::move(buffer.mData)), mReserved(buffer.mReserved), mUsed(buffer.mUsed), mOffset(buffer.mOffset) {
    buffer.mReserved = 0;
    buffer.mUsed = 0;
    buffer.mOffset = 0;
}

Buffer::~Buffer() { }

BufferRangeConst Buffer::append(const void *data, uint32_t len) {
    return write(data, len, mUsed);
}

BufferRangeConst Buffer::append(const char *data, uint32_t len) {
    return append(static_cast<const void *>(data), make_si(len) * make_si<uint32_t>(sizeof(char)));
}

BufferRangeConst Buffer::append(const Buffer &other) {
    return append(other.const_data(), other.size());
}

BufferRangeConst Buffer::append(const BufferRangeConst &range) {
    return append(range.const_data(), range.size());
}

BufferRangeConst Buffer::write(const void *data, uint32_t len, uint32_t offset) {
    auto requested = mOffset + make_si(offset) + make_si(len);
    if (requested > mReserved)
        increase(requested + mReserved * 2_si32);

    // now copy new data (if not nullptr)
    if (data != nullptr)
        memcpy(&mData()[mOffset + make_si(offset)], data, len);

    if (make_si(offset) + make_si(len) > mUsed)
        mUsed = make_si(offset) + make_si(len);

    return BufferRangeConst(*this, offset, len);
}

BufferRangeConst Buffer::write(const Buffer &other, uint32_t offset) {
    return write(other.const_data(), other.size(), offset);
}

BufferRangeConst Buffer::write(const BufferRange &other, uint32_t offset) {
    return write(other.const_data(), other.size(), offset);
}

BufferRangeConst Buffer::write(const BufferRangeConst &other, uint32_t offset) {
    return write(other.const_data(), other.size(), offset);
}

void Buffer::consume(uint32_t n) {
    if (n > mUsed)
        n = mUsed;
    mOffset += make_si(n);
    mUsed -= make_si(n);
}

void Buffer::reset(uint32_t offsetDiff) {
    if (offsetDiff > mOffset)
        offsetDiff = 0;
    mUsed += make_si(offsetDiff);
    mOffset -= make_si(offsetDiff);
}

uint32_t Buffer::increase(const uint32_t newCapacity, const bool by) {
    auto capa = make_si(newCapacity);
    if (by)
        capa += mUsed;

    // no need to increase, since buffer is as big as requested
    if (capa <= mReserved - mOffset)
        return mReserved - mOffset;

    // reallocate
    mReserved = capa;
    SecureUniquePtr<uint8_t[]> newData(mReserved);

    // copy whole old buffer into new one. But drop the already skipped bytes (mOffset)
    memcpy(newData().get(), &mData()[mOffset], mUsed);

    mData = std::move(newData);
    mOffset = 0;

    return mReserved;
}

uint32_t Buffer::increase(const uint32_t newCapacity, const uint8_t value, const bool by) {
    uint32_t r = increase(newCapacity, by);
    // initialize with supplied value
    for (uint32_t i = mUsed; i < r; ++i)
        static_cast<uint8_t *>(data())[i] = value;

    return r;
}

void Buffer::padd(const uint32_t offset, const uint32_t size, const uint8_t value) {
    increase(make_si(offset) + make_si(size), value);
    // do not overwrite existing bytes with supplied value (bytes with offset < mUsed). New bytes have been set to value
    // already by increase(..)

    // mark them as used
    if (make_si(offset) + make_si(size) > mUsed)
        use(make_si(offset) + make_si(size) - mUsed);
}

void Buffer::padd(const uint32_t newSize, const uint8_t value) {
    if (newSize > mUsed)
        padd(mUsed, make_si(newSize) - mUsed, value);
}

void Buffer::padd(BufferRange range, uint8_t value) {
    padd(range.offset(), range.size(), value);
}

uint32_t Buffer::size() const {
    return mUsed;
}

void *Buffer::data(uint32_t p) {
    if (p > size())
        p = size();
    return &mData()[mOffset + make_si(p)];
}

const void *Buffer::const_data(uint32_t p) const {
    if (p > size())
        p = size();
    return const_cast<const uint8_t *>(&mData()[mOffset + make_si(p)]);
}

BufferRangeConst Buffer::const_data(uint32_t offset, uint32_t sz) const {
    if (offset > size())
        offset = size();

    if (make_si(offset) + make_si(sz) > size())
        sz = make_si(size()) - make_si(offset);

    return {*this, offset, sz};
}

void Buffer::use(uint32_t n) {
    if ((mReserved - mOffset) >= make_si(n) + mUsed)
        mUsed += make_si(n);
    else
        mUsed = mReserved - mOffset;
}

void Buffer::unuse(uint32_t n) {
    if (n > mUsed)
        n = mUsed;
    mUsed -= make_si(n);
}

void Buffer::clear(bool shred) {
    mUsed = 0;
    mOffset = 0;

    // overwrite memory securely
    if (shred)
        MemoryShredder::shred(mData().get(), mReserved);
}

bool Buffer::operator==(const Buffer &other) const {
    return size() == other.size() && comparisonHelper(const_data(), other.const_data(), this->size());
}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
    mData = std::move(other.mData);
    mReserved = other.mReserved;
    mUsed = other.mUsed;
    mOffset = other.mOffset;

    other.mReserved = 0;
    other.mUsed = 0;
    other.mOffset = 0;

    return *this;
}
