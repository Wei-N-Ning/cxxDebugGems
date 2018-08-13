
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <cassert>

#include <unwind.h>  // gcc
#include <backtrace.h>  // gcc
#include <dlfcn.h>  // -ldl
#include <cxxabi.h>  // gcc

namespace core {

inline char const *demangle_alloc(char const *name);

inline void demangle_free(char const *name);

class scoped_demangled_name {
private:
    char const *m_p;

public:
    explicit scoped_demangled_name(char const *name)
        : m_p(demangle_alloc(name)) {
    }

    ~scoped_demangled_name() {
        demangle_free(m_p);
    }

    char const *get() const {
        return m_p;
    }

    scoped_demangled_name(scoped_demangled_name const &) = delete;

    scoped_demangled_name &operator=(scoped_demangled_name const &) = delete;
};

inline char const *demangle_alloc(char const *name) {
    int status = 0;
    std::size_t size = 0;
    return abi::__cxa_demangle(name, NULL, &size, &status);
}

inline void demangle_free(char const *name) {
    std::free(const_cast< char * >( name ));
}

inline std::string demangle(char const *name) {
    scoped_demangled_name demangled_name(name);
    char const *p = demangled_name.get();
    if (!p)
        p = name;
    return p;
}

} // namespace core

namespace {

struct Frame {
    const void* m_addr;

    explicit Frame(const void* addr) : m_addr(addr) {}
};

struct pc_data {
    std::string* function;
    std::string* filename;
    std::size_t line;
};

int libbacktrace_full_callback(void *data, uintptr_t /*pc*/, const char *filename, int lineno, const char *function) {
    pc_data& d = *static_cast<pc_data*>(data);
    if (d.filename && filename) {
        *d.filename = filename;
    }
    if (d.function && function) {
        *d.function = function;
    }
    d.line = lineno;
    return 0;
}

void libbacktrace_error_callback(void * /*data*/, const char * /*msg*/, int /*errnum*/) {
}

std::array<char, 40> to_dec_array(std::size_t value) {
    std::array<char, 40> ret;
    if (!value) {
        ret[0] = '0';
        ret[1] = '\0';
        return ret;
    }

    std::size_t digits = 0;
    for (std::size_t value_copy = value; value_copy; value_copy /= 10) {
        ++digits;
    }

    for (std::size_t i = 1; i <= digits; ++i) {
        ret[digits - i] = '0' + (value % 10);
        value /= 10;
    }

    ret[digits] = '\0';

    return ret;
}

typedef const void* native_frame_ptr_t;

struct location_from_symbol {
    ::Dl_info dli_;

    explicit location_from_symbol(const void* addr)
        : dli_()
    {
        if (!::dladdr(const_cast<void*>(addr), &dli_)) {
            dli_.dli_fname = 0;
        }
    }

    bool empty() const {
        return !dli_.dli_fname;
    }

    const char* name() const {
        return dli_.dli_fname;
    }
};

static char to_hex_array_bytes[] = "0123456789ABCDEF";

template<class T>
inline std::array<char, 2 + sizeof(void *) * 2 + 1> to_hex_array(T addr) {
    std::array<char, 2 + sizeof(void *) * 2 + 1> ret = {"0x"};
    ret.back() = '\0';

    const std::size_t s = sizeof(T);

    char *out = ret.data() + s * 2 + 1;

    for (std::size_t i = 0; i < s; ++i) {
        const unsigned char tmp_addr = (addr & 0xFFu);
        *out = to_hex_array_bytes[tmp_addr & 0xF];
        --out;
        *out = to_hex_array_bytes[tmp_addr >> 4];
        --out;
        addr >>= 8;
    }

    return ret;
}

inline std::array<char, 2 + sizeof(void *) * 2 + 1> to_hex_array(const void *addr) {
    return to_hex_array(
        reinterpret_cast< std::make_unsigned<std::ptrdiff_t>::type >(addr)
    );
}

struct to_string_using_backtrace {
    std::string res;
    ::backtrace_state* state;
    std::string filename;
    std::size_t line;

