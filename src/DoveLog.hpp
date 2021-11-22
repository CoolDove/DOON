#pragma once
#include <stdio.h>
#include <string>
#include <list>
#include <unordered_map>
#include <mutex>

#ifdef DEBUG
#define DLOG_INIT                 \
    Dove::LogManager::get();      \
    if (Dove::on_init != nullptr) \
    Dove::on_init()
#define DLOG_CLOSE                 \
    if (Dove::on_close != nullptr) \
        Dove::on_close();          \
    delete Dove::LogManager::get()

#define DLOG_ERROR(...)                                                                                             \
    {   char buff[Dove::MAX_LOG_LENGTH];                                                                            \
        snprintf(buff, Dove::MAX_LOG_LENGTH, __VA_ARGS__);                                                          \
        Dove::LogManager::get()->push_msg({Dove::LogType::DLT_ERROR, buff, __FILE__, __FUNCTION__, __LINE__});}

#define DLOG_WARN(...)                                                                                              \
    {   char buff[Dove::MAX_LOG_LENGTH];                                                                            \
        snprintf(buff, Dove::MAX_LOG_LENGTH, __VA_ARGS__);                                                          \
        Dove::LogManager::get()->push_msg({Dove::LogType::DLT_WARN, buff, __FILE__, __FUNCTION__, __LINE__});}

#define DLOG_INFO(...)                                                                                              \
    {   char buff[Dove::MAX_LOG_LENGTH];                                                                            \
        snprintf(buff, Dove::MAX_LOG_LENGTH, __VA_ARGS__);                                                          \
        Dove::LogManager::get()->push_msg({Dove::LogType::DLT_INFO, buff, __FILE__, __FUNCTION__, __LINE__});}

#define DLOG_DEBUG(...)                                                                                             \
    {	char buff[Dove::MAX_LOG_LENGTH];                                                                            \
        snprintf(buff, Dove::MAX_LOG_LENGTH, __VA_ARGS__);                                                          \
        Dove::LogManager::get()->push_msg({Dove::LogType::DLT_DEBUG, buff, __FILE__, __FUNCTION__, __LINE__});}

#define DLOG_TRACE(...)                                                                                        	    \
    {	char buff[Dove::MAX_LOG_LENGTH];                                                                            \
        snprintf(buff, Dove::MAX_LOG_LENGTH, __VA_ARGS__);                                                          \
        Dove::LogManager::get()->push_msg({Dove::LogType::DLT_TRACE, buff, __FILE__, __FUNCTION__, __LINE__});}
#else
#define DLOG_INIT
#define DLOG_ERROR(...)
#define DLOG_WARN(...)
#define DLOG_INFO(...)
#define DLOG_DEBUG(...)
#define DLOG_TRACE(...)
#endif

#define DLOG_ON_PUSH Dove::on_push_msg
#define DLOG_ON_INIT Dove::on_init
#define DLOG_ON_CLOSE Dove::on_close

namespace Dove
{
    using DMSG_FLAG = unsigned char;
    const inline DMSG_FLAG DMSG_FLAG_TYPE = 1 << 0;
    const inline DMSG_FLAG DMSG_FLAG_CONTENT = 1 << 1;
    const inline DMSG_FLAG DMSG_FLAG_FILE = 1 << 2;
    const inline DMSG_FLAG DMSG_FLAG_FUNC = 1 << 3;
    const inline DMSG_FLAG DMSG_FLAG_LINE = 1 << 4;

    const inline DMSG_FLAG DMSG_FLAG_SIMPLE = 1 << 0 | 1 << 1;
    const inline DMSG_FLAG DMSG_FLAG_ALL = 0xFF;

    const inline unsigned int MAX_LOG_LENGTH = 4096;

    const static inline std::string g_log_types[5] = {
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE"};

    enum class LogType : unsigned char
    {
        DLT_ERROR = 0,
        DLT_WARN = 1,
        DLT_INFO = 2,
        DLT_DEBUG = 3,
        DLT_TRACE = 4
    };

    struct LogMsg
    {
        LogType Type;
        std::string Content;
        std::string File;
        std::string Function;
        long Line;

        std::string to_string(DMSG_FLAG _flag = Dove::DMSG_FLAG_SIMPLE) const
        {
            std::string str = "";
            if (_flag & DMSG_FLAG_TYPE)
                str.append(g_log_types[(unsigned char)Type] + ":");
            if (_flag & DMSG_FLAG_CONTENT)
                str.append("\t" + Content);
            if (_flag & DMSG_FLAG_FILE)
                str.append(" -<" + File + "> ");
            if (_flag & DMSG_FLAG_FUNC)
                str.append(" -function: " + this->Function);
            if (_flag & DMSG_FLAG_LINE)
                str.append(" -line: " + std::to_string(Line));
            return str;
        };
    };

    using CallBackOnPushMsg = void (*)(const LogMsg &);
    using CallBackOnInit = void (*)();
    using CallBackOnClose = void (*)();

    inline CallBackOnPushMsg on_push_msg = nullptr;
    inline CallBackOnInit on_init = nullptr;
    inline CallBackOnClose on_close = nullptr;

    class LogManager
    {
    private:
        LogManager()
        {
        }

    public:
        ~LogManager()
        {
            ms_instance = nullptr;
        }
        static LogManager *get()
        {
            if (ms_instance == nullptr)
                ms_instance = new LogManager();

            return ms_instance;
        }

    public:
        static LogManager *ms_instance;
        std::list<LogMsg> &get_log_msgs() noexcept { return m_msgs; }
        std::list<LogMsg *> &get_msgs_by_type(const LogType &_type) noexcept { return m_msgs_by_type[_type]; };
        LogMsg *push_msg(const LogMsg &_msg) noexcept
        {
            static std::mutex mtx;
            mtx.lock();

            m_msgs.emplace_back(_msg);

            LogMsg *pushed_msg = &m_msgs.back();
            m_msgs_by_type[_msg.Type].push_back(pushed_msg);

            if (on_push_msg != nullptr)
                on_push_msg(_msg);

            mtx.unlock();
            return pushed_msg;
        }

    private:
        std::list<LogMsg> m_msgs;
        std::unordered_map<LogType, std::list<LogMsg *>> m_msgs_by_type;
    };

    inline LogManager *LogManager::ms_instance = nullptr;
}

inline Dove::LogManager *DLOG_GET_MANAGER()
{
    return Dove::LogManager::get();
};
inline std::list<Dove::LogMsg> &DLOG_GET_MSGS()
{
    return Dove::LogManager::get()->get_log_msgs();
};
inline std::list<Dove::LogMsg *> &DLOG_GET_MSGS_BY_TYPE(const Dove::LogType &_type)
{
    return Dove::LogManager::get()->get_msgs_by_type(_type);
};
