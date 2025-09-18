#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <ctime>
#include <filesystem>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/chrono.h"

enum class e_Loglevel
{
    LOG_NONE,
    LOG_NOTICE,
    LOG_WARNING,
    LOG_LETHAL,
    LOG_DEBUG,
    LOG_MAX
};

static constexpr unsigned int MICROSECONDS_IN_MINUTE = 60000000;

template< typename T >
constexpr uint8_t GetDatatype()
{
    if constexpr ( std::is_same_v< T, unsigned long > || std::is_same_v< T, unsigned int > )
        return 1;
    else if constexpr ( std::is_same_v< T, long > || std::is_same_v< T, int > )
        return 2;
    else if constexpr ( std::is_same_v< T, double > || std::is_same_v< T, float > )
        return 3;
    else
        return 0;
}

inline void
DeleteOldestFiles( const std::filesystem::path& directoryPath )
{
    static constexpr int maxLogFiles = 20;
    std::vector< std::filesystem::directory_entry > files;
    for ( const auto& file : std::filesystem::directory_iterator( directoryPath ) )
    {
        if ( file.is_regular_file() && file.path().extension() == ".txt" )
            files.push_back( file );
    }

    if ( files.size() < maxLogFiles )
        return;

    auto sortAlgorytm = []( const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b ) {
        return a.last_write_time() > b.last_write_time();
    };
    std::sort( files.begin(), files.end(), sortAlgorytm );

    for ( unsigned int i = maxLogFiles; i < files.size(); i++ )
    {
        try
        {
            std::filesystem::remove( files[ i ].path() );
        } catch ( const std::exception& e )
        {
            fmt::print( "Error while deleting file : {}", e.what() );
        }
    }
}

inline std::string CreateLogFile()
{
    if ( !std::filesystem::is_directory( "Logs" ) || !std::filesystem::exists( "Logs" ) )
        std::filesystem::create_directory( "Logs" );

    DeleteOldestFiles( "Logs/" );
    std::time_t currentTime = std::time( nullptr );
    return fmt::format( "Logs/log_{:%Y-%m-%d_%H-%M}.txt", fmt::localtime( currentTime ) );
}

inline constexpr std::string_view RemoveReturnTypeFromFunctionName( const char* functionName )
{
    size_t begin = 0;
    size_t end = 0;
    for ( size_t i = 0; functionName[ i ] != '\0'; ++i )
    {
        if ( functionName[ i ] == ' ' )
            begin = i + 1;
        if ( functionName[ i ] == '(' )
        {
            end = i - begin;
            break;
        }
    }
    return std::string_view( functionName + begin, end );
}

inline void ClearAllSeparatorsFromLine( std::string& line, const std::string& separator )
{
    size_t pos = 0;
    while ( ( pos = line.find( separator, pos ) ) != std::string::npos )
        line.erase( pos, separator.length() );
    while ( ( pos = line.find( '\n', pos ) ) != std::string::npos )
        line.erase( pos, separator.length() );
}

inline std::string LogLevelToString( e_Loglevel level )
{
    switch ( level )
    {
        case e_Loglevel::LOG_NONE:
            return "NONE";
        case e_Loglevel::LOG_DEBUG:
            return "DEBUG";
        case e_Loglevel::LOG_NOTICE:
            return "NOTICE";
        case e_Loglevel::LOG_WARNING:
            return "WARNING";
        case e_Loglevel::LOG_LETHAL:
            return "LETHAL";
        default:
            return "UNKNOWN";
    }
}

class LogManager
{
public:
    ~LogManager() = default;
    LogManager( const LogManager& ) = delete;
    static LogManager& Get()
    {
        static LogManager instance;

        return instance;
    }

    inline static void IgnoreLogLevelBelow( e_Loglevel logLevel )
    {
        Get().LogLevelIgnoredBelow_ = logLevel;
    }

    inline static void AddCallbackForLogLevel( e_Loglevel logLevel, std::function< void( std::string ) > function )
    {
        Get().callbackFunctions_[ static_cast< int >( logLevel ) ] = function;
    }

    template< typename... Args >
    inline static void LogError( bool condition, const char* fullFunctionName, int line, e_Loglevel level, const char* message, Args&&... args )
    {
        Get().ILogError( condition, fullFunctionName, line, level, message, std::forward< Args >( args )... );
    }

    template< typename... Args >
    inline static std::string FormatToString( const char* stringToFormat, Args&&... args )
    {
        return Get().IFormatToString( stringToFormat, std::forward< Args >( args )... );
    }

    inline static std::string GetSeparator()
    {
        return Get().separator_;
    }

private:
    LogManager()
        : LogLevelIgnoredBelow_( e_Loglevel::LOG_NONE )
        , callbackFunctions_( static_cast< int >( e_Loglevel::LOG_MAX ) )
        , tickDurationMs_( 1000 )
        , shouldTerminate_( false )
        , isMultithreadActivated_( false )
    {
        startTime_ = std::chrono::steady_clock::now();
        logFile_.open( CreateLogFile() );
    }