    void prepare_function_name(const void* addr) {
        pc_data data = {&res, &filename, 0};
        if (state) {
            backtrace_pcinfo(
                state,
                reinterpret_cast<uintptr_t>(addr),
                &libbacktrace_full_callback,
                &libbacktrace_error_callback,
                &data
            );
        }
        line = data.line;
    }

    bool prepare_source_location(const void* /*addr*/) {
        if (filename.empty() || !line) {
            return false;
        }
        res += " at ";
        res += filename;
        res += ':';
        res += to_dec_array(line).data();
        return true;
    }

    std::string operator()(native_frame_ptr_t addr) {
        res.clear();
        prepare_function_name(addr);
        if (! res.empty()) {
            res = core::demangle(res.c_str());
        } else {
            res = to_hex_array(addr).data();
        }

        if (prepare_source_location(addr)) {
            return res;
        }

        location_from_symbol loc(addr);
        if (!loc.empty()) {
            res += " in ";
            res += loc.name();
        }

        return res;
    }

};

std::string to_string(const Frame* frames, std::size_t size) {
    std::string res;
    res.reserve(64 * size);

    to_string_using_backtrace impl;

    for (std::size_t i = 0; i < size; ++i) {
        if (i < 10) {
            res += ' ';
        }
        res += to_dec_array(i).data();
        res += '#';
        res += ' ';
        res += impl(frames[i].m_addr);
        res += '\n';
    }

    return res;
}

struct unwind_state {
    std::size_t frames_to_skip;
    native_frame_ptr_t* current;
    native_frame_ptr_t* end;
};

inline _Unwind_Reason_Code unwind_callback(::_Unwind_Context* context, void* arg) {
    // Note: do not write `::_Unwind_GetIP` because it is a macro on some platforms.
    // Use `_Unwind_GetIP` instead!
    unwind_state* const state = static_cast<unwind_state*>(arg);
    if (state->frames_to_skip) {
        --state->frames_to_skip;
        return _Unwind_GetIP(context) ? ::_URC_NO_REASON : ::_URC_END_OF_STACK;
    }

    *state->current =  reinterpret_cast<native_frame_ptr_t>(
        _Unwind_GetIP(context)
    );

    ++state->current;
    if (!*(state->current - 1) || state->current == state->end) {
        return ::_URC_END_OF_STACK;
    }
    return ::_URC_NO_REASON;
}

std::size_t collect(native_frame_ptr_t *out_frames, std::size_t max_frames_count, std::size_t skip) {
    std::size_t frames_count = 0;
    if (!max_frames_count) {
        return frames_count;
    }

    unwind_state state = {skip + 1, out_frames, out_frames + max_frames_count};
    ::_Unwind_Backtrace(&unwind_callback, &state);
    frames_count = state.current - out_frames;

    if (frames_count && out_frames[frames_count - 1] == nullptr) {
        --frames_count;
    }

    return frames_count;
}

struct Stacktrace {

    Stacktrace() {
        init();
    }

    bool isValid() const {
        return ! m_frames.empty();
    }

    const std::vector<Frame>& as_vector() const {
        return m_frames;
    }
    
    std::size_t size() const {
        return m_frames.size();
    }
    
    void init() {
        std::vector<native_frame_ptr_t > buf(m_maxStackSize, nullptr);
        std::size_t frames_count = collect(&buf[0], buf.size(), 1);
        fill(&buf[0], frames_count);
    }

    void fill(native_frame_ptr_t* begin, std::size_t size) {
        if (!size) {
            return;
        }

        m_frames.reserve(static_cast<std::size_t>(size));
        for (std::size_t i = 0; i < size; ++i) {
            if (!begin[i]) {
                return;
            }
            m_frames.emplace_back(begin[i]);
        }
    }

    static constexpr int m_maxStackSize = 128;

    std::vector<Frame> m_frames;
};

template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const Stacktrace& st) {
    if (st.isValid()) {
        os << to_string(&(st.as_vector()[0]), st.size());
    }

    return os;
}

}

void do_backtrace() {
    std::cout << Stacktrace();
}

