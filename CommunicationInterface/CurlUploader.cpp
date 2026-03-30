#include "CurlUploader.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstring>
#include <memory>
#include <QDebug>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include<QJsonObject>
#include<qjsondocument.h>
#include<QJsonArray>
#include<QJsonParseError>
// 辅助：修剪字符串首尾空白
static std::string trimString(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

CurlUploader::CurlUploader()
    : curl_(nullptr),
    custom_header_list_(nullptr),
    method_(Method::POST),
    verify_ssl_(true),
    timeout_(30),
    verbose_(false),
    m_send_url(""),
    last_response_code_(0),
    m_log_request_url("")
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CurlUploader::~CurlUploader() {
    cleanup();
    curl_global_cleanup();
}

void CurlUploader::setMethod(Method method) {
    method_ = method;
}

void CurlUploader::setVerifySSL(bool verify) {
    verify_ssl_ = verify;
}

void CurlUploader::setTimeout(long timeout_seconds) {
    timeout_ = timeout_seconds;
}

void CurlUploader::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void CurlUploader::setCaPath(const std::string& path) {
    ca_certificate_dir_ = path;
}

void CurlUploader::setHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

void CurlUploader::removeHeader(const std::string& key) {
    headers_.erase(key);
}

void CurlUploader::setProgressCallback(ProgressCallback callback) {
    progress_callback_ = callback;
}

void CurlUploader::setResponseCallback(ResponseCallback callback) {
    response_callback_ = callback;
}

bool CurlUploader::uploadFile(const std::string& url, const std::string& file_path, const std::string& field_name) {
    struct stat file_info;
    if (stat(file_path.c_str(), &file_info) != 0) {
        last_error_ = "文件不存在或无法访问: " + file_path;
        return false;
    }

    if (!initCurl()) {
        return false;
    }

    UploadData upload_data;
    upload_data.file = nullptr;
    upload_data.filename = file_path;
    upload_data.filesize = file_info.st_size;
    upload_data.data_offset = 0;
    std::string readBuffer; // 用于存储服务器响应

    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &readBuffer);

    setHeader("Content-Length", std::to_string(file_info.st_size));
    setHeader("Content-Type", "application/octet-stream");

    // 设置通用选项
    if (!setupCommonOptions(url, curl_, upload_data)) {
        return false;
    }

    // 根据方法设置特定选项
    bool success = false;
    if (method_ == Method::PUT) {
        success = setupPutOptions(curl_, upload_data);
    }
    else {
        success = setupPostOptions(curl_, upload_data, field_name);
    }

    if (!success) {
        cleanup();
        return false;
    }

    // 执行上传
    response_content_.clear();
    CURLcode res = curl_easy_perform(curl_);

    // 清理资源
    if (upload_data.file) {
        fclose(upload_data.file);
    }

    // 检查结果
    if (res != CURLE_OK) {
        last_error_ = curl_easy_strerror(res);
        cleanup();
        return false;
    }

    // 获取响应码
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

    // 调用响应回调
    if (response_callback_) {
        response_callback_(response_content_, last_response_code_);
    }

    cleanup();
    return true;
}

bool CurlUploader::uploadData(const std::string& url, const std::string& data, const std::string& field_name) {
    if (!initCurl()) {
        return false;
    }
    UploadData upload_data;
    upload_data.file = nullptr;
    upload_data.filename = "";
    upload_data.filesize = data.size();
    upload_data.data_content = data;
    upload_data.data_offset = 0;

    setHeader("Content-Type", "application/octet-stream");

    // AppId 与 Secret
    if (!m_app_id.empty()) {
        setHeader("AppId", m_app_id);
    }
    if (!m_app_secret.empty()) {
        setHeader("Secret", m_app_secret);
    }

    // 设置通用选项
    if (!setupCommonOptions(url, curl_, upload_data)) {
        return false;
    }
   
    // 根据方法设置特定选项
    bool success = false;
    if (method_ == Method::PUT) {
        success = setupPutOptions(curl_, upload_data);
    }
    else {
        success = setupPostOptions(curl_, upload_data, field_name);
    }

    if (!success) {
        cleanup();
        return false;
    }   

    // 执行上传
    response_content_.clear();
    CURLcode res = curl_easy_perform(curl_);

    // 检查结果
    if (res != CURLE_OK) {
        last_error_ = curl_easy_strerror(res);
        cleanup();
        return false;
    }

    // 获取响应码
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

    // 调用响应回调
    if (response_callback_) {
        response_callback_(response_content_, last_response_code_);
    }

    cleanup();

    //qDebug() << "last_response_code_ is " << last_response_code_;

    if (200 == last_response_code_) {
        return true;
    }
    else {
        return false;
    }
}

std::string CurlUploader::getLastError() const {
    return last_error_;
}

long CurlUploader::getLastResponseCode() const {
    return last_response_code_;
}