    template< typename... Args >
    void ILogError( bool condition, const char* fullFunctionName, int line, e_Loglevel level, const char* message, Args&&... args )
    {
        if ( !condition )
            return;

        std::string strMessage;
        std::ostringstream errorMsg;
        try
        {
            if constexpr ( sizeof...( args ) > 0 )
                strMessage = fmt::format( fmt::runtime( message ), std::forward< Args >( args )... );
            else
                strMessage = message;
        }
        catch ( const std::exception& logError )
        {
            fmt::print( "LogManager format Error : {}", logError.what() );
            std::exit( EXIT_FAILURE );
        }

        ClearAllSeparatorsFromLine( strMessage, separator_ );
        std::string_view fullFunctionNameStr = RemoveReturnTypeFromFunctionName( fullFunctionName );

        std::time_t currentTime = std::time( nullptr );
        std::string timeString = fmt::format( "{:%H:%M:%S} || ", fmt::localtime( currentTime ) );

        errorMsg << timeString << LogLevelToString( level ) << separator_ << fullFunctionNameStr << separator_ << "Line : " << line << separator_ << strMessage;

        if ( level >= LogLevelIgnoredBelow_ )
            AddMessage( errorMsg.str() );

        if ( level == e_Loglevel::LOG_LETHAL )
            throw std::runtime_error( errorMsg.str() );

        if ( callbackFunctions_[ static_cast< int >( level ) ] )
            callbackFunctions_[ static_cast< int >( level ) ]( strMessage );
    }

    template< typename... Args >
    std::string IFormatToString( const char* stringToFormat, Args&&... args )
    {
        std::string strMessage = stringToFormat;
        try
        {
            if constexpr ( sizeof...( args ) > 0 )
                strMessage = fmt::format( fmt::runtime( strMessage ), std::forward< Args >( args )... );
            else
                strMessage = strMessage;
        } 
        catch ( const std::exception& logError )
        {
            fmt::print( "LogManager Error : {}", logError.what() );
            std::exit( EXIT_FAILURE );
        }
        return strMessage;
    }

    void AddMessage( const std::string& message )
    {
        logFile_ << message << "\n";
        logFile_.flush();

        if ( isMultithreadActivated_ )
        {
            std::lock_guard< std::mutex > guard( messageVectorLock_ );
            messages_.push_back( message );
        }
        else
        {
            std::cout << message << "\n";
        }
    }

private:
    e_Loglevel LogLevelIgnoredBelow_;
    std::chrono::steady_clock::time_point startTime_;
    std::ofstream logFile_;
    std::vector< std::function< void( std::string ) > > callbackFunctions_;
    std::mutex messageVectorLock_;
    std::vector< std::string > messages_;
    int tickDurationMs_;
    int tickNumber_;
    bool shouldTerminate_;
    std::thread logThread_;
    bool isMultithreadActivated_;
    const std::string separator_ = " || ";
};

#ifndef NDEBUG
#    ifdef __GNUG__
#        define FULL_FUNCTION_NAME __PRETTY_FUNCTION__
#    else
#        define FULL_FUNCTION_NAME __FUNCTION__
#    endif

#    define LOG_DEBUG( message, ... ) LogManager::LogError( true, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_DEBUG, message, ##__VA_ARGS__ )
#    define LOG_NOTICE( message, ... ) LogManager::LogError( true, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_NOTICE, message, ##__VA_ARGS__ )
#    define LOG_WARNING( message, ... ) LogManager::LogError( true, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_WARNING, message, ##__VA_ARGS__ )
#    define LOG_LETHAL( message, ... ) LogManager::LogError( true, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_LETHAL, message, ##__VA_ARGS__ )
#    define LOG_CONDITIONAL_NOTICE( condition, message, ... ) LogManager::LogError( condition, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_NOTICE, message, ##__VA_ARGS__ )
#    define LOG_CONDITIONAL_WARNING( condition, message, ... ) LogManager::LogError( condition, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_WARNING, message, ##__VA_ARGS__ )
#    define LOG_CONDITIONAL_LETHAL( condition, message, ... ) LogManager::LogError( condition, FULL_FUNCTION_NAME, __LINE__, e_Loglevel::LOG_LETHAL, message, ##__VA_ARGS__ )
#    define LOG_TERMINATE() LogManager::TerminateThread()
#    define LOG_ACTIVATE_MULTITHREAD( x ) LogManager::SetMultithreadActivate( x )
#    define LOG_IGNORE_BELOW( x ) LogManager::IgnoreLogLevelBelow( x )

#else
#    define LOG_DEBUG( message, ... )
#    define LOG_NOTICE( message, ... )
#    define LOG_WARNING( message, ... )
#    define LOG_LETHAL( message, ... )
#    define LOG_CONDITIONAL_NOTICE( condition, message, ... )
#    define LOG_CONDITIONAL_WARNING( condition, message, ... )
#    define LOG_CONDITIONAL_LETHAL( condition, message, ... )
#    define LOG_TERMINATE()
#    define LOG_ACTIVATE_MULTITHREAD( x )
#    define LOG_IGNORE_BELOW( x )
#endif

#endif // !LOG_MANAGER_H