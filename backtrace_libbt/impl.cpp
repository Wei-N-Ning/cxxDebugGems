
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

namespace {

struct Frame {
    
};

std::string to_string(const Frame* frames, std::size_t size) {
    return "";
}

struct Stacktrace {
    
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
        std::vector<void *> buf(m_maxStackSize, nullptr);
        //do {
            //std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(&buf[0], buf.size(), frames_to_skip + 1);
            //if (buf.size() > frames_count || frames_count >= max_depth) {
                //std::size_t size = (max_depth < frames_count ? max_depth : frames_count);
                //fill(&buf[0], size);
                //return;
            //}
            //buf.resize(buf.size() * 2);
        //} while (buf.size() < buf.max_size()); 
    }
    
    constexpr int m_maxStackSize = 128;

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