std::string CurlUploader::requestLogUrl(const std::string& fileName, const long& fileSize)
{
    CURLcode res;
    std::string readBuffer; // 用于存储服务器响应

    // 参数校验 - 根据协议文档
    if (fileName.empty() || fileSize <= 0 || m_uav_type.empty() || m_sn.empty()) {
        qDebug() << "requestLogUrl input param error!";
        qDebug() << "fileName:" << QString::fromStdString(fileName);
        qDebug() << "fileSize:" << fileSize;
        qDebug() << "uav_type:" << QString::fromStdString(m_uav_type);
        qDebug() << "sn:" << QString::fromStdString(m_sn);
        return "";
    }

    // 如果有sortie，则logType必传 - 根据协议文档
    if (!m_sortie.empty() && m_log_type.empty()) {
        qDebug() << "Error: sortie is not empty, but logType is empty!";
        return "";
    }

    //qDebug() << "requestLogUrl fileName is " << QString::fromStdString(fileName);
    //qDebug() << "requestLogUrl fileSize is " << fileSize;
    //qDebug() << "requestLogUrl uav_type is " << QString::fromStdString(m_uav_type);
    //qDebug() << "requestLogUrl sn is " << QString::fromStdString(m_sn);
    //qDebug() << "requestLogUrl sortie is " << QString::fromStdString(m_sortie);
    //qDebug() << "requestLogUrl logType is " << QString::fromStdString(m_log_type);
    //qDebug() << "requestLogUrl logSrc is " << QString::fromStdString(m_log_src);
    //qDebug() << "requestLogUrl url is " << QString::fromStdString(m_log_request_url);
    //qDebug() << "requestLogUrl appid is " << QString::fromStdString(m_app_id);

    if (!initCurl()) {
        return "";
    }

    if (curl_) {
        if (m_log_request_url.empty()) {
            cleanup();
            return "";
        }

        // 1. 设置目标 URL
        curl_easy_setopt(curl_, CURLOPT_URL, m_log_request_url.c_str());

        // 2. 设置 SSL 验证
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

        // 3. 设置回调函数来处理响应数据
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, logUrlCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &readBuffer);

        // 4. 设置请求头
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // 增加 AppId 和 Secret
        if (!m_app_id.empty()) {
            std::string headerAppId = "AppId: " + m_app_id;
            headers = curl_slist_append(headers, headerAppId.c_str());
        }
        if (!m_app_secret.empty()) {
            std::string headerSecret = "Secret: " + m_app_secret;
            headers = curl_slist_append(headers, headerSecret.c_str());
        }
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        // 5. 创建JSON请求数据 - 根据协议文档
        std::string postData = createRequestJson(fileName, fileSize);

        // 调试输出
        qDebug() << "POST Data: " << QString::fromStdString(postData);

        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, postData.c_str());

        // 6. 执行请求
        res = curl_easy_perform(curl_);

        // 7. 检查错误
        if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            last_error_ = std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
        }
        else {
            qDebug() << "Server Response: " << QString::fromStdString(readBuffer);
            // 8. 解析响应 - 使用自定义解析函数
            m_send_url = parseUploadUrlFromJson(readBuffer);
            if (m_send_url.empty()) {
                qDebug() << "Failed to parse upload URL from response";
            }
        }

        // 释放 headers 列表
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    cleanup();
    return m_send_url;
}

std::string CurlUploader::getResponseMD5()
{
    return m_response_MD5;
}

bool CurlUploader::uploadSuccessReport(const std::string& url, const std::string& logTime, const int logNum,
    const std::string& MetaData ="")
{
    CURLcode res;
    std::string readBuffer; // 用于存储服务器响应

    if (!initCurl()) {
        return false;
    }

    bool overall_success = false;

    if (curl_) {
        if (url.empty()) {
            cleanup();
            return false;
        }

        qDebug() << "uploadSuccessReport send_url is " << QString::fromStdString(m_send_url);

        // 构建查询字符串 - 根据协议文档
        std::string full_url = url + "?";
        full_url += "model=" + urlEncode(m_uav_type) + "&";
        full_url += "path=" + urlEncode(m_log_return_path) + "&";
        full_url += "sn=" + urlEncode(m_sn) + "&";
        full_url += "rcSn=" + urlEncode(m_sn) + "&"; // 地面站sn暂时用设备sn代替
        full_url += "metadata=" + urlEncode(MetaData) + "&";
        full_url += "logSrc=" + urlEncode(m_log_src) + "&";
        full_url += "logType=" + urlEncode(m_log_type) + "&";
        full_url += "logTime=" + urlEncode(logTime) + "&";
        full_url += "logNum=" + std::to_string(logNum);

        // metadata字段可选，这里暂时不传
        // full_url += "&metadata=" + urlEncode("{}");

        qDebug() << "uploadSuccessReport full_url is " << QString::fromStdString(full_url);

        // 设置目标 URL 为带查询参数的 URL
        curl_easy_setopt(curl_, CURLOPT_URL, full_url.c_str());

        // 3. 设置 SSL 验证
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

        // 4. 设置回调函数来处理响应数据
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, logUrlCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &readBuffer);

        // 5. 设置为 GET 请求
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

        // 6. 设置头（保留 AppId/Secret）
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!m_app_id.empty()) {
            std::string headerAppId = "AppId: " + m_app_id;
            headers = curl_slist_append(headers, headerAppId.c_str());
        }
        if (!m_app_secret.empty()) {
            std::string headerSecret = "Secret: " + m_app_secret;
            headers = curl_slist_append(headers, headerSecret.c_str());
        }
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        qDebug() << "uploadSuccessReport GET URL: " << QString::fromStdString(full_url);

        // 7. 执行请求
        res = curl_easy_perform(curl_);

        // 检查错误并解析返回 body 中的 code 字段
        if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            last_error_ = curl_easy_strerror(res);
            overall_success = false;
        }
        else {
            qDebug() << "uploadSuccess Server Response: " << QString::fromStdString(readBuffer);

            // 解析响应
            std::string code = parseJsonValue(readBuffer, "code");
            if (!code.empty() && code == "200") {
                overall_success = true;
                last_response_code_ = 200;
            }
            else {
                std::string msg = parseJsonValue(readBuffer, "msg");
                qDebug() << "Upload success report failed, code:" << QString::fromStdString(code)
                    << "message:" << QString::fromStdString(msg);
                overall_success = false;
                last_response_code_ = code.empty() ? 0 : std::stoi(code);
            }
        }

        // 释放 headers 列表
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    cleanup();

    return overall_success;
}

