
#include "UomDataUpload.h"

struct MemoryData
{
	std::string data;
};


UomDataUpload::UomDataUpload()
	
{
	/*m_appid = "admin";
	m_appkey  = "s3jhh0vr9-7c78-4819-3c32-ec4cd2e000";  
	//m_token = "dbd39d77c3c0f05cbbafc3ca9f83cdbcd7942a1ab531094cd53e55a1e67617d9";

	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
	{
		printf("curl_global_init failed.");
	}*/
}


UomDataUpload::~UomDataUpload()
{
	//curl_global_cleanup(); 
}
 
 

/*void UomDataUpload::uploadData(const UomUAVSate& state)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(std::move(state));
	}

	AsyncTask::resume();
}


 
bool UomDataUpload::execute()
{
	if (!isTokenValid())
	{
		//请求token
		if (!requestToken(m_appid, m_appkey))
		{
			//等待2秒
			std::this_thread::sleep_for(std::chrono::seconds(2));

			//返回ture 继续循环请求
			return true;
		}
		//else
		//{
		//	//启动定时器，连续发送;
		//	printf("-----\n");
		//}
	}


	std::unique_lock<std::mutex> lock(mutex); 

	if (queue.empty())
	{
		lock.unlock();

		AsyncTask::pause();
		return true;
	}

	//获取数据
	UomUAVSate state = std::move(queue.front());  

	lock.unlock();

	//上传UOM数据
	if (postUomData(m_token, state))
	{
		//上传成功移除元素
		std::lock_guard<std::mutex> lock(mutex);
		queue.pop();
	}
	int aaa = queue.size();
	//Sleep(80);

	return true;
}


bool  UomDataUpload::isTokenValid()
{
	if (m_token.length() > 0)
	{
		return true;
	}
	return false;
	
}

 
bool UomDataUpload::requestToken(const std::string& appid, const std::string& appkey)
{
	printf("Begin************************************获取tokent.\n");

	const char* szTokenUrl = "https://ualins.uatair.com/betaAirspaceServer/hawksystemserver/client/loginByKey";

	//构造头
	std::map<std::string, std::string> heades;
	heades.insert(std::pair<std::string, std::string>("Content-Type", "application/json; charset=UTF-8"));

	//构造json内容
	char szJson[1024];
	_snprintf_s(szJson, 1024, "{\"appid\": \"%s\",  \"appkey\": \"%s\"}", appid.c_str(), appkey.c_str());

	MemoryData  memData;
	if (postRequest(szTokenUrl, szJson, heades, &memData))
	{

		printf("end------------------------%s.\n", memData.data.c_str());
		
		UomJsonPackage package;
		
		std::string strToken;
		if (package.ParseTokenStatus(memData.data, strToken))
		{
			m_token = strToken;

			

			return true;
		}
	}

	return false;
}

bool UomDataUpload::postUomData(const std::string& strToken, const UomUAVSate& state)
{
	printf("begin------------------------报送UOM数据.\n");

	const char* szPostUrl = "https://ualins.uatair.com/betaAirspaceServer/datareportserver/push-device-data2";

	//构造头
	std::map<std::string, std::string> heades;
	heades.insert(std::pair<std::string, std::string>("Content-Type", "application/json; charset=UTF-8"));
	heades.insert(std::pair<std::string, std::string>("token", strToken));

	//构造json内容
	UomJsonPackage package;  
	//std::string szJson = package.GenerateJsonString(state.wayPoints, state.uavPose, state.flightSate);
	std::string szJson = package.GenerateJsonString(state);
	//int index = szJson.find("groundAirType");
	//std::string aa =  szJson.substr(index, 20);
	//printf("-------%s\n", aa.data());
	MemoryData  memData;
	if (postRequest(szPostUrl, szJson.c_str(), heades, &memData))
	{
		printf("end------------------------%s.\n", memData.data.c_str());

		if (package.ParseUomStatus(memData.data))
		{
			return true;
		}

	}

	return true;
}


bool UomDataUpload::postRequest(const char *url, const char *data, const std::map<std::string, std::string>& heades, void* useData)
{
	int  ret = -1;

	//创建头
	struct curl_slist *listHeaders = NULL;
	for (const auto& pair : heades)
	{
		std::string strHeader = pair.first + ":" + pair.second;
		listHeaders = curl_slist_append(listHeaders, strHeader.c_str());
	}

	if (listHeaders == NULL)
	{
 
		goto label;
	}

	//----------------------------------------------------------------------

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		printf("curl_easy_init failed.");
		goto label;
	}
	 
	
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, listHeaders);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // 是否验证服务器SSL证书的有效性 0：不验证 1：验证
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); // 是否检查服务器的SSL证书中的主机名与请求的主机名匹配 0：不检查 2：检查
	// curl_easy_setopt(curl, CURLOPT_CAINFO, ""); // 指定CA证书的位置
	curl_easy_setopt(curl, CURLOPT_URL, url);          // url地址
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);  // post数据
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, useData);//传递给 write_callback() 回调函数的第四个参数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_recv_callback); // 接受数据回调
	curl_easy_setopt(curl, CURLOPT_POST, 1);                       // post请求
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);                    // 是否打印调试信息
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);                     // 是否输出响应头
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);                   // 设置整个cURL函数执行过程的最长等待时间，单位：秒

	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK)
	{
		printf("curl error code: %d\n", ret);
	}

label:
	if (listHeaders)
	{
		curl_slist_free_all(listHeaders);
	}

	if (curl)
	{
		curl_easy_cleanup(curl);
	}

	return (ret == CURLE_OK);
}

size_t UomDataUpload::curl_recv_callback(char *buffer, size_t size, size_t nitems, void *outstream)
{
	int nbytes = size * nitems;
	if (nbytes > 0)
	{
		MemoryData* pMemData = (MemoryData*)outstream;
		if (pMemData != NULL)
		{
			pMemData->data.append(buffer, size * nitems);
		}
		else
		{
			printf("memory error.\n");
		}
	}

	return size * nitems;
}*/