
#include "bktce.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <unwind.h>
#include <backtrace.h>
#include <dlfcn.h>
#include <cxxabi.h>

namespace {

//! A helper function to encapsulate symbol-demangling
string_t demangle(const char* i_symbol) {
    int status = 0;
    std::size_t size = 0;
    char* p = abi::__cxa_demangle(i_symbol, nullptr, &size, &status);
    
    //! if demangling has no effect or it fails for some reason, 
    //! return the original symbol back;
    if (p == nullptr || status != 0) {
        return i_symbol;
    }
    
    string_t tmp(p);
    std::free(p);
    return tmp;    
}

//! Get the binary target's filename from a frame pointer via the gnu 
//! linker;
//! This function is called if the translator fails to translate the 
//! frame pointer to a source code location;
string_t getBinaryFilenameLD(native_frame_ptr_t i_address) {
    Dl_info dli;
    if (! dladdr(const_cast<void*>(i_address), &dli)) {
        return "";
    }
    return dli.dli_fname;
}

//! libbacktrace callback argument
//! See gcc/libbacktrace/backtrace.h
struct PCData {
    string_t* function;
    string_t* filename;
    std::size_t lineNumber;
};

//! libbacktrace callback function;
//! To retrieve the source code information from the program counter
int libbacktrace_full_callback(void* o_data, 
                               uintptr_t /*not used*/, 
                               const char* i_filename, 
                               int i_lineno, 
                               const char* i_function) {
    PCData& data = *static_cast<PCData *>(o_data);
    if (data.filename && i_filename) {
        *data.filename = i_filename;
    }
    if (data.function && i_function) {
        *data.function = demangle(i_function);
    }
    data.lineNumber = static_cast<std::size_t>(i_lineno);
    return 0;
}

//! libbacktrace callback function;
//! We don't have a use case where we would need to handle failed
//! backtrace operation hence the empty function body;
void libbacktrace_error_callback(void* /*not used*/, 
                                 const char* /*not used*/, 
                                 int /*not used*/) {
}

//! used as argument by _Unwind_Backtrace and its callback function;
//! current and end delineates the array that holds the collected
//! native frame pointers
struct UnwindState {
    std::size_t m_numSkippedFrames;
    native_frame_ptr_t* m_current;
    native_frame_ptr_t* m_end;
};

//! this is the callback function passed to _Unwind_Backtrace;
//! its role is to tell _Unwind_Backtrace to continue iterating the
//! stack frames or stop;
//! according to libbacktrace/unwind.h
//! to continue: return _URC_NO_REASON
//! to stop: return _URC_END_OF_STACK
_Unwind_Reason_Code unwindCallback(_Unwind_Context* i_context, void* i_state) {

    UnwindState* state = nullptr;

    //! identify the end of stack condition if the stack is empty
    //! this can happen if the target is compiled with -fomit-frame-pointer
    //! or other equivalent option
    if (! i_state) {
        return _URC_END_OF_STACK;
    }
    state = static_cast<UnwindState *>(i_state);

    //! to skip the first N frames
    //! note that while skipping, it can already reach the end of the stack
    if (state->m_numSkippedFrames) {
        state->m_numSkippedFrames -= 1;
        if (_Unwind_GetIP(i_context)) {
            return _URC_NO_REASON;
        } else {
            return _URC_END_OF_STACK;
        }
    }

    //! during iteration;
    //! populate the state structure fields
    *state->m_current =  reinterpret_cast<native_frame_ptr_t>(_Unwind_GetIP(i_context));
    state->m_current += 1;

    //! identify the end of stack condition
    if (!*(state->m_current - 1) || state->m_current == state->m_end) {
        return _URC_END_OF_STACK;
    }

    //! continue iteration
    return _URC_NO_REASON;
}

}

Frame::Frame(native_frame_ptr_t i_address)
 : m_native(i_address) {
    backtrace_state* backtraceState = backtrace_create_state(
        nullptr, 
        0, 
        &libbacktrace_error_callback, 
        nullptr);
    PCData data = {&m_function, &m_sourceFilename, 0};
    if (backtraceState) {
        backtrace_pcinfo(
            backtraceState,
            reinterpret_cast<uintptr_t>(i_address),
            &libbacktrace_full_callback,
            &libbacktrace_error_callback,
            &data
        );
        m_sourceLineNumber = data.lineNumber;
    }
    if (! hasSourceInfo()) {
        m_binaryFilename = getBinaryFilenameLD(m_native);
    }
}

native_frame_ptr_t Frame::get() const {
    return m_native;
}

bool_t Frame::hasSourceInfo() const {
    return m_sourceFilename.size() > 0 && m_sourceLineNumber > 0;
}

const string_t& Frame::getSourceFilename() const {
    return m_sourceFilename;
}
    
const string_t& Frame::getBinaryFilename() const {
    return m_binaryFilename;
}
    
std::size_t Frame::getSourceLineNumber() const {
    return m_sourceLineNumber;
}

string_t Frame::toString() const {
    std::stringstream ss;
    ss << std::hex << m_native << std::dec << " ";
    if (hasSourceInfo()) {
        ss << m_function << " at " << m_sourceFilename << ":" << m_sourceLineNumber;
    } else {
        ss << " in " << m_binaryFilename;
    }
    ss << std::endl;    
    return ss.str();
}

const std::vector<Frame>& Stacktrace::getFrames() const {
    return m_frames;
}

std::size_t Stacktrace::size() const {
    return m_frames.size();
}

Stacktrace::Stacktrace(std::size_t i_numSkippedFrames) {
    
    //! about the hardcoded max stack size:
    //! 128 is seen in boost's implementation (1.68.0);
    //! however in a deep recursion (such as the linear optimization
    //! algorithm) the number of frames could go beyond 128 even 256,
    //! hence the choice of 512 here
    static const int s_maxStackSize = 512;
    std::vector<native_frame_ptr_t> buf(s_maxStackSize, nullptr);
    std::size_t numFramesCollected = 0;

    UnwindState state = {
        i_numSkippedFrames,
        buf.data(),
        buf.data() + s_maxStackSize,
    };
    _Unwind_Backtrace(&unwindCallback, &state);

    //! calculate the number of collected frame pointers
    for (std::size_t i = 0; i < s_maxStackSize; ++i) {
        if (buf[i] == nullptr) {
            break;
        }
        numFramesCollected++;
    }
    
    //! can not see any frame 
    if (! numFramesCollected) {
        return;
    }

    m_frames.reserve(numFramesCollected);
    for (std::size_t i = 0; i < numFramesCollected; ++i) {

        //! if the frame pointer is null, it is the end of the stack
        if (! buf[i]) {
            return;
        }

        //! create the Frame object
        m_frames.emplace_back(buf[i]);
    }
}

void simple_backtrace() {
    Stacktrace st;
    if (st.size() == 0) {
        return;
    }
    int index = 0;
    for (const Frame& fr : st.getFrames()) {
        std::cout << std::right << std::setfill(' ') << std::setw(3) << index;
        std::cout << "# ";
        std::cout << fr.toString();
        index += 1;
    }
    std::cout << std::endl;
}

