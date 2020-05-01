#pragma once

#include "Core/Singleton.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Arcane
{
	class Logger : public Singleton
	{
	private:
		Logger();
		~Logger();
	public:
		static Logger& GetInstance();
		inline static std::shared_ptr<spdlog::logger> GetEngineLogger() { return s_EngineLogger; }
		inline static std::shared_ptr<spdlog::logger> GetGameLogger() { return s_GameLogger; }
	private:
		static void Init();
	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
		static std::shared_ptr<spdlog::logger> s_GameLogger;
	};
}

// Engine Log Macros
#ifdef ARC_FINAL
#define ARC_LOG_FATAL
#define ARC_LOG_ERROR
#define ARC_LOG_WARN
#define ARC_LOG_INFO
#define ARC_LOG_TRACE
#else
#define ARC_LOG_FATAL(...) Arcane::Logger::GetEngineLogger()->critical(__VA_ARGS__)
#define ARC_LOG_ERROR(...) Arcane::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define ARC_LOG_WARN(...)  Arcane::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define ARC_LOG_INFO(...)  Arcane::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ARC_LOG_TRACE(...) Arcane::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#endif