std::string  CurlUploader::getSortie(const std::string& url,const std::string& sn)
{
    CURLcode res;
    std::string readBuffer; // 用于存储服务器响应

    if (!initCurl()) {
        return false;
    }
    if (curl_) {
        if (url.empty()) {
            cleanup();
            return false;
        }
        // 构建查询字符串 - 根据协议文档
        std::string full_url = url + "?";
        full_url += "sn=" + urlEncode(sn) + "&";

        // 设置目标 URL 为带查询参数的 URL
        curl_easy_setopt(curl_, CURLOPT_URL, full_url.c_str());

        // 3. 设置 SSL 验证
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

        // 4. 设置回调函数来处理响应数据
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, logUrlCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &readBuffer);

        // 5. 设置为 GET 请求
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

        // 6. 设置头（保留 AppId/Secret）
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!m_app_id.empty()) {
            std::string headerAppId = "AppId: " + m_app_id;
            headers = curl_slist_append(headers, headerAppId.c_str());
        }
        if (!m_app_secret.empty()) {
            std::string headerSecret = "Secret: " + m_app_secret;
            headers = curl_slist_append(headers, headerSecret.c_str());
        }
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

     //   qDebug() << "uploadSuccessReport GET URL: " << QString::fromStdString(full_url);

        // 7. 执行请求
        res = curl_easy_perform(curl_);

        // 检查错误并解析返回 body 中的 code 字段
        if (res != CURLE_OK) {
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            last_error_ = curl_easy_strerror(res);
        
        }
        else {
            qDebug() << "uploadSuccess Server Response: " << QString::fromStdString(readBuffer);

            // 解析响应
            std::string code = parseJsonValue(readBuffer, "code");
            std::string sortie= parseJsonValue(readBuffer, "msg");

            if (!code.empty() && code == "200") {
                last_response_code_ = 200;
            }
            else {
                std::string msg = parseJsonValue(readBuffer, "msg");
                qDebug() << "Upload success report failed, code:" << QString::fromStdString(code)
                    << "message:" << QString::fromStdString(msg);
                last_response_code_ = code.empty() ? 0 : std::stoi(code);
            }
            return sortie;
        }

         //释放 headers 列表
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    cleanup();
    return "";

}

// libcurl回调函数实现
size_t CurlUploader::readCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    UploadData* upload_data = static_cast<UploadData*>(userdata);
    size_t buffer_size = size * nmemb;
    if (!upload_data->filename.empty()) {
        // 文件上传
        if (!upload_data->file) {
            upload_data->file = fopen(upload_data->filename.c_str(), "rb");
            if (!upload_data->file) {
                return CURL_READFUNC_ABORT;
            }
        }
        // fread 返回的是读取的项目数量（items），libcurl 期望返回字节数
        size_t bytes_read = fread(ptr, 1, buffer_size, upload_data->file);
        return bytes_read;
    }
    else {
        // 数据上传
        size_t data_remaining = upload_data->data_content.size() - upload_data->data_offset;
        if (data_remaining == 0) {
            return 0; // EOF
        }

        size_t bytes_to_copy = min(buffer_size, data_remaining);
        memcpy(ptr, upload_data->data_content.data() + upload_data->data_offset, bytes_to_copy);
        upload_data->data_offset += bytes_to_copy;
        return bytes_to_copy;
    }
}

