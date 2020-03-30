/*
 * Copyright (C) 2015-2018 The ViaDuck Project
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

#ifndef SECUREMEMORY_BUFFERRANGE_H
#define SECUREMEMORY_BUFFERRANGE_H

#include <string>

#include "Range.h"

class Buffer;
typedef Range<Buffer> BufferRange;
typedef Range<const Buffer> BufferRangeConst;

#include "Buffer.h"

// taken from https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std {

    /**
     * Implement hash function for Range class (so that Range is usable in a hash map)
     */
    template<>
    struct hash<const BufferRangeConst> {
        __attribute__((no_sanitize("integer"))) std::size_t operator()(const BufferRangeConst &k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            // taken from https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
            size_t current = 0;
            for (uint32_t a = 0; a < k.size(); a++)
                current ^=
                        static_cast<const uint8_t *>(k.const_object().const_data())[k.offset() + a] + 0x9e3779b9 +
                        (current << 6) + (current >> 2);
            return current;
        }
    };
}


#endif //SECUREMEMORY_BUFFERRANGE_H
