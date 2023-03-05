#include <memory>
#include <vector>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> class HashMap
{
private:
    class Node
    {
    public:
        std::pair<const KeyType, ValueType> pair;
        Node *l = nullptr;
        Node *r = nullptr;
        size_t displace = 0;
        explicit Node(std::pair<const KeyType, ValueType> pair_): pair(pair_), l(nullptr), r(nullptr)
        {

        }


        Node(): pair(std::make_pair(KeyType(), ValueType())), l(nullptr), r(nullptr)
        {

        }
    };
    std::vector<Node*> _data;

    Node* _ptrBegin = nullptr;
    Node* _ptrEnd = nullptr;

    size_t _loadFactor = 2;
    size_t _curSize = 0;
    size_t _maxSize = 2;
    Hash _mapHash;
public:
    explicit HashMap(Hash mapHash = Hash()) : _mapHash(mapHash)
    {
        _data.assign(_maxSize, nullptr);
    }


    template<class TempIterator>
    HashMap(TempIterator iteratorBegin, TempIterator iteratorEnd, Hash mapHash = Hash()) : _mapHash(mapHash)
    {
        _data.assign(_maxSize, nullptr);
        while (iteratorBegin != iteratorEnd)
        {
            insert(*iteratorBegin);
            ++iteratorBegin;
        }
    }


    HashMap(std::initializer_list<std::pair<const KeyType, ValueType>> initializerList, Hash mapHash = Hash()) : _mapHash(mapHash)
    {
        _data.assign(_maxSize, nullptr);
        for (auto &[key, value] : initializerList)
        {
            insert(std::make_pair(key, value));
        }
    }


    HashMap(const HashMap& other, Hash mapHash = Hash()) : _mapHash(mapHash)
    {
        std::vector<std::pair<const KeyType, ValueType>> content;
        auto iterBegin = other.begin();
        while (iterBegin != other.end()) {
            content.emplace_back(*iterBegin);
            ++iterBegin;
        }
        auto curPtr = _ptrBegin;
        while (curPtr != _ptrEnd)
        {
            auto ptrNext = curPtr->r;
            erase(curPtr->pair.first);
            curPtr = ptrNext;
        }
        _loadFactor = other._loadFactor;
        _maxSize = other._maxSize;
        _curSize = 0;
        _mapHash = other._mapHash;
        _ptrBegin = _ptrEnd = nullptr;
        _data.assign(_maxSize, nullptr);
        for (auto const& pair : content)
        {
            insert(pair);
        }
    }


    HashMap &operator=(const HashMap& other)
    {
        std::vector<std::pair<const KeyType, ValueType>> content;
        auto iterBegin = other.begin();
        while (iterBegin != other.end()) {
            content.emplace_back(*iterBegin);
            ++iterBegin;
        }
        auto curPtr = _ptrBegin;
        while (curPtr != _ptrEnd)
        {
            auto ptrNext = curPtr->r;
            erase(curPtr->pair.first);
            curPtr = ptrNext;
        }
        _loadFactor = other._loadFactor;
        _maxSize = other._maxSize;
        _curSize = 0;
        _mapHash = other._mapHash;
        _ptrBegin = _ptrEnd = nullptr;
        _data.assign(_maxSize, nullptr);
        for (auto const& pair : content)
        {
            insert(pair);
        }
        return *this;
    }


    ~HashMap()
    {
        clear();
    }


    [[nodiscard]] size_t size() const
    {
        return _curSize;
    }


    [[nodiscard]] bool empty() const
    {
        return _curSize == 0;
    }


    [[nodiscard]] Hash hash_function() const
    {
        return _mapHash;
    };


    void insert(std::pair<const KeyType, ValueType> pair)
    {
        Node* curPtr = new Node(pair);
        size_t idx = _mapHash(curPtr->pair.first) % _maxSize;
        while (_data[idx] != nullptr)
        {
            if (_data[idx]->pair.first == curPtr->pair.first) {
                // Keys are equal
                delete curPtr;
                return;
            }
            if (curPtr->displace > _data[idx]->displace)
            {
                if (_ptrBegin == _data[idx])
                {
                    _ptrBegin = curPtr;
                }
                curPtr->l = _data[idx]->l;
                curPtr->r = _data[idx]->r;
                if (curPtr->l != nullptr)
                {
                    (curPtr->l)->r = curPtr;
                }
                if (curPtr->r != nullptr)
                {
                    (curPtr->r)->l = curPtr;
                }
                _data[idx]->l = nullptr;
                _data[idx]->r = nullptr;
                std::swap(_data[idx], curPtr);
            }
            ++idx;
            if (idx == _maxSize)
            {
                idx = 0;
            }
            ++curPtr->displace;
        }
        _data[idx] = curPtr;
        curPtr->r = _ptrBegin;
        if (_ptrBegin != nullptr)
        {
            _ptrBegin->l = curPtr;
        }
        _ptrBegin = curPtr;
        ++_curSize;
        if (_curSize * _loadFactor > _maxSize)
        {
            rebuild();
        }
    }


    void erase(const KeyType curKey)
    {
        size_t idx = _mapHash(curKey) % _maxSize;
        while (_data[idx] != nullptr)
        {
            if (_data[idx]->pair.first == curKey)
            {
                if (_data[idx]->l != nullptr)
                {
                    (_data[idx]->l)->r = _data[idx]->r;
                }
                if (_data[idx]->r != nullptr)
                {
                    (_data[idx]->r)->l = _data[idx]->l;
                }
                if (_data[idx] == _ptrBegin)
                {
                    _ptrBegin = _data[idx]->r;
                }
                delete _data[idx];
                _data[idx] = nullptr;
                --_curSize;

                size_t prevIdx = idx;
                ++idx;
                if (idx == _maxSize)
                {
                    idx = 0;
                }

                while (_data[idx] != nullptr && _data[idx]->displace > 0)
                {
                    _data[prevIdx] = _data[idx];
                    --_data[prevIdx]->displace;
                    _data[idx] = nullptr;
                    prevIdx = idx;
                    ++idx;
                    if (idx == _maxSize)
                    {
                        idx = 0;
                    }
                }

                return;
            }
            ++idx;
            if (idx == _maxSize)
            {
                idx = 0;
            }
        }
    }


    class iterator{
    public:
        Node* _ptr = nullptr;

        iterator()
        {
            _ptr = nullptr;
        }


        explicit iterator(Node* ptr)
        {
            _ptr = ptr;
        }


        std::pair<const KeyType, ValueType>& operator*() const
        {
            return _ptr->pair;
        }


        std::pair<const KeyType, ValueType>* operator->() const
        {
            return &(_ptr->pair);
        }


        iterator& operator=(iterator other) {
            _ptr = other._ptr;
            return *this;
        }


        iterator& operator++()
        {
            _ptr = _ptr->r;
            return *this;
        }


        iterator operator++(int) { // NOLINT
            iterator prev_version = *this;
            _ptr = _ptr->r;
            return prev_version;
        }


        bool operator == (const iterator& other) const
        {
            return other._ptr == _ptr;
        }


        bool operator != (const iterator& other) const
        {
            return other._ptr != _ptr;
        }
    };


    iterator begin()
    {
        return iterator(_ptrBegin);
    }


    iterator end()
    {
        return iterator(_ptrEnd);
    }


    class const_iterator{
    public:
        Node* _ptr = nullptr;

        const_iterator()
        {
            _ptr = nullptr;
        }


        explicit const_iterator(Node* ptr)
        {
            _ptr = ptr;
        }


        const std::pair<const KeyType, ValueType>& operator*() const
        {
            return _ptr->pair;
        }


        const std::pair<const KeyType, ValueType>* operator->() const
        {
            return &(_ptr->pair);
        }


        const_iterator& operator=(const_iterator other) {
            _ptr = other._ptr;
            return *this;
        }


        const_iterator& operator++()
        {
            _ptr = _ptr->r;
            return *this;
        }


        const_iterator operator++(int) { // NOLINT
            const_iterator prev_version = *this;
            _ptr = _ptr->r;
            return prev_version;
        }


        bool operator == (const const_iterator& other) const
        {
            return other._ptr == _ptr;
        }


        bool operator != (const const_iterator& other) const
        {
            return other._ptr != _ptr;
        }
    };


    const_iterator begin() const
    {
        return const_iterator(_ptrBegin);
    }


    const_iterator end() const
    {
        return const_iterator(_ptrEnd);
    }


    iterator find(const KeyType curKey)
    {
        size_t idx = _mapHash(curKey) % _maxSize;
        while (_data[idx] != nullptr)
        {
            if (_data[idx]->pair.first == curKey)
            {
                return iterator(_data[idx]);
            }
            ++idx;
            if (idx == _maxSize)
            {
                idx = 0;
            }
        }
        return end();
    }


    const_iterator find(const KeyType curKey) const
    {
        size_t idx = _mapHash(curKey) % _maxSize;
        while (_data[idx] != nullptr)
        {
            if (_data[idx]->pair.first == curKey)
            {
                return const_iterator(_data[idx]);
            }
            ++idx;
            if (idx == _maxSize)
            {
                idx = 0;
            }
        }
        return end();
    }


    ValueType& operator[] (const KeyType curKey)
    {
        iterator result = find(curKey);
        if (result == end())
        {
            std::pair<const KeyType, ValueType> add = std::make_pair(curKey, ValueType());
            insert(add);
            result = find(curKey);
        }
        return (*result).second;
    }


    const ValueType& at(const KeyType curKey) const
    {
        const_iterator result = find(curKey);
        if (result == end())
        {
            throw std::out_of_range("");
        }
        return (*result).second;
    }


    void clear()
    {
        auto curPtr = _ptrBegin;
        while (curPtr != _ptrEnd)
        {
            auto ptrNext = curPtr->r;
            erase(curPtr->pair.first);
            curPtr = ptrNext;
        }
        _curSize = 0;
        _ptrBegin = _ptrEnd = nullptr;
    }


    void rebuild()
    {
        std::vector<std::pair<const KeyType, ValueType>> content;
        content.reserve(_curSize);
        iterator curIt = begin();
        while (curIt != end())
        {
            content.emplace_back(*curIt);
            ++curIt;
        }
        size_t prevMaxSize = _maxSize;
        clear();
        _maxSize = prevMaxSize * 2;
        _data.assign(_maxSize, nullptr);
        for (auto &pair : content)
        {
            insert(pair);
        }
    }
};