size_t CurlUploader::logUrlCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
CurlUploader::MqttCredentials CurlUploader::requestMqttCredentials(
    const std::string& gateway,
    const std::string& clientId,
    const std::string& type)
{
    MqttCredentials credentials;

    if (m_mqttAuthUrl.empty()) {
        credentials.error = "MQTT auth URL not set";
        return credentials;
    }

    // 构建请求
    std::string requestData = makeMqttAuthRequest(clientId, gateway, type);

    // 使用curl发送请求
    CURL* curl = curl_easy_init();
    if (!curl) {
        credentials.error = "Failed to initialize curl";
        return credentials;
    }

    // 设置响应缓冲区
        std::string responseBuffer;

    // 设置curl选项
    curl_easy_setopt(curl, CURLOPT_URL, m_mqttAuthUrl.c_str());
    // 2. 设置 SSL 验证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);
      
    // 设置回调函数和缓冲区
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, logUrlCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    // 4. 设置请求头
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    //// 增加 AppId 和 Secret
    //if (!m_app_id.empty()) {
    //    std::string headerAppId = "AppId: " + m_app_id;
    //    headers = curl_slist_append(headers, headerAppId.c_str());
    //}
    //if (!m_app_secret.empty()) {
    //    std::string headerSecret = "Secret: " + m_app_secret;
    //    headers = curl_slist_append(headers, headerSecret.c_str());
    //}
    headers = curl_slist_append(headers, ("clientid: " + clientId).c_str());  // 添加clientid头部
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());


    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    // 清理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // 检查结果
    if (res != CURLE_OK) {
        credentials.error = std::string("CURL error: ") + curl_easy_strerror(res);
        return credentials;
    }

    // 解析响应
    credentials = parseMqttResponse(responseBuffer);

    return credentials;
}

bool CurlUploader::getLogFileConfig(int pageSize, int pageNum)
{
    if (m_logConfigUrl.empty()) {
        qWarning() << "Log config URL not set";
        return false;
    }

    // 构建请求URL
    std::string url = buildLogConfigRequest(pageSize, pageNum);
    qDebug() << "Requesting log config from:" << QString::fromStdString(url);

    // 使用curl发送请求
    CURL* curl = curl_easy_init();
    if (!curl) {
        qWarning() << "Failed to initialize curl";
        return false;
    }

    // 设置响应缓冲区
    std::string responseBuffer;

    // 设置目标 URL 为带查询参数的 URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 3. 设置 SSL 验证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

    // 4. 设置回调函数来处理响应数据
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, logUrlCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    // 5. 设置为 GET 请求
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    // 6. 设置头（保留 AppId/Secret）
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!m_app_id.empty()) {
        std::string headerAppId = "AppId: " + m_app_id;
        headers = curl_slist_append(headers, headerAppId.c_str());
    }
    if (!m_app_secret.empty()) {
        std::string headerSecret = "Secret: " + m_app_secret;
        headers = curl_slist_append(headers, headerSecret.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    qDebug() << "Requesting log GET URL: " << QString::fromStdString(url);

    // 7. 执行请求
    CURLcode res = curl_easy_perform(curl);

    // 清理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // 检查结果
    if (res != CURLE_OK) {
        qWarning() << "Requesting log CURL error:" << curl_easy_strerror(res);
        return false;
    }
    qDebug() << "Requesting log Server Response: " << QString::fromStdString(responseBuffer);
    // 解析响应
    return parseLogConfigResponse(responseBuffer);
}

std::vector<LogFileConfig> CurlUploader::getLogConfigs() const
{
    return m_logConfigs;
}

int CurlUploader::getTd550FileSize() const
{
    if (m_logConfigs.empty()) {
        return 1; // 默认1MB
    }
    return m_latestConfig.td550FileSize;
}

int CurlUploader::getT1400FileSize() const
{
    if (m_logConfigs.empty()) {
        return 1; // 默认1MB
    }
    return m_latestConfig.t1400FileSize;
}
int CurlUploader::getR6000FileSize() const
{
    if (m_logConfigs.empty()) {
        return 1; // 默认1MB
    }
    return m_latestConfig.R6000FileSize;
}

std::string CurlUploader::getSortie()
{
    return m_sortie;
}

int CurlUploader::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow) {
    CurlUploader* uploader = static_cast<CurlUploader*>(clientp);
    if (uploader->progress_callback_ && ultotal > 0) {
        double progress = static_cast<double>(ulnow) / static_cast<double>(ultotal) * 100.0;
        uploader->progress_callback_(progress, ulnow, ultotal);
    }
    return 0;
}

// 回调函数用于写入响应头
size_t CurlUploader::headerCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    CurlUploader* uploader = static_cast<CurlUploader*>(userdata);
    size_t totalSize = size * nitems;
    std::string headerLine(buffer, totalSize);

    qDebug() << "headerCallback is " << QString::fromStdString(headerLine);

    if (std::string::npos != headerLine.find("ETag")) {
        // 查找冒号的位置
        size_t first_quote = headerLine.find('\"');
        if (std::string::npos != first_quote) {
            size_t second_quote = headerLine.find('\"', first_quote + 1);
            if (std::string::npos != second_quote) {
                if (uploader) {
                    uploader->m_response_MD5 = headerLine.substr(first_quote + 1, second_quote - first_quote - 1);
                    qDebug() << "Got ETag MD5: " << QString::fromStdString(uploader->m_response_MD5);
                }
                else {
                    qDebug() << "uploader is created failed!!!!";
                }
            }
        }
    }

    return totalSize;
}

