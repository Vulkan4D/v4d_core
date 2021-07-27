#pragma once



#ifdef _DEBUG
	namespace dbg {
		inline std::string basename(const std::string& file) {
			unsigned int i = file.find_last_of("\\/");
			if (i == std::string::npos) {
				return file;
			} else {
				return file.substr(i + 1);
			}
		}
	}
	#ifdef _WINDOWS
	
		#include <windows.h>
		#include <intrin.h>
		#include <dbghelp.h>
		#include <vector>
		
		namespace dbg {
			
			struct StackFrame {
				DWORD64 address;
				std::string name;
				std::string module;
				unsigned int line;
				std::string file;
			};
			
			inline std::vector<StackFrame> stack_trace(int max) {
				HANDLE process = GetCurrentProcess();
				HANDLE thread = GetCurrentThread();
				CONTEXT context = {};
				context.ContextFlags = CONTEXT_FULL;
				RtlCaptureContext(&context);
				SymInitialize(process, NULL, TRUE);
				// SymSetOptions(SYMOPT_LOAD_ANYTHING | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_ALLOW_ZERO_ADDRESS | SYMOPT_DEBUG);
				DWORD machine = IMAGE_FILE_MACHINE_AMD64;
				STACKFRAME64 frame {};
					frame.AddrPC.Offset = context.Rip;
					frame.AddrPC.Mode = AddrModeFlat;
					frame.AddrFrame.Offset = context.Rsp;
					frame.AddrFrame.Mode = AddrModeFlat;
					frame.AddrStack.Offset = context.Rsp;
					frame.AddrStack.Mode = AddrModeFlat;
				int index = 0;
				std::vector<StackFrame> frames;
				while (StackWalk64(machine, process, thread, &frame, &context , NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
					StackFrame f {};
					f.address = frame.AddrPC.Offset;
					DWORD64 moduleBase = 0;
					moduleBase = SymGetModuleBase64(process, frame.AddrPC.Offset);
					char moduelBuff[MAX_PATH];
					if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduelBuff, MAX_PATH)) {
						f.module = basename(moduelBuff);
					} else {
						f.module = "Unknown Module";
					}
					DWORD64 offset = 0;
					char symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + 255];
					PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)symbolBuffer;
					symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
					symbol->MaxNameLength = 254;
					if (SymGetSymFromAddr64(process, frame.AddrPC.Offset, &offset, symbol)) {
						f.name = symbol->Name;
					} else {
						DWORD error = GetLastError();
						f.name = "Unknown Function";
					}
					IMAGEHLP_LINE line {};
					line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
					DWORD offset_ln = 0;
					if (SymGetLineFromAddr64(process, symbol->Address, &offset_ln, &line)) {
						f.file = line.FileName;
						f.line = line.LineNumber;
						{size_t pos; while ((pos = f.file.find("\\")) != std::string::npos) f.file.replace(pos, 1, "/");}
						{std::string basePath = _V4D_PROJECT_PATH; if (size_t pos = f.file.find(basePath); pos == 0) f.file.replace(0, basePath.length(), "");}
					} else {
						DWORD error = GetLastError();
					} 
					if (index > 0) frames.push_back(f);
					if (++index > max) break;
				}
				SymCleanup(process);
				return frames;
			}
		}
		
		#define STACKTRACE(MAX_STACK_COUNT) {\
			auto stack = ::dbg::stack_trace(MAX_STACK_COUNT);\
			std::cerr << "========= call stack ==========\n";\
			for (uint32_t i = 0; i < stack.size(); ++i) {\
				if (stack[i].file != "" && stack[i].line != 0) {\
					std::cerr << "\t" << i << ": " << stack[i].module << " 0x" << std::hex << stack[i].address << " " << stack[i].name << " [" << stack[i].file << ":" << std::dec << stack[i].line << "]\n";\
				} else {\
					std::cerr << "\t" << i << ": " << stack[i].module << " 0x" << std::hex << stack[i].address << " " << stack[i].name << "\n";\
				}\
			}\
			std::cerr << "===============================" << std::endl;\
		}
	#else
		#include <execinfo.h>
		#include <cxxabi.h>
		#define STACKTRACE(MAX_STACK_COUNT) {\
			void* stack[MAX_STACK_COUNT];\
			size_t stackCount = backtrace(stack, MAX_STACK_COUNT);\
			char** frames = backtrace_symbols(stack, stackCount);\
			std::cerr << "========= call stack ==========\n";\
				for (size_t i = 0; i < stackCount; ++i) {\
					std::string frame = frames[i];\
					int start = frame.find("(");\
					if (start == std::string::npos) start = frame.find(" ");\
					int plus = frame.find("+0x");\
					int plusEnd = frame.length()-frame.find(")")+1;\
					int plusLength = frame.length()-plus-plusEnd;\
					int end = frame.length() - plus+1;\
					int length = frame.length()-start-end;\
					std::string module = frame.substr(0, start);\
					std::string moduleName = ::dbg::basename(module);\
					std::string addrInModule = (plusLength > 0 && plus != std::string::npos) ? frame.substr(plus+1, plusLength) : "";\
					std::string symbol = (length > 0 && start != std::string::npos) ? frame.substr(start+1, length) : "";\
					int demangleStatus = -1;\
					char* demangledSymbol = abi::__cxa_demangle(symbol.c_str(), 0, 0, &demangleStatus);\
					if (demangleStatus == 0) {\
						std::cerr << "\t" << i << ": " << moduleName << " " << stack[i] << " " << demangledSymbol << " ";\
					} else {\
						std::cerr << "\t" << i << ": " << moduleName << " " << stack[i] << " " << symbol << " ";\
					}\
					free(demangledSymbol);\
					system((std::string("addr2line ") + addrInModule + " -e " + module + " 1>&2").c_str());\
				}\
			std::cerr << "===============================" << std::endl;\
			free(frames);\
		}
	#endif
#else
	#define STACKTRACE(MAX_STACK_COUNT)
#endif
