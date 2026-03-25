#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <curl/curl.h>
#include <QString>
#include <cstddef>  // 对于 size_t
// 内部结构体
struct UploadData {
    FILE* file;
    std::string filename;
    curl_off_t filesize;
    size_t data_offset;
    std::string data_content;
};
// 新增日志配置结构体
struct LogFileConfig {
    std::string id;               // 配置ID
    int td550FileSize;           // td550日志上传文件大小（MB）
    int t1400FileSize;           // t1400日志上传文件大小（MB）
    int R6000FileSize;             // 小飞机日志上传限制大小（MB）
    int fileMergeTime;           // 日志拼接间隔时间（分钟）

    // 转换为字符串表示
    std::string toString() const {
        return "ID: " + id +
            ", TD550文件大小: " + std::to_string(td550FileSize) + "MB" +
            ", T1400文件大小: " + std::to_string(t1400FileSize) + "MB" +
            ",R6000文件大小: " + std::to_string(R6000FileSize) + "MB" +
            ", 文件合并时间: " + std::to_string(fileMergeTime) + "分钟";
    }
};

class CurlUploader {
public:
    // 上传方法枚举
    enum class Method {
        PUT,
        POST
    };

    // 进度回调类型
    using ProgressCallback = std::function<void(double progress, curl_off_t uploaded, curl_off_t total)>;

    // 响应回调类型
    using ResponseCallback = std::function<void(const std::string& response, long status_code)>;

    CurlUploader();
    ~CurlUploader();

    // 禁用拷贝构造和赋值
    CurlUploader(const CurlUploader&) = delete;
    CurlUploader& operator=(const CurlUploader&) = delete;

    // 设置基本选项
    void setMethod(Method method);
    void setVerifySSL(bool verify);
    void setTimeout(long timeout_seconds);
    void setVerbose(bool verbose);
    void setCaPath(const std::string& path);

    // 设置自定义头
    void setHeader(const std::string& key, const std::string& value);
    void removeHeader(const std::string& key);

    // 设置回调函数
    void setProgressCallback(ProgressCallback callback);
    void setResponseCallback(ResponseCallback callback);

    // 文件上传方法
    bool uploadFile(const std::string& url, const std::string& file_path, const std::string& field_name = "");
    bool uploadData(const std::string& url, const std::string& data, const std::string& field_name = "");
    int extractSeqFromFilename(const QString& filename, QString& seq, QString& name);
    size_t findNthLastUnderscore(const QString& str, int n, size_t startPos = 0);
    // 获取错误信息
    std::string getLastError() const;
    long getLastResponseCode() const;

    // 请求日志上传链接 - 接口1：获取上传授权路径
    std::string requestLogUrl(const std::string& fileName, const long& fileSize);

    // 上传成功报告 - 接口2：上传成功接口
    bool uploadSuccessReport(const std::string& url, const std::string& logTime, const int logNum,
        const std::string& metadata);
    //获取架次信息
    std::string  getSortie(const std::string& url,const std::string& sn);

    // 获取返回MD5
    std::string getResponseMD5();

    // 获取响应路径path
    std::string getReturnPath();

    // 设置日志请求URL
    void setLogRequestUrl(const std::string& url) {
        m_log_request_url = url;
    }

    // 设置appId和appSecret
    void setAppIdSecret(const std::string& appId, const std::string& appSecret) {
        m_app_id = appId;
        m_app_secret = appSecret;
    }

    // 设置日志参数 - 根据协议文档
    void setLogParam(const std::string& uav_type, const std::string& sn,
        const std::string& sortie, const std::string& log_type,
        const std::string& log_src) {
        m_uav_type = uav_type;
        m_sn = sn;
        m_sortie = sortie;
        m_log_type = log_type;
        m_log_src = log_src;
    }
    struct MqttCredentials {
        std::string clientid;     // 新增：从响应中获取的clientid
        std::string username;     // 用户名
        std::string password;     // 密码
        std::string server;       // 服务器地址
        int port;                 // 端口
        std::string wsUrl;        // WebSocket URL（可选）
        std::string wssUrl;       // WebSocket Secure URL（可选）
        std::string key;          // 密钥（可选）
        std::string error;        // 错误信息
    };
    MqttCredentials requestMqttCredentials(
        const std::string& gateway,
        const std::string& clientId="e5cd7e4891bf95d1d19206ce24a7b32e",
        const std::string& type = "station");
    void setMqttAuthUrl(const std::string& url)
    {
        m_mqttAuthUrl = url;
    }

