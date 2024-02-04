#pragma once

#include <array>

template <typename KeyType, typename ValueType, size_t Size>
class FlatMap {
public:
    struct MapIterator {
	public:
		MapIterator(const ValueType* begin, size_t i)
			: begin_(begin)
			, idx(i)
		{ }

		std::pair<KeyType, ValueType&> operator*() {
			return { (KeyType) idx, begin_[idx] };
		}

        std::pair<KeyType, const ValueType&> operator*() const {
			return { (KeyType) idx, begin_[idx] };
		}

		void operator++() {
			idx++;
		}

		bool operator!=(const MapIterator& rhs) const {
			assert(begin_ == rhs.begin_);
			return idx != rhs.idx;
		}

		bool operator==(const MapIterator& rhs) const {
			assert(begin_ == rhs.begin_);
			return idx == rhs.idx;
		}

	private:
		const ValueType* begin_;
		size_t idx;
	};

    FlatMap(const std::array<ValueType, Size>& arr) : arr_(arr) { }

    const ValueType& operator[](const KeyType& key) const { return arr_[static_cast<size_t>(key)]; }
    ValueType& operator[](const KeyType& key) { return arr_[static_cast<size_t>(key)]; }

    const ValueType& at(const KeyType& key) const { return arr_.at(static_cast<size_t>(key)); }
    ValueType& at(const KeyType& key) { return arr_.at(static_cast<size_t>(key)); }

    const MapIterator begin() const { return MapIterator(arr_.data(), 0); }
    const MapIterator end() const { return MapIterator(arr_.data(), Size); }

    MapIterator begin() { return MapIterator(arr_.data(), 0); }
    MapIterator end() { return MapIterator(arr_.data(), Size); }

    const std::array<ValueType, Size>& values() const { return arr_; }
    std::array<ValueType, Size>& values() { return arr_; }

private:
    std::array<ValueType, Size> arr_ = {};
};
