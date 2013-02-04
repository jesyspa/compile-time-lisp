#pragma once
#include <cassert>
#include <map>
#include <type_traits>
#include <utility>

template<typename>
struct const_false : std::false_type {};

template<typename...>
struct list;

template<typename HEAD, typename... Rest>
struct list<HEAD, Rest...> {
    using head = HEAD;
    using tail = list<Rest...>;
};

template<typename HEAD>
struct list<HEAD> {
    using head = HEAD;
    using tail = list<>;
};

template<>
struct list<> {};

using nil = list<>;

template<typename T>
struct type_id {
    static void* get() {
        static char c;
        return &c;
    }
};

struct env {
    env* parent;
    std::map<void*, int> values;

    template<typename T>
    int lookup() const {
        auto it = values.find(type_id<T>::get());
        if (it != values.end())
            return it->second;
        if (parent)
            return parent->lookup<T>();
        assert(false && "No such variable");
    }

    template<typename, typename>
    struct raise_env_impl;

    template<typename... PARAMS, typename... ARGS>
    env raise_env(ARGS... args) {
        env e{this, {}};
        raise_env_impl<list<PARAMS...>, list<ARGS...>>::call(e, std::forward<ARGS>(args)...);
        return e;
    }
};

template<>
struct env::raise_env_impl<nil, nil> {
    static void call(env&) {}
};

template<typename PHEAD, typename... PARAMS, typename AHEAD, typename... ARGS>
struct env::raise_env_impl<list<PHEAD, PARAMS...>, list<AHEAD, ARGS...>> {
    static void call(env& e, AHEAD head, ARGS... args) {
        e.values[type_id<PHEAD>::get()] = head;
        raise_env_impl<list<PARAMS...>, list<ARGS...>>::call(e);
    }
};

struct plus {
    static auto call(env&, int x, int y) {
        return x + y;
    }
};

template<typename FIRST, typename SECOND>
struct pair {
    using first = FIRST;
    using second = SECOND;
};

template<int N>
struct constant {
    static int call(env&) {
        return N;
    }
};

template<typename NAME, typename ENV>
struct lookup {
    using value = lookup<NAME, typename ENV::tail>;
};

template<typename NAME, typename VALUE, typename... REST>
struct lookup<NAME, list<pair<NAME, VALUE>, REST...>> {
    using value = VALUE;
};

template<typename NAME>
struct lookup<NAME, nil> {
    static_assert(const_false<NAME>::value, "Use of undeclared variable.");
};

template<typename>
struct var {};

template<typename, typename>
struct lambda {};

template<typename>
struct compile_impl;

template<int N>
struct compile_impl<constant<N>> : constant<N> {};

template<typename FUNC, typename... ARGS>
struct compile_impl<list<FUNC, ARGS...>> {
    static auto call(env& e) {
        return FUNC::call(e, compile_impl<ARGS>::call(e)...);
    }
};

template<typename NAME>
struct compile_impl<var<NAME>> {
    static auto call(env& e) {
        return e.lookup<NAME>();
    }
};

template<typename... PARAMS, typename BODY>
struct compile_impl<lambda<list<PARAMS...>, BODY>> {
    template<typename... ARGS>
    static auto call(env& e, ARGS... args) {
        auto env = e.raise_env<PARAMS...>(std::forward<ARGS>(args)...);
        return compile_impl<BODY>::call(env);
    }
};

template<typename T>
struct callable {
    template<typename... ARGS>
    auto operator()(ARGS... args) const {
        env e;
        return T::call(e, std::forward<ARGS>(args)...);
    }
};

template<typename PARAMS, typename BODY>
struct compile {
    static callable<compile_impl<lambda<PARAMS, BODY>>> result() {
        return {};
    }
};
