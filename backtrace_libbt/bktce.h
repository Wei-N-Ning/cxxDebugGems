#ifndef _BACKTRACE_LIB_H
#define _BACKTRACE_LIB_H

#include <string>
#include <vector>

#include <cstdlib>

using string_t = std::string;
using bool_t = bool;
using native_frame_ptr_t = const void *;

//! A textural representation of an x86_64 runtime stack-frame;
//! Provides accessor methods to retrieve the frame pointer; 
//! If debug symbols are available in the target binary, source code 
//! filename and line number are also available  
class Frame {
public:
    explicit Frame(native_frame_ptr_t i_address);

    //! Returns the native frame pointer;
    native_frame_ptr_t get() const;
    
    //! Returns a print-friendly string;
    string_t toString() const;
    
    //! Is source code information accessible
    bool_t hasSourceInfo() const;

    //! Returns full path to the source code;
    const string_t& getSourceFilename() const;
    
    //! Returns full path to the binary file;
    const string_t& getBinaryFilename() const;
    
    //! Return the source code line number
    std::size_t getSourceLineNumber() const;

private:
    native_frame_ptr_t m_native;
    string_t m_function;
    string_t m_sourceFilename;
    string_t m_binaryFilename;
    std::size_t m_sourceLineNumber;
};

//! The textural representation of an x86_64 runtime stack.
//! The instantiation of the class triggeres stack unwinding.
//! The caller may use "numSkippedFrames" parameter to control the 
//! position of the starting frame. 
//! Note that if the target is built with fomit-frame-pointer (or other
//! equivalent options) this container may be empty;
class Stacktrace {
public:
    // skip the call to the constructor and the unwinding function
    Stacktrace(std::size_t i_numSkippedFrames = 2);

    //! access each frame from the interior to the exterior;
    const std::vector<Frame>& getFrames() const;
    
    std::size_t size() const;

private:
    std::vector<Frame> m_frames;
};

//! unwind the stack then writes out the information collected from 
//! each frame to stdout.
void simple_backtrace();

#endif // _BACKTRACE_LIB_H
