#ifndef UTILS_HPP
#define UTILS_HPP


namespace cbot {

    // @note: This is a better impl I found on stackoverflow (I lost that post), the other way I did it a few
    // years ago was to use functor object to store the function and the object:
    // @ref: https://github.com/ultrasilicon/libagio/blob/master/include/callback.h#L30
    /**
     * @brief Bind member function with object without complicated std::bind placeholders.
     * @param f The member function pointer.
     * @param obj The object pointer.
     * @return A lambda that wraps the member function call.
     */
    template <typename Func, typename Obj>
    auto bind(Func f, Obj* obj) {
        return [=](auto&&... args) {
            return (obj->*f)(std::forward<decltype(args)>(args)...);
        };
    }

    inline void to_upper(std::string& s) {
        std::transform(
            s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); }
        );
    }

    inline std::string to_upper_copy(const std::string& input) {
        std::string output = input;
        std::transform(
            output.begin(), output.end(), output.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); }
        );
        return output;
    }
}

#define AI_GENERATED ;

#endif // UTILS_HPP
