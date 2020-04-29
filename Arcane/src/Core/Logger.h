#pragma once

#include "Core/Core.h"
#include "Core/Singleton.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Arcane
{
	class ARCANE_API Logger : public Singleton
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
#define ARC_ENGINE_FATAL
#define ARC_ENGINE_ERROR
#define ARC_ENGINE_WARN
#define ARC_ENGINE_INFO
#define ARC_ENGINE_TRACE
#define ARC_GAME_FATAL
#define ARC_GAME_ERROR
#define ARC_GAME_WARN
#define ARC_GAME_INFO
#define ARC_GAME_TRACE
#else
#define ARC_ENGINE_FATAL(...) Arcane::Logger::GetEngineLogger()->critical(__VA_ARGS__)
#define ARC_ENGINE_ERROR(...) Arcane::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define ARC_ENGINE_WARN(...)  Arcane::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define ARC_ENGINE_INFO(...)  Arcane::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ARC_ENGINE_TRACE(...) Arcane::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define ARC_GAME_FATAL(...) Arcane::Logger::GetGameLogger()->critical(__VA_ARGS__)
#define ARC_GAME_ERROR(...) Arcane::Logger::GetGameLogger()->error(__VA_ARGS__)
#define ARC_GAME_WARN(...)  Arcane::Logger::GetGameLogger()->warn(__VA_ARGS__)
#define ARC_GAME_INFO(...)  Arcane::Logger::GetGameLogger()->info(__VA_ARGS__)
#define ARC_GAME_TRACE(...) Arcane::Logger::GetGameLogger()->trace(__VA_ARGS__)
#endif