size_t CurlUploader::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    CurlUploader* uploader = static_cast<CurlUploader*>(userdata);
    size_t total_size = size * nmemb;
    uploader->response_content_.append(ptr, total_size);
    return total_size;
}

// 内部方法实现
bool CurlUploader::initCurl() {
    cleanup();
    curl_ = curl_easy_init();
    if (!curl_) {
        last_error_ = "无法初始化libcurl";
        return false;
    }
    return true;
}

bool CurlUploader::setupCommonOptions(const std::string& url, CURL* curl, UploadData& upload_data) {
    // 设置URL
    int outlen;
    char* decodedUrl = curl_easy_unescape(NULL, url.c_str(), 0, &outlen);

    if (!decodedUrl) {
        last_error_ = "decode url fail: " + url;
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, decodedUrl);
    // 释放 curl_easy_unescape 分配的内存
    curl_free(decodedUrl);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

    if (!ca_certificate_dir_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, ca_certificate_dir_.c_str());
    }

    // 设置超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_);

    // 设置详细模式
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose_ ? 1L : 0L);

    // 设置进度回调
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // 设置头回调函数
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    // 设置响应回调
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    // 设置自定义头
    if (!headers_.empty()) {
        // 释放之前可能存在的自定义头列表
        if (custom_header_list_) {
            curl_slist_free_all(custom_header_list_);
            custom_header_list_ = nullptr;
        }
        struct curl_slist* header_list = nullptr;
        for (const auto& header : headers_) {
            std::string header_str = header.first + ": " + header.second;
            header_list = curl_slist_append(header_list, header_str.c_str());
        }
        custom_header_list_ = header_list;
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, custom_header_list_);
    }

    return true;
}

bool CurlUploader::setupPutOptions(CURL* curl, UploadData& upload_data) {
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, upload_data.filesize);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_data);
    return true;
}

bool CurlUploader::setupPostOptions(CURL* curl, UploadData& upload_data, const std::string& field_name) {
    if (upload_data.filename.empty()) {
        // 数据上传
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, upload_data.filesize);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, upload_data.data_content.c_str());
    }
    else {
        // 文件上传（使用multipart/form-data）
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);

        curl_mime_name(part, field_name.empty() ? "file" : field_name.c_str());
        curl_mime_filedata(part, upload_data.filename.c_str());

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    }
    return true;
}