    void setLogConfigUrl(const std::string& url) { m_logConfigUrl = url; }
    std::string getLogConfigUrl() const { return m_logConfigUrl; }
    bool getLogFileConfig(int pageSize = 10, int pageNum = 1);
    std::vector<LogFileConfig> getLogConfigs() const;

    // 获取特定型号的配置
    int getTd550FileSize() const;
    int getT1400FileSize() const;
    int getR6000FileSize() const;

    //返回架次号
    std::string  getSortie();
private:
    // libcurl回调函数
    static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
        curl_off_t ultotal, curl_off_t ulnow);
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t readCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t logUrlCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata);

    // 内部方法
    bool initCurl();
    bool setupCommonOptions(const std::string& url, CURL* curl, UploadData& upload_data);
    bool setupPutOptions(CURL* curl, UploadData& upload_data);
    bool setupPostOptions(CURL* curl, UploadData& upload_data, const std::string& field_name);
    void cleanup();

    // JSON处理辅助函数
    std::string createRequestJson(const std::string& fileName, long fileSize);
    std::string parseUploadUrlFromJson(const std::string& jsonResponse);

    // 辅助函数：转义JSON字符串中的特殊字符
    static std::string escapeJsonString(const std::string& str);

    // 辅助函数：解析JSON字符串中的值
    static std::string parseJsonValue(const std::string& json, const std::string& key);

    // 辅助函数：解析JSON对象
    static std::string parseJsonObject(const std::string& json, const std::string& key);

    // 辅助函数：URL编码
    static std::string urlEncode(const std::string& str);
	std::string buildLogConfigRequest(int pageSize, int pageNum);
    bool parseLogConfigResponse(const std::string& response);
    // 新增私有方法
   // std::string makeMqttAuthRequest(const std::string& clientId,const std::string& gateway,const std::string& type);
    MqttCredentials parseMqttResponse(const std::string& response);



    // 新增：构建MQTT认证请求
    std::string makeMqttAuthRequest(const std::string& clientId,
        const std::string& gateway,
        const std::string& type)
    {
        // 构建JSON请求体
        std::string jsonData = "{\"gateway\":\"" + gateway +
            "\",\"type\":\"" + type + "\"}";

        return jsonData;
    }
    // 新增成员变量
    std::string m_mqttAuthUrl;

private:
    // libcurl相关
    CURL* curl_;
    struct curl_slist* custom_header_list_;  // 自定义HTTP头部列表

    // 基本配置选项
    Method method_;
    bool verify_ssl_;
    long timeout_;
    bool verbose_;

    // 错误和响应信息
    std::string last_error_;
    long last_response_code_;

    // 证书和路径
    std::string ca_certificate_dir_;

    // HTTP头部管理
    std::map<std::string, std::string> headers_;

    // 回调函数
    ProgressCallback progress_callback_;
    ResponseCallback response_callback_;
    std::string response_content_;  // 响应内容

    // MD5校验
    std::string m_response_MD5;

    // 上传相关
    std::string m_send_url;          // 上传URL    

    // 协议参数 - 根据协议文档
    std::string m_log_request_url;   // 接口1路径：lfy-api/manage/logFile/getUploadUrl
    std::string m_app_id;            // AppId
    std::string m_app_secret;        // Secret
    std::string m_log_sucess_url;
    std::string m_log_return_path;// 响应路径（接口1返回的path）
    // 日志上传参数
    std::string m_uav_type;          // model - 设备型号（必传）
    std::string m_sn;                // sn - 设备序列号（必传）
    std::string m_sortie;            // sortie - 日志架次（可选，有sortie时logType必传）
    std::string m_log_type;          // logType - 日志类型（有sortie时必传）
    std::string m_log_src;           // logSrc - 日志上报来源（可选）
                                     // DRONE_ONCOMP：无人机机载上报
                                     // GROUND_STATION：地面站上报
    // 新增URL
    std::string m_logConfigUrl;
    // 新增成员变量
    std::vector<LogFileConfig> m_logConfigs;
    LogFileConfig m_latestConfig;

};