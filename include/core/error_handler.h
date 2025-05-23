#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace neumann {

/**
 * @brief 错误代码枚举
 */
enum class ErrorCode {
    SUCCESS = 0,

    // 文件相关错误 (100-199)
    FILE_NOT_FOUND = 100,
    FILE_READ_ERROR = 101,
    FILE_WRITE_ERROR = 102,
    FILE_PERMISSION_ERROR = 103,

    // 数据相关错误 (200-299)
    INVALID_DATA_FORMAT = 200,
    INSUFFICIENT_DATA_POINTS = 201,
    DATA_OUT_OF_RANGE = 202,
    DATA_PARSING_ERROR = 203,

    // 计算相关错误 (300-399)
    INVALID_CONFIDENCE_LEVEL = 300,
    CALCULATION_ERROR = 301,
    STANDARD_VALUES_NOT_FOUND = 302,

    // 网络相关错误 (400-499)
    NETWORK_ERROR = 400,
    PORT_IN_USE = 401,
    WEB_SERVER_ERROR = 402,

    // 配置相关错误 (500-599)
    CONFIG_LOAD_ERROR = 500,
    CONFIG_SAVE_ERROR = 501,
    INVALID_CONFIG_VALUE = 502,

    // 系统相关错误 (600-699)
    MEMORY_ERROR = 600,
    SYSTEM_ERROR = 601,
    PERMISSION_DENIED = 602,

    // 未知错误
    UNKNOWN_ERROR = 999
};

/**
 * @brief 错误信息结构
 */
struct ErrorInfo {
    ErrorCode code;
    std::string message;
    std::string suggestion;
    std::string technicalDetails;
};

/**
 * @brief 应用程序异常类
 */
class NeumannException : public std::runtime_error
{
public:
    NeumannException(ErrorCode code, const std::string &message = "",
                     const std::string &suggestion = "", const std::string &technicalDetails = "");

    ErrorCode getErrorCode() const;
    std::string getSuggestion() const;
    std::string getTechnicalDetails() const;
    ErrorInfo getErrorInfo() const;

private:
    ErrorCode errorCode;
    std::string suggestion;
    std::string technicalDetails;
};

/**
 * @brief 错误处理器类
 *
 * 提供统一的错误处理、日志记录和用户友好的错误信息
 */
class ErrorHandler
{
public:
    /**
     * @brief 获取ErrorHandler单例实例
     * @return ErrorHandler的共享实例
     */
    static ErrorHandler &getInstance();

    /**
     * @brief 处理错误
     * @param code 错误代码
     * @param context 错误上下文信息
     * @param technicalDetails 技术详细信息
     * @return 错误信息结构
     */
    ErrorInfo handleError(ErrorCode code, const std::string &context = "",
                          const std::string &technicalDetails = "");

    /**
     * @brief 获取错误描述
     * @param code 错误代码
     * @return 用户友好的错误描述
     */
    std::string getErrorMessage(ErrorCode code) const;

    /**
     * @brief 获取错误建议
     * @param code 错误代码
     * @return 解决建议
     */
    std::string getErrorSuggestion(ErrorCode code) const;

    /**
     * @brief 记录错误到日志
     * @param errorInfo 错误信息
     */
    void logError(const ErrorInfo &errorInfo);

    /**
     * @brief 显示错误给用户
     * @param errorInfo 错误信息
     * @param showTechnicalDetails 是否显示技术详细信息
     */
    void displayError(const ErrorInfo &errorInfo, bool showTechnicalDetails = false);

    /**
     * @brief 设置是否启用详细错误信息
     * @param enable 是否启用
     */
    void setVerboseErrors(bool enable);

private:
    // 私有构造函数
    ErrorHandler();

    // 禁用拷贝和赋值
    ErrorHandler(const ErrorHandler &) = delete;
    ErrorHandler &operator=(const ErrorHandler &) = delete;

    // 初始化错误消息
    void initializeErrorMessages();

    // 错误消息映射
    std::map<ErrorCode, std::string> errorMessages;
    std::map<ErrorCode, std::string> errorSuggestions;

    // 是否启用详细错误信息
    bool verboseErrors;
};

// 便利宏定义
#define THROW_ERROR(code, context) throw neumann::NeumannException(code, "", "", context)

#define HANDLE_ERROR(code, context) neumann::ErrorHandler::getInstance().handleError(code, context)

}  // namespace neumann