void CurlUploader::cleanup() {
    // 释放通过 curl_slist 创建的自定义头列表（如果存在）
    if (custom_header_list_) {
        curl_slist_free_all(custom_header_list_);
        custom_header_list_ = nullptr;
    }
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

// JSON处理辅助函数 - 自己实现的版本
std::string CurlUploader::createRequestJson(const std::string& fileName, long fileSize)
{
    // 手动构建JSON字符串 - 根据协议文档
    std::string json = "{";

    // 必需字段 - 根据协议文档
    json += "\"model\":\"" + escapeJsonString(m_uav_type) + "\",";
    json += "\"sn\":\"" + escapeJsonString(m_sn) + "\",";
    json += "\"filename\":\"" + escapeJsonString(fileName) + "\",";
    json += "\"size\":" + std::to_string(fileSize);

    // 可选字段 - 根据协议文档
    if (!m_log_src.empty()) {
        json += ",\"logSrc\":\"" + escapeJsonString(m_log_src) + "\"";
    }

    if (!m_sortie.empty()) {
        json += ",\"sortie\":\"" + escapeJsonString(m_sortie) + "\"";
    }

    // 如果有sortie，则logType必传 - 根据协议文档
    if (!m_sortie.empty() && !m_log_type.empty()) {
        json += ",\"logType\":\"" + escapeJsonString(m_log_type) + "\"";
    }
    else if (!m_log_type.empty()) {
        // 没有sortie时，logType可选
        json += ",\"logType\":\"" + escapeJsonString(m_log_type) + "\"";
    }

    json += "}";

    return json;
}

std::string CurlUploader::parseUploadUrlFromJson(const std::string& jsonResponse)
{
    // 解析服务器响应
    std::string url;
    std::string path;
    std::string filename;
    std::string  sortie;

    // 查找code字段
    std::string code = parseJsonValue(jsonResponse, "code");
    if (code != "200") {
        std::string msg = parseJsonValue(jsonResponse, "msg");
        last_error_ = "API returned error code: " + code + " Message: " + msg;
        qDebug() << "API error:" << QString::fromStdString(last_error_);
        return "";
    }

    // 获取data对象
    std::string dataStr = parseJsonObject(jsonResponse, "data");
    if (!dataStr.empty()) {
        // 从data对象中提取各个字段
        url = parseJsonValue(dataStr, "url");
        path = parseJsonValue(dataStr, "path");
        filename = parseJsonValue(dataStr, "filename");
        m_sortie = parseJsonValue(dataStr, "sortie"); // 提取sortie字段
        // 去除首尾空白
        url = trimString(url);
        path = trimString(path);
        filename = trimString(filename);

        qDebug() << "Parsed response:";
        qDebug() << "  url:" << QString::fromStdString(url);
        qDebug() << "  path:" << QString::fromStdString(path);
        qDebug() << "  filename:" << QString::fromStdString(filename);
        m_log_return_path = path;
        // 检查url是否为空
        if (url.empty()) {
            last_error_ = "Empty url in response data";
            qDebug() << "Error: Empty url in response";
            return "";
        }
    }
    else {
        last_error_ = "No data object in response";
        qDebug() << "Error: No data object in response";
        return "";
    }

    return url;
}

std::string CurlUploader::getReturnPath()
{
    return m_log_return_path;
}

// 辅助函数：转义JSON字符串中的特殊字符
std::string CurlUploader::escapeJsonString(const std::string& str)
{
    std::string result;
    for (char c : str) {
        switch (c) {
        case '\"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '/': result += "\\/"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
        }
    }
    return result;
}

// 辅助函数：解析JSON字符串中的值
// 辅助函数：解析JSON字符串中的值（增强版，支持处理转义字符）
std::string CurlUploader::parseJsonValue(const std::string& json, const std::string& key)
{
    std::string searchStr = "\"" + key + "\":";
    size_t pos = json.find(searchStr);
    if (pos == std::string::npos) {
        return "";
    }

    pos += searchStr.length();

    // 跳过空白字符
    while (pos < json.length() && isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }

    if (pos >= json.length()) {
        return "";
    }

    // 处理字符串值（带引号的）
    if (json[pos] == '\"') {
        pos++; // 跳过开头的引号
        std::string value;
        bool escaped = false;

        while (pos < json.length()) {
            char c = json[pos];

            if (escaped) {
                // 处理转义字符
                switch (c) {
                case '\"': value += '\"'; break;
                case '\\': value += '\\'; break;
                case '/': value += '/'; break;
                case 'b': value += '\b'; break;
                case 'f': value += '\f'; break;
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                case 'u': // Unicode转义，简化处理
                    if (pos + 4 < json.length()) {
                        // 跳过Unicode编码
                        pos += 4;
                        value += '?'; // 用问号代替
                    }
                    break;
                default: value += c; break;
                }
                escaped = false;
            }
            else if (c == '\\') {
                escaped = true;
            }
            else if (c == '\"') {
                // 找到结束引号
                return value;
            }
            else {
                value += c;
            }

            pos++;
        }
    }

    // 处理数字值
    else if (isdigit(static_cast<unsigned char>(json[pos])) || json[pos] == '-') {
        size_t start = pos;
        while (pos < json.length()) {
            char c = json[pos];
            if (!isdigit(static_cast<unsigned char>(c)) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E') {
                break;
            }
            pos++;
        }
        return json.substr(start, pos - start);
    }

    // 处理布尔值 true/false
    else if (json.compare(pos, 4, "true") == 0) {
        return "true";
    }
    else if (json.compare(pos, 5, "false") == 0) {
        return "false";
    }

    // 处理 null
    else if (json.compare(pos, 4, "null") == 0) {
        return "null";
    }

    // 处理对象或数组（返回空字符串，因为parseJsonObject会处理）
    else if (json[pos] == '{' || json[pos] == '[') {
        return "";
    }

    return "";
}

// 辅助函数：解析JSON对象 - 最简实现
// 辅助函数：解析JSON对象
std::string CurlUploader::parseJsonObject(const std::string& json, const std::string& key)
{
    std::string searchStr = "\"" + key + "\":";
    size_t pos = json.find(searchStr);
    if (pos == std::string::npos) {
        return "";
    }

    pos += searchStr.length();

    // 跳过空白字符
    while (pos < json.length() && isspace(static_cast<unsigned char>(json[pos]))) {
        pos++;
    }

    if (pos >= json.length()) {
        return "";
    }

    char startChar = json[pos];

    // 处理对象 {}
    if (startChar == '{') {
        int depth = 1;
        size_t start = pos;
        pos++;

        while (pos < json.length() && depth > 0) {
            char c = json[pos];

            if (c == '\"') {
                // 跳过字符串
                pos++;
                bool escaped = false;
                while (pos < json.length()) {
                    if (escaped) {
                        escaped = false;
                    }
                    else if (json[pos] == '\\') {
                        escaped = true;
                    }
                    else if (json[pos] == '\"') {
                        break;
                    }
                    pos++;
                }
            }
            else if (c == '{') {
                depth++;
            }
            else if (c == '}') {
                depth--;
            }

            pos++;
        }

        if (depth == 0) {
            return json.substr(start, pos - start);
        }
    }

    // 处理数组 []
    else if (startChar == '[') {
        int depth = 1;
        size_t start = pos;
        pos++;

        while (pos < json.length() && depth > 0) {
            char c = json[pos];

            if (c == '\"') {
                // 跳过字符串
                pos++;
                bool escaped = false;
                while (pos < json.length()) {
                    if (escaped) {
                        escaped = false;
                    }
                    else if (json[pos] == '\\') {
                        escaped = true;
                    }
                    else if (json[pos] == '\"') {
                        break;
                    }
                    pos++;
                }
            }
            else if (c == '[') {
                depth++;
            }
            else if (c == ']') {
                depth--;
            }

            pos++;
        }

        if (depth == 0) {
            return json.substr(start, pos - start);
        }
    }

    return "";
}

// 辅助函数：URL编码 - 最简实现
std::string CurlUploader::urlEncode(const std::string& str)
{
    const char hex[] = "0123456789ABCDEF";
    std::string encoded;

    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        }
        else if (c == ' ') {
            encoded += '+';
        }
        else {
            encoded += '%';
            encoded += hex[(c >> 4) & 0xF];
            encoded += hex[c & 0xF];
        }
    }

    return encoded;
}

