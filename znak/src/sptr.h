#pragma once
#include <utility>
#include <type_traits>
#include <stdexcept>
	

namespace znak
{

struct nocopy
{
    nocopy() {};
    nocopy(const nocopy& other) = delete;
    void operator=(const nocopy&) = delete;
};

template <typename T>
struct anyptr : nocopy
{
    virtual T* getPtr() = 0;

    operator T&() const { return *(getPtr()); }
    operator T&() { return *(getPtr()); }
    const T* operator->() const { return getPtr(); }
    T* operator->() { return &getPtr(); }
    T& operator*() { return *getPtr(); }
    const T& operator*() const { return getPtr(); }

    virtual ~anyptr() = default;

    virtual bool isReal() const;
};

template <typename T>
struct anyptr_real : anyptr<T>
{
    anyptr_real(T* ptr) : ptr(ptr), borrowed(false) {}
    static anyptr_real<T>&& borrow(const T& val) { return {&val, true}; }
    static anyptr_real<T>* borrowNew(const T& val) { return new anyptr_real<T>(&val, true); }

    virtual T* getPtr() override { return ptr; }
    bool isReal() const override { return true; }

    T* ptr;
    bool borrowed;

    virtual ~anyptr_real() { if(!borrowed) delete ptr; }
private:
    anyptr_real(T* ptr, bool borrowed) : ptr(ptr), borrowed(borrowed) {}
};



template <typename T>
struct wrapval
{
    virtual T* ptr() = 0;
    virtual const T* ptr() const = 0;
    wrapval() {}

    wrapval(const wrapval& other) = delete;
    void operator=(const wrapval&) = delete;

    virtual ~wrapval() = default;
};

template <typename TOuter, typename TInner = TOuter>
struct wrapval_impl : wrapval<TOuter>
{
    enum class AllowRef: char { YES };

    wrapval_impl(TInner&& createVal) : value(std::forward<TInner>(createVal)) { }
    wrapval_impl(TInner& copyVal) = delete;
    wrapval_impl() = delete;

    static wrapval_impl* copyToPtr(const TInner& value) 
    {
        return new wrapval_impl(value, AllowRef::YES);
    }
    
    virtual TOuter* ptr() override { return &value; }
    virtual const TOuter* ptr() const override { return &value; }

    virtual ~wrapval_impl() = default;

private:
    TInner value;
    wrapval_impl(const TInner& copyVal, AllowRef) : value(copyVal) {}
};

template <typename T>
struct anyptr_rref : anyptr<T>
{
    anyptr_rref(T&& createVal) : value(std::move(createVal)) { }

    static anyptr_rref&& copy(const T& value) 
    {
        T copyVal = value;
        return anyptr_rref(std::move(copyVal));
    }
    static anyptr_rref* copyNew(const T& value) 
    {
        T copyVal = value;
        return new anyptr_rref(std::move(copyVal));
    }

    virtual T* getPtr() override { return &value; }
    bool isReal() const override { return false; }
    virtual ~anyptr_rref() = default;
private:
    T value;
};



template <typename T>
struct sptr
{
    template <typename TDerived>
    using enable_if_base = std::enable_if<std::is_base_of<T, TDerived>::value, T>;

    enum class CREATE_WRAPPER: char { YES };
    enum class CREATE_WEAK: char { YES };

    sptr() {}
    sptr(T* rawPtr) : raw(rawPtr) {}
    // template <typename TSubClass, typename std::enable_if<std::is_convertible<TSubClass, T>::value, T>::type* = nullptr>
    // sptr(sptr<TSubClass>&& toBase) 
    // { 
    //     raw = toBase.raw->getPtr();
    //     toBase.raw = nullptr; 
    // }

    sptr(const sptr& other) = delete;
    void operator=(const sptr&) = delete;
    //template <typename TSubClass, typename std::enable_if<std::is_base_of<T, TSubClass>::value, T>::type* = nullptr>
    // allow internal creation from rvr
    template <typename TSubClass, typename enable_if_base<TSubClass>::type* = nullptr>
    sptr(TSubClass&& manageValue) : sptr(new wrapval_impl<T, TSubClass>(std::forward<TSubClass>(manageValue)), CREATE_WRAPPER::YES) {}
    // do not allow refference copy
    template <typename TSubClass, typename enable_if_base<TSubClass>::type* = nullptr>
    sptr(TSubClass& manageValue) = delete;

    template <typename TSubClass, typename enable_if_base<TSubClass>::type* = nullptr>
    static sptr borrow(TSubClass& borrowPtr) { return sptr(&borrowPtr, CREATE_WEAK::YES); }

    template <typename TSubClass, typename enable_if_base<TSubClass>::type* = nullptr>
    static sptr borrow(sptr<TSubClass>& borrowPtr) { return sptr(borrowPtr.getPtr(), CREATE_WEAK::YES); }

    template <typename TSubClass, typename enable_if_base<TSubClass>::type* = nullptr>
    static sptr copy(const TSubClass& copyValue) { return sptr(wrapval_impl<T, TSubClass>::copyToPtr(copyValue), CREATE_WRAPPER::YES); }
    //static sptr<T>&& copy(const T& copyValue) { return sptr(anyptr_rref<T>::copyNew(copyValue), 1); }

    operator bool() const { return raw != nullptr || wrapper != nullptr; }

    #pragma region operator shortcuts
    operator const T&() const { return *ptr(); }
    operator T&() { return *ptr(); }
    const T* operator->() const { return ptr(); }
    T* operator->() { return ptr(); }
    T& operator*() { return *ptr(); }
    const T& operator*() const { return ptr(); }
    #pragma endregion

    #pragma region move semantics
    sptr& operator=(sptr&& moveHere)
    {
        raw = std::move(moveHere.raw);
        wrapper = std::move(moveHere.wrapper);
        weak = moveHere.weak;

        moveHere.wrapper = nullptr;
        moveHere.raw = nullptr;
        return *this;
    }
    sptr(sptr&& moveHere)
     : raw(std::move(moveHere.raw))
     , wrapper(std::move(moveHere.wrapper))
     , weak(moveHere.weak)
    {
        moveHere.wrapper = nullptr;
        moveHere.raw = nullptr;
    }
    #pragma endregion

    T* getPtr() { return ptr(); }
    const T* getPtr() const { return ptr(); }

    ~sptr() 
    {
        if(!weak)
            delete raw;
        delete wrapper;
    }
private:

    struct delegptr : nocopy
    {
        virtual T* ptr() = 0;
    };
    template <typename TSubClass>
    struct delegbase : delegptr
    {
        friend struct sptr<T>;
        friend struct sptr<TSubClass>;
        virtual T* ptr() override { return origPtr.ptr(); }
    private:
        sptr<TSubClass> origPtr;
    };

    T* raw = nullptr;
    wrapval<T>* wrapper = nullptr;
    bool weak = false;


    sptr(wrapval<T>* wrapper, CREATE_WRAPPER) : wrapper(wrapper) {}
    sptr(T* borrowed, CREATE_WEAK) : raw(borrowed), weak(true) {}

    T* ptr() 
    { 
        if(raw)
            return raw;
        else if(wrapper)
            return wrapper->ptr();
        else 
            throw std::runtime_error("Tried to dereference a dead sptr.");
    }
    const T* ptr() const 
    {
        return (const T*)const_cast<sptr*>(this)->ptr();
    }


};
}