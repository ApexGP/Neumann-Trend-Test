#include "core/error_handler.h"

#include <iostream>
#include <sstream>

#include "core/i18n.h"

namespace neumann {

// NeumannException 实现
NeumannException::NeumannException(ErrorCode code, const std::string &message,
                                   const std::string &suggestion,
                                   const std::string &technicalDetails)
    : std::runtime_error(message.empty() ? ErrorHandler::getInstance().getErrorMessage(code)
                                         : message),
      errorCode(code),
      suggestion(suggestion.empty() ? ErrorHandler::getInstance().getErrorSuggestion(code)
                                    : suggestion),
      technicalDetails(technicalDetails)
{
}

ErrorCode NeumannException::getErrorCode() const
{
    return errorCode;
}

std::string NeumannException::getSuggestion() const
{
    return suggestion;
}

std::string NeumannException::getTechnicalDetails() const
{
    return technicalDetails;
}

ErrorInfo NeumannException::getErrorInfo() const
{
    return {errorCode, what(), suggestion, technicalDetails};
}

// ErrorHandler 实现
ErrorHandler::ErrorHandler() : verboseErrors(false)
{
    initializeErrorMessages();
}

ErrorHandler &ErrorHandler::getInstance()
{
    static ErrorHandler instance;
    return instance;
}

ErrorInfo ErrorHandler::handleError(ErrorCode code, const std::string &context,
                                    const std::string &technicalDetails)
{
    ErrorInfo info;
    info.code = code;
    info.message = getErrorMessage(code);
    info.suggestion = getErrorSuggestion(code);
    info.technicalDetails = technicalDetails;

    if (!context.empty()) {
        info.message += " (" + context + ")";
    }

    // 记录错误
    logError(info);

    return info;
}

std::string ErrorHandler::getErrorMessage(ErrorCode code) const
{
    auto it = errorMessages.find(code);
    if (it != errorMessages.end()) {
        return _(it->second);
    }
    return _("error.unknown");
}

std::string ErrorHandler::getErrorSuggestion(ErrorCode code) const
{
    auto it = errorSuggestions.find(code);
    if (it != errorSuggestions.end()) {
        return _(it->second);
    }
    return _("suggestion.contact_support");
}

void ErrorHandler::logError(const ErrorInfo &errorInfo)
{
    // 简单的错误日志记录到标准错误输出
    std::cerr << "[ERROR " << static_cast<int>(errorInfo.code) << "] " << errorInfo.message
              << std::endl;

    if (verboseErrors && !errorInfo.technicalDetails.empty()) {
        std::cerr << "[TECHNICAL] " << errorInfo.technicalDetails << std::endl;
    }
}

void ErrorHandler::displayError(const ErrorInfo &errorInfo, bool showTechnicalDetails)
{
    std::cout << "\n" << _("status.error") << ": " << errorInfo.message << std::endl;

    if (!errorInfo.suggestion.empty()) {
        std::cout << _("suggestion.title") << ": " << errorInfo.suggestion << std::endl;
    }

    if (showTechnicalDetails && !errorInfo.technicalDetails.empty()) {
        std::cout << _("technical.details") << ": " << errorInfo.technicalDetails << std::endl;
    }

    std::cout << std::endl;
}

void ErrorHandler::setVerboseErrors(bool enable)
{
    verboseErrors = enable;
}

void ErrorHandler::initializeErrorMessages()
{
    // 文件相关错误
    errorMessages[ErrorCode::FILE_NOT_FOUND] = "error.file_not_found";
    errorSuggestions[ErrorCode::FILE_NOT_FOUND] = "suggestion.check_file_path";

    errorMessages[ErrorCode::FILE_READ_ERROR] = "error.file_read_error";
    errorSuggestions[ErrorCode::FILE_READ_ERROR] = "suggestion.check_file_permissions";

    errorMessages[ErrorCode::FILE_WRITE_ERROR] = "error.file_write_error";
    errorSuggestions[ErrorCode::FILE_WRITE_ERROR] = "suggestion.check_write_permissions";

    errorMessages[ErrorCode::FILE_PERMISSION_ERROR] = "error.file_permission_error";
    errorSuggestions[ErrorCode::FILE_PERMISSION_ERROR] = "suggestion.run_as_admin";

    // 数据相关错误
    errorMessages[ErrorCode::INVALID_DATA_FORMAT] = "error.invalid_data";
    errorSuggestions[ErrorCode::INVALID_DATA_FORMAT] = "suggestion.check_data_format";

    errorMessages[ErrorCode::INSUFFICIENT_DATA_POINTS] = "error.insufficient_data";
    errorSuggestions[ErrorCode::INSUFFICIENT_DATA_POINTS] = "suggestion.add_more_data";

    errorMessages[ErrorCode::DATA_OUT_OF_RANGE] = "error.data_out_of_range";
    errorSuggestions[ErrorCode::DATA_OUT_OF_RANGE] = "suggestion.check_data_values";

    errorMessages[ErrorCode::DATA_PARSING_ERROR] = "error.data_parsing_error";
    errorSuggestions[ErrorCode::DATA_PARSING_ERROR] = "suggestion.fix_data_format";

    // 计算相关错误
    errorMessages[ErrorCode::INVALID_CONFIDENCE_LEVEL] = "error.invalid_confidence_level";
    errorSuggestions[ErrorCode::INVALID_CONFIDENCE_LEVEL] = "suggestion.use_valid_confidence";

    errorMessages[ErrorCode::CALCULATION_ERROR] = "error.calculation_failed";
    errorSuggestions[ErrorCode::CALCULATION_ERROR] = "suggestion.check_input_data";

    errorMessages[ErrorCode::STANDARD_VALUES_NOT_FOUND] = "error.standard_values_not_found";
    errorSuggestions[ErrorCode::STANDARD_VALUES_NOT_FOUND] =
        "suggestion.check_standard_values_file";

    // 网络相关错误
    errorMessages[ErrorCode::NETWORK_ERROR] = "error.network_error";
    errorSuggestions[ErrorCode::NETWORK_ERROR] = "suggestion.check_network";

    errorMessages[ErrorCode::PORT_IN_USE] = "error.port_in_use";
    errorSuggestions[ErrorCode::PORT_IN_USE] = "suggestion.try_different_port";

    errorMessages[ErrorCode::WEB_SERVER_ERROR] = "error.web_server_error";
    errorSuggestions[ErrorCode::WEB_SERVER_ERROR] = "suggestion.restart_server";

    // 配置相关错误
    errorMessages[ErrorCode::CONFIG_LOAD_ERROR] = "error.config_load_error";
    errorSuggestions[ErrorCode::CONFIG_LOAD_ERROR] = "suggestion.reset_config";

    errorMessages[ErrorCode::CONFIG_SAVE_ERROR] = "error.config_save_error";
    errorSuggestions[ErrorCode::CONFIG_SAVE_ERROR] = "suggestion.check_write_permissions";

    errorMessages[ErrorCode::INVALID_CONFIG_VALUE] = "error.invalid_config_value";
    errorSuggestions[ErrorCode::INVALID_CONFIG_VALUE] = "suggestion.reset_config";

    // 系统相关错误
    errorMessages[ErrorCode::MEMORY_ERROR] = "error.memory_error";
    errorSuggestions[ErrorCode::MEMORY_ERROR] = "suggestion.reduce_data_size";

    errorMessages[ErrorCode::SYSTEM_ERROR] = "error.system_error";
    errorSuggestions[ErrorCode::SYSTEM_ERROR] = "suggestion.contact_support";

    errorMessages[ErrorCode::PERMISSION_DENIED] = "error.permission_denied";
    errorSuggestions[ErrorCode::PERMISSION_DENIED] = "suggestion.run_as_admin";

    // 未知错误
    errorMessages[ErrorCode::UNKNOWN_ERROR] = "error.unknown";
    errorSuggestions[ErrorCode::UNKNOWN_ERROR] = "suggestion.contact_support";
}

}  // namespace neumann