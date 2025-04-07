#ifndef UTILS_HPP
#define UTILS_HPP


namespace cbot {
    template <typename Func, typename Obj>
    auto bind(Func f, Obj* obj) {
        return [=](auto&&... args) {
            return (obj->*f)(std::forward<decltype(args)>(args)...);
        };
    }
}

#endif // UTILS_HPP