std::string CurlUploader::buildLogConfigRequest(int pageSize, int pageNum)
{
    // 构建查询参数
    std::string url = m_logConfigUrl;

    if (pageSize > 0 || pageNum > 1) {
        url += "?";
        bool firstParam = true;

        if (pageSize > 0) {
            url += "pageSize=" + std::to_string(pageSize);
            firstParam = false;
        }

        if (pageNum > 0) {
            if (!firstParam) url += "&";
            url += "pageNum=" + std::to_string(pageNum);
        }
    }

    return url;
}

bool CurlUploader::parseLogConfigResponse(const std::string& response)
{
    // 清空现有配置
    m_logConfigs.clear();
    m_latestConfig = LogFileConfig();

    try {
        // 使用Qt的JSON解析
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response), &jsonError);

        if (jsonError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << jsonError.errorString();
            return false;
        }

        if (!doc.isObject()) {
            qWarning() << "Response is not a JSON object";
            return false;
        }

        QJsonObject root = doc.object();

        // 检查code字段
        if (!root.contains("code")) {
            qWarning() << "No 'code' field in response";
            return false;
        }

        int code = root["code"].toInt(-1);
        if (code != 200) {
            QString message = root["msg"].toString("Unknown error");
            qWarning() << "Server error:" << code << "-" << message;
            return false;
        }

        // 检查rows字段
        if (!root.contains("rows") || !root["rows"].isArray()) {
            qWarning() << "No 'rows' array in response";
            return false;
        }

        QJsonArray rows = root["rows"].toArray();
        if (rows.isEmpty()) {
            qWarning() << "No configuration data found";
            return false;
        }

        // 解析每个配置项
        for (const QJsonValue& row : rows) {
            if (row.isObject()) {
                QJsonObject configObj = row.toObject();
                LogFileConfig config;
                config.id = configObj["id"].toString().toStdString();
                config.td550FileSize = configObj["td550FileSize"].toInt(1);  // 默认1MB
                config.t1400FileSize = configObj["t1400FileSize"].toInt(1);  // 默认1MB
                config.R6000FileSize = configObj["r6000FileSize"].toInt(1);      // 默认1MB
                config.fileMergeTime = configObj["fileMergeTime"].toInt(30); // 默认30分钟

                m_logConfigs.push_back(config);

                qDebug() << "Parsed log config:" << QString::fromStdString(config.toString());
            }
        }

        // 设置最新配置（通常是第一个）
        if (!m_logConfigs.empty()) {
            m_latestConfig = m_logConfigs[0];
            qInfo() << "Latest log config:" << QString::fromStdString(m_latestConfig.toString());
        }

        // 打印统计信息
        int total = root["total"].toInt(0);
        qInfo() << "Log configuration loaded:" << m_logConfigs.size() << "items, total:" << total;

        return true;

    }
    catch (const std::exception& e) {
        qCritical() << "Exception while parsing log config:" << e.what();
        return false;
    }
}

int CurlUploader::extractSeqFromFilename(const QString& filename, QString& timestamp, QString& sortieStr)
{
    std::string filenameStr = filename.toStdString();

    // 找最后一个 "."
    size_t dotPos = filenameStr.rfind('.');
    if (dotPos == std::string::npos) {
        return -1;
    }

    // 倒数第 1~5 个下划线
    size_t pos1 = findNthLastUnderscore(filename, 1, dotPos);
    size_t pos2 = findNthLastUnderscore(filename, 2, dotPos);
    size_t pos3 = findNthLastUnderscore(filename, 3, dotPos);
    size_t pos4 = findNthLastUnderscore(filename, 4, dotPos);
    size_t pos5 = findNthLastUnderscore(filename, 5, dotPos);

    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return -1; // 至少序号和时间戳必须存在
    }

    int seq = -1;

    // 提取序号
    std::string seqStr = filenameStr.substr(pos1 + 1, dotPos - pos1 - 1);
    try {
        seq = std::stoi(seqStr);
    }
    catch (...) {
        seq = -1;
    }

    // 提取时间戳
    std::string tsStr = filenameStr.substr(pos2 + 1, pos1 - pos2 - 1);
    long long ts = 0;
    try {
        ts = std::stoll(tsStr);
    }
    catch (...) {
        return -1;
    }

    // 转换为 UTC 时间
    std::time_t t = static_cast<time_t>(ts / 1000); // 如果是毫秒时间戳需要除以1000
    std::tm tm_utc;

