#pragma once

#include <sstream>
#include <vector>
#include <cstdarg>
#include <cstdio>

#include "platform/api.h"

namespace LogDetail
{
	/// Formats and hands one line to the platform log. Kept out of line from
	/// the macros below so the varargs handling exists once.
	inline void Write(Platform::LogLevel level, const char* tag, const char* format, ...)
	{
		char buffer[1024];

		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof buffer, format, args);
		va_end(args);

		Platform::LogLine(level, tag, buffer);
	}
}

#define LOGW(...) LogDetail::Write(Platform::LogLevel::Warning, "AX",  __VA_ARGS__)
#define LOGI(...) LogDetail::Write(Platform::LogLevel::Info,    "AXL", __VA_ARGS__)
#define LOGE(...) LogDetail::Write(Platform::LogLevel::Error,   "AXL", __VA_ARGS__)

struct TraceInfo
{
	std::string name;
	std::vector<std::string> params;
};

class Log
{
public:
	static void traceLastFunc(const std::string& func_name);

	template<typename T>
	static void addParameter(const std::string& func_param_name, T value)
	{
		//auto params = m_listTracedFuncs.front().params;

		//std::stringstream ss;
		//ss << func_param_name << " = " << value;
		//params.push_back(ss.str());
		//LOGI(ss.str().c_str()); 
	};

private:

	//static std::list<TraceInfo> m_listTracedFuncs;
};
