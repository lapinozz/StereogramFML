#pragma once

#include <vector>
#include <cinttypes>
#include <algorithm>
#include <iterator>

template <typename T>
class MappedVector
{
    public:

        struct Storage
        {
            bool initialized = false;
            char value[sizeof(T)];
        };

        class iterator
        {
            friend MappedVector;

            public:
                typedef iterator self_type;
                typedef T value_type;
                typedef T& reference;
                typedef T* pointer;
                typedef std::forward_iterator_tag iterator_category;
                typedef int difference_type;
                iterator(Storage* ptr, Storage* end) : ptr_(ptr), end_(end) { }
                self_type operator++(int junk) { do ptr_++; while(!ptr_->initialized && ptr_ != end_); return *this; }
                self_type operator++() { self_type i = *this; do ptr_++; while(!ptr_->initialized && ptr_!= end_); return i; }
                reference operator*() { return *(T*)(ptr_->value); }
                pointer operator->() { return (T*)(ptr_->value); }
                bool operator==(const self_type& rhs) const { return ptr_ == rhs.ptr_; }
                bool operator!=(const self_type& rhs) const { return ptr_ != rhs.ptr_; }
            private:
                Storage* ptr_;
                Storage* end_;
        };

        class const_iterator
        {
            friend MappedVector;

            public:
                typedef const_iterator self_type;
                typedef T value_type;
                typedef T& reference;
                typedef T* pointer;
                typedef int difference_type;
                typedef std::forward_iterator_tag iterator_category;
//                const_iterator(const iterator& it) : ptr_(it.ptr_), end_(it.end_) { }
                const_iterator(pointer* ptr, pointer* end) : ptr_(ptr), end_(end) { }
                self_type operator++() { self_type i = *this; do ptr_++; while(!ptr_->initialized && ptr_ != end_); return i; }
                self_type operator++(int junk) { do ptr_++; while(!ptr_->initialized && ptr_!= end_); return *this; }
                const reference operator*() { return *(T*)(ptr_->value); }
                const pointer operator->() { return (T*)(ptr_->value); }
                bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
                bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
            private:
                Storage* ptr_;
                Storage* end_;
        };

        iterator begin()
        {
            return iterator(mBuffer.data(), mBuffer.data() + mBuffer.size());
        }

        iterator end()
        {
            return iterator(mBuffer.data() + mBuffer.size(), mBuffer.data() + mBuffer.size());
        }

        const_iterator begin() const
        {
            return const_iterator(mBuffer.data(), mBuffer.data() + mBuffer.size());
        }

        const_iterator end() const
        {
            return const_iterator(mBuffer.data() + mBuffer.size(), mBuffer.data() + mBuffer.size());
        }

        using IdType = int32_t;

        T& operator[](const IdType id)
        {
            return *(T*)&mBuffer[id].value;
        }

        const T& operator[](const IdType id) const
        {
            return *(T*)&mBuffer[id].value;
        }

        IdType insert(const T& t)
        {
            if(mFreeId.empty())
            {
                mBuffer.push_back({});
                new (&mBuffer.back().value) T(t);
                mBuffer.back().initialized = true;

                return mBuffer.size() - 1;
            }
            else
            {
                const IdType id = mFreeId.back();
                mFreeId.pop_back();

                new (&mBuffer[id].value) T(t);
                mBuffer[id].initialized = true;

                return id;
            }
        }

        template <typename...Args>
        IdType emplace(Args...args)
        {
            if(mFreeId.empty())
            {
                mBuffer.push_back({});
                new (&mBuffer.back().value) T(args...);
                mBuffer.back().initialized = true;

                return mBuffer.size() - 1;
            }
            else
            {
                const IdType id = mFreeId.back();
                mFreeId.pop_back();

                mBuffer.push_back({});
                new (&mBuffer.back.value) T(args...);
                mBuffer[id].initialized = true;

                return id;
            }
        }

        void remove(const IdType id)
        {
            if(mBuffer[id].initialized)
            {
                ((T*)&mBuffer[id].value)->T::~T();
                mBuffer[id].initialized = false;

                if(id + 1 == mBuffer.size())
                    mBuffer.resize(id);
                else
                    mFreeId.push_back(id);
            }
        }

        void remove(const iterator& it)
        {
            if(it == end())
                return;

//            auto id = (it.ptr_ - mBuffer.data()) / sizeof(Storage);
            auto id = it.ptr_ - mBuffer.data();

            if(id < mBuffer.size() && mBuffer[id].initialized)
            {
                ((T*)&mBuffer[id].value)->T::~T();
                mBuffer[id].initialized = false;

                if(id + 1 == mBuffer.size())
                    mBuffer.resize(id);
                else
                    mFreeId.push_back(id);
            }
        }