#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif

    // 格式化为 yyyyMMddTHH:mm:ssZ
    std::stringstream ss;
    ss << std::put_time(&tm_utc, "%Y%m%dT%H:%M:%SZ");
    timestamp = QString::fromStdString(ss.str());

    // 提取sortie
    if (pos5 != std::string::npos) {
        sortieStr = QString::fromStdString(filenameStr.substr(pos5 + 1, pos4 - pos5 - 1));
    }
    else {
        sortieStr = "";
    }

    return seq;
}

// 获取倒数第 n 个 "_" 的位置（n 从 1 开始）
size_t CurlUploader::findNthLastUnderscore(const QString& str, int n, size_t startPos)
{
    std::string strStd = str.toStdString();

    if (startPos == std::string::npos) {
        startPos = strStd.size();
    }

    size_t pos = startPos;

    for (int i = 0; i < n; ++i) {
        pos = strStd.rfind('_', pos - 1);
        if (pos == std::string::npos) {
            return std::string::npos;
        }
    }
    return pos;
}
// 新增：解析MQTT响应
CurlUploader::MqttCredentials CurlUploader::parseMqttResponse(const std::string& response)
{
    MqttCredentials credentials;
    credentials.port = 1883; // 默认端口

    qDebug() << "mqtt response:" << QString::fromStdString(response);

    try {
        // 首先检查响应是否为空
        if (response.empty()) {
            credentials.error = "Empty response";
            return credentials;
        }

        // 使用Qt的JSON解析（更可靠）
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response), &jsonError);

        if (jsonError.error != QJsonParseError::NoError) {
            credentials.error = std::string("JSON parse error: ") + jsonError.errorString().toStdString();
            return credentials;
        }

        if (!doc.isObject()) {
            credentials.error = "Response is not a JSON object";
            return credentials;
        }

        QJsonObject root = doc.object();

        // 检查code字段
        if (!root.contains("code")) {
            credentials.error = "No 'code' field in response";
            return credentials;
        }

        int code = root["code"].toInt(-1);
        qDebug() << "Response code:" << code;

        // 这里code为200表示成功（根据您的响应）
        if (code != 200 && code != 0) {
            QString message = root["msg"].toString("Unknown error");
            credentials.error = std::string("Server error: ") + message.toStdString();
            return credentials;
        }

        // 检查data字段
        if (!root.contains("data")) {
            credentials.error = "No 'data' field in response";
            return credentials;
        }

        QJsonObject data = root["data"].toObject();
        if (data.isEmpty()) {
            credentials.error = "Empty 'data' field";
            return credentials;
        }

        // 解析clientid
        if (data.contains("clientid")) {
            credentials.clientid = data["clientid"].toString().toStdString();
            qDebug() << "Parsed clientid:" << QString::fromStdString(credentials.clientid);
        }
        else {
            credentials.error = "No 'clientid' field in data";
            return credentials;
        }

        // 解析username
        if (data.contains("username")) {
            credentials.username = data["username"].toString().toStdString();
            qDebug() << "Parsed username:" << QString::fromStdString(credentials.username);
        }
        else {
            credentials.error = "No 'username' field in data";
            return credentials;
        }

        // 解析password
        if (data.contains("password")) {
            credentials.password = data["password"].toString().toStdString();
            qDebug() << "Parsed password length:" << credentials.password.length();
            qDebug() << "Parsed password" << QString::fromStdString(credentials.password);
        }
        else {
            credentials.error = "No 'password' field in data";
            return credentials;
        }

        // 解析server URL（从url字段中提取）
        if (data.contains("url")) {
            QString url = data["url"].toString();
            qDebug() << "Parsed URL:" << url;

            // 解析URL获取服务器和端口
            if (url.startsWith("mqtt://")) {
                QString hostPort = url.mid(7); // 移除"mqtt://"
                QStringList parts = hostPort.split(":");
                if (parts.size() >= 1) {
                    credentials.server = parts[0].toStdString();
                    qDebug() << "Parsed server:" << QString::fromStdString(credentials.server);
                }
                if (parts.size() >= 2) {
                    credentials.port = parts[1].toInt();
                    qDebug() << "Parsed port:" << credentials.port;
                }
            }
        }

        // 解析wsUrl和wssUrl（可选）
        if (data.contains("wsUrl") && !data["wsUrl"].isNull()) {
            credentials.wsUrl = data["wsUrl"].toString().toStdString();
        }

        if (data.contains("wssUrl") && !data["wssUrl"].isNull()) {
            credentials.wssUrl = data["wssUrl"].toString().toStdString();
        }

        // 解析key（可选）
        if (data.contains("key")) {
            credentials.key = data["key"].toString().toStdString();
        }

        qDebug() << "MQTT credentials parsed successfully";

    }
    catch (const std::exception& e) {
        credentials.error = std::string("Parse error: ") + e.what();
        qCritical() << "Parse exception:" << e.what();
    }

    return credentials;
}
