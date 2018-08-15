
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <cassert>

#include <unwind.h>
#include <backtrace.h>
#include <dlfcn.h>
#include <cxxabi.h>

using bool_t = bool;
using string_t = std::string;
using native_frame_ptr_t = const void*;

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

//! Uses GCC libbacktrace to translate an address in .text section to
//! a location in the source code;
//! Requires the target to provide debug symbols
class AddressToSourceTranslator {
public:
    AddressToSourceTranslator();
    string_t translate(native_frame_ptr_t i_address);

    //! libbacktrace callback argument
    //! See gcc/libbacktrace/backtrace.h
    struct PCData {
        string_t* function;
        string_t* filename;
        std::size_t lineNumber;
    };

    //! callback functions to be passed to GCC's libbacktrace
    static int libbacktrace_full_callback(void*, uintptr_t, const char*, int, const char*);
    static void libbacktrace_error_callback(void*, const char*, int);

private:
    void getSourceCodeInfo(native_frame_ptr_t i_address);
    bool formatSourceCodeInfo();
    string_t getBinaryFilename(native_frame_ptr_t i_address);

    //! callback state to be passed to GCC"s libstacktrace
    backtrace_state* m_backtraceState = nullptr;

    //! textural data collected by the callbacks
    string_t m_formattedText;
    string_t m_filename;
    std::size_t m_lineNumber = 0;
};

AddressToSourceTranslator::AddressToSourceTranslator() {
    m_backtraceState = backtrace_create_state(nullptr, 
                                              0, 
                                              libbacktrace_error_callback, 
                                              nullptr);
}

string_t AddressToSourceTranslator::translate(native_frame_ptr_t i_address) {
    m_formattedText.clear();
    getSourceCodeInfo(i_address);
    if (! m_formattedText.empty()) {
        m_formattedText = demangle(m_formattedText.c_str());
    } else {
        std::stringstream ss;
        ss << std::hex << i_address;
        m_formattedText = ss.str();
    }

    //! successful translation
    if (formatSourceCodeInfo()) {
        return m_formattedText;
    }

    //! unsuccessful translation
    string_t binaryFilename = getBinaryFilename(i_address);
    if (! binaryFilename.empty()) {
        m_formattedText += " in ";
        m_formattedText += binaryFilename;
    }

    return m_formattedText;
}

//! libbacktrace callback function;
//! To retrieve the source code information from the program counter
int AddressToSourceTranslator::libbacktrace_full_callback(
    void* o_data, uintptr_t /*not used*/, const char* i_filename, int i_lineno, const char* i_function) {
    PCData& data = *static_cast<PCData *>(o_data);
    if (data.filename && i_filename) {
        *data.filename = i_filename;
    }
    if (data.function && i_function) {
        *data.function = i_function;
    }
    data.lineNumber = static_cast<std::size_t>(i_lineno);
    return 0;
}

//! libbacktrace callback function;
//! We don't have a use case where we would need to handle failed
//! backtrace operation hence the empty function body;
void AddressToSourceTranslator::libbacktrace_error_callback(
    void* /*not used*/, const char* /*not used*/, int /*not used*/) {
}

void AddressToSourceTranslator::getSourceCodeInfo(native_frame_ptr_t i_address) {
    PCData data = {
        &m_formattedText, 
        &m_filename, 
        0
    };
    if (m_backtraceState) {
        backtrace_pcinfo(
            m_backtraceState,
            reinterpret_cast<uintptr_t>(i_address),
            &libbacktrace_full_callback,
            &libbacktrace_error_callback,
            &data
        );
    }
    m_lineNumber = data.lineNumber;
}

bool AddressToSourceTranslator::formatSourceCodeInfo() {
    if (m_filename.empty() || !m_lineNumber) {
        return false;
    }
    std::stringstream ss;
    ss << " at " << m_filename << ":" << m_lineNumber;
    m_formattedText += ss.str();
    return true;
}

//! Get the binary target's filename from a frame pointer via the gnu 
//! linker;
//! This function is called if the translator fails to translate the 
//! frame pointer to a source code location;
string_t AddressToSourceTranslator::getBinaryFilename(native_frame_ptr_t i_address) {
    Dl_info dli;
    if (! dladdr(const_cast<void*>(i_address), &dli)) {
        return "";
    }
    return dli.dli_fname;
}

//! used as argument by _Unwind_Backtrace and its callback function;
//! current and end delineates the array that holds the collected
//! native frame pointers
class UnwindState {
public:
    UnwindState(std::size_t i_numSkippedFrames,
                native_frame_ptr_t* i_pointerArray,
                std::size_t i_pointerArraySize);

    std::size_t m_numSkippedFrames;
    native_frame_ptr_t* m_current;
    native_frame_ptr_t* m_end;
};

UnwindState::UnwindState(std::size_t i_numSkippedFrames,
                         native_frame_ptr_t* i_pointerArray,
                         std::size_t i_pointerArraySize)
    : m_numSkippedFrames(i_numSkippedFrames),
      m_current(i_pointerArray),
      m_end(i_pointerArray + i_pointerArraySize) {
}

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