        virtual ~MappedVector()
        {
            for(const auto storage : mBuffer)
                if(storage.initialized)
                    ((T*)&storage.value)->T::~T();
        }

        MappedVector() = default;
        MappedVector(const MappedVector&) = delete;

    private:
        std::vector<Storage> mBuffer;
        std::vector<IdType> mFreeId;
};

//#pragma once
//
//#include <vector>
//#include <cinttypes>
//#include <algorithm>
//#include <iterator>
//
//template <typename T>
//class MappedVector
//{
//    public:
//
//        class Storage
//        {
//            bool initialized = false;
//            char value[sizeof(T)];
//        };
//
//        class iterator
//        {
//            public:
//                typedef iterator self_type;
//                typedef T value_type;
//                typedef T& reference;
//                typedef T* pointer;
//                typedef std::forward_iterator_tag iterator_category;
//                typedef int difference_type;
//                iterator(pointer* ptr, pointer* end) : ptr_(ptr), end_(end) { }
//                self_type operator++() { self_type i = *this; while(!(*ptr_) && ptr_ <= end_) ptr_++; return i; }
//                self_type operator++(int junk) { while(!(*ptr_) && ptr_ != end_)ptr_++;; return *this; }
//                reference operator*() { return **ptr_; }
//                pointer operator->() { return *ptr_; }
//                bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
//                bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
//            private:
//                pointer* ptr_;
//                pointer* end_;
//        };
//
//        class const_iterator
//        {
//            public:
//                typedef const_iterator self_type;
//                typedef T value_type;
//                typedef T& reference;
//                typedef T* pointer;
//                typedef int difference_type;
//                typedef std::forward_iterator_tag iterator_category;
//                const_iterator(pointer* ptr, pointer* end) : ptr_(ptr), end_(end) { }
//                self_type operator++() { self_type i = *this; while(!*ptr_ && ptr_ <= end_)ptr_++; return i; }
//                self_type operator++(int junk) { while(!*ptr_ && ptr_ != end_)ptr_++; return *this; }
//                const reference operator*() { return **ptr_; }
//                const pointer operator->() { return *ptr_; }
//                bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
//                bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
//            private:
//                pointer* ptr_;
//                pointer* end_;
//        };
//
//        iterator begin()
//        {
//            return iterator(mBuffer.data(), mBuffer.data() + mBuffer.size());
//        }
//
//        iterator end()
//        {
//            return iterator(mBuffer.data() + mBuffer.size(), mBuffer.data() + mBuffer.size());
//        }
//
//        const_iterator begin() const
//        {
//            return const_iterator(mBuffer.data(), mBuffer.data() + mBuffer.size());
//        }
//
//        const_iterator end() const
//        {
//            return const_iterator(mBuffer.data() + mBuffer.size(), mBuffer.data() + mBuffer.size());
//        }
//
//        using IdType = int32_t;
//
//        T& operator[](const IdType id)
//        {
//            return *mBuffer[id];
//        }
//
//        const T& operator[](const IdType id) const
//        {
//            return *mBuffer[id];
//        }
//
//        IdType insert(const T& t)
//        {
//            if(mFreeId.empty())
//            {
//                mBuffer.push_back(new T(t));
//                return mBuffer.size() - 1;
//            }
//            else
//            {
//                const IdType id = mFreeId.back();
//                mFreeId.pop_back();
//
//                mBuffer[id] = new T(t);
//                return id;
//            }
//        }
//
//        template <typename...Args>
//        IdType emplace(Args...args)
//        {
//            if(mFreeId.empty())
//            {
//                mBuffer.push_back(new T(args...));
//                return mBuffer.size() - 1;
//            }
//            else
//            {
//                const IdType id = mFreeId.back();
//                mFreeId.pop_back();
//
//                mBuffer[id] = new T(args...);
//                return id;
//            }
//        }
//
//        void remove(const IdType id)
//        {
//            if(mBuffer[id])
//            {
//                delete mBuffer[id];
//                mBuffer[id] = nullptr;
//
//                if(id + 1 == mBuffer.size())
//                    mBuffer.resize(id);
//                else
//                    mFreeId.push_back(id);
//            }
//        }
//
//        virtual ~MappedVector()
//        {
//            for(const auto ptr : mBuffer)
//                if(ptr)
//                    delete ptr;
//        }
//
//        MappedVector() = default;
//        MappedVector(const MappedVector&) = delete;
//
//    private:
//
////        std::vector<T*> mBuffer;
//        std::vector<Storage> mBuffer;
//        std::vector<IdType> mFreeId;
//};
//