//! this is core of stacktrace: to iterate over the frames on the runtime
//! stack; this function wraps GCC libbacktrace's _Unwind_Backtrace();
//! the later expects a pre-allocated contiguous buffer (an array) marked
//! by o_pointerArray and pointerArraySize and sets the frame pointer
//! to each element from the interior the exterior;
//! if i_numSkippedFrames is larger than 0, the first N frames are
//! skipped; this mechanism helps the implementer to hide the bootstrapping
//! functions (see the details in Stacktrace class)
//!
//! returns the actual number of frames collected
std::size_t collectNativeFramePointers(std::size_t i_pointerArraySize,
                                       std::size_t i_numSkippedFrames,
                                       native_frame_ptr_t* o_pointerArray) {
    std::size_t numFramesCollected = 0;
    UnwindState state(i_numSkippedFrames, o_pointerArray, i_pointerArraySize);
    _Unwind_Backtrace(&unwindCallback, &state);

    //! calculate the number of collected frames (they could completely
    //! fill or partially fill the given array)
    numFramesCollected = 0;
    for (std::size_t i = 0; i < i_pointerArraySize; ++i) {
        if (o_pointerArray[i] == nullptr) {
            break;
        }
        numFramesCollected++;
    }
    return numFramesCollected;
}

//! A textural representation of an x86_64 runtime stack-frame;
//! Provides accessor methods to retrieve program counter (PC), source 
//! code filename and line number if debug symbols are available in 
//! the target;
//! It also bookkeeps the native frame pointer so that caller may
//! use other means to inspect the frame;
class Frame {
public:
    explicit Frame(native_frame_ptr_t i_address);

    //! Returns the native frame pointer to the caller;
    native_frame_ptr_t get() const;
    
    //! Returns a print-friendly string;
    string_t toString() const;

private:
    native_frame_ptr_t m_native;
};

Frame::Frame(native_frame_ptr_t i_address)
    : m_native(i_address) {
}

native_frame_ptr_t Frame::get() const {
    return m_native;
}

string_t Frame::toString() const {
    std::stringstream ss;
    AddressToSourceTranslator translator;
    ss << "# " << translator.translate(get()) << std::endl;
    return ss.str();
}

//! The textural representation of an x86_64 runtime stack;
//! It holds the information of each frame and provides accessor methods;
//! Note that if the target is built with omit-frame-pointer (or other
//! equivalent option) this class may fail to see any frame
class Stacktrace {
public:
    Stacktrace();

    //! holds any frame or not
    bool isValid() const;

    //! access each frame from the interior to the exterior;
    const std::vector<Frame>& getFrames() const;
    
    std::size_t size() const;

    //! about the hardcoded max stack size:
    //! 128 is seen in boost's implementation (1.68.0);
    //! however in a deep recursion (such as the linear optimization
    //! algorithm) the number of frames could go beyond 128 even 256,
    //! hence the choice of 512 here
    static constexpr int m_maxStackSize = 512;

private:
    //! initialize our frame container
    void initialize();

    //! iterates over the native frame pointers and populate our frame
    //! container
    void populateFrames(native_frame_ptr_t* i_begin, std::size_t i_size);

    std::vector<Frame> m_frames;
};

Stacktrace::Stacktrace() {
    initialize();
}

bool Stacktrace::isValid() const {
    return ! m_frames.empty();
}

const std::vector<Frame>& Stacktrace::getFrames() const {
    return m_frames;
}

std::size_t Stacktrace::size() const {
    return m_frames.size();
}

void Stacktrace::initialize() {
    std::vector<native_frame_ptr_t> buf(m_maxStackSize, nullptr);

    //! numSkippedFrames:
    //! we skip the first 4 frames:
    //! 1) the call to ostream << Stacktrace()
    //! 2) the call to initialize()
    //! 3) the call to populateFrames()
    //! 4) the call to collectNativeFramePointers()
    std::size_t frames_count = collectNativeFramePointers(m_maxStackSize, 4, &buf[0]);
    populateFrames(&buf[0], frames_count);
}

void Stacktrace::populateFrames(native_frame_ptr_t* i_begin, std::size_t i_size) {
    if (!i_size) {
        return;
    }
    m_frames.reserve(i_size);

    for (std::size_t i = 0; i < i_size; ++i) {

        //! according to libc's man page (man backtrace_symbols)
        //! if the frame pointer is null, it is the end of the stack
        if (! i_begin[i]) {
            return;
        }

        //! avoid creating temporary frame object
        m_frames.emplace_back(i_begin[i]);
    }
}

}

extern "C" void do_backtrace() {
    Stacktrace st;
    if (! st.isValid()) {
        return;
    }
    int index = 0;
    for (const Frame& fr : st.getFrames()) {
        std::cout << std::right << std::setfill(' ') << std::setw(3) << index;
        std::cout << fr.toString();
        index += 1;
    }
    std::cout << std::endl;
}

