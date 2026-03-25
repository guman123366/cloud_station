 

#include "UomDataDefine.h"
#include "AsyncTask.h"

#include <map>
#include "UomJsonPackage.h"
//#include"../ThridParty/include/curl/curl.h"

class  UomDataUpload : public AsyncTask
{
public:
	UomDataUpload();
	virtual ~UomDataUpload();

/*public: 
	void uploadData(const UomUAVSate& state);

private:
	bool isTokenValid();
	bool  requestToken(const std::string& appid, const std::string& appkey);
	bool  postUomData(const std::string& token, const UomUAVSate& state);
	bool  postRequest(const char *data, const char *url, const std::map<std::string, std::string>& heades, void* useData);
	
	static size_t curl_recv_callback(char *buffer, size_t size, size_t nitems, void *outstream);

protected:
	bool execute();

private:
	std::queue<UomUAVSate>     queue;
	std::mutex                 mutex;

	int                        nFailed;

	std::string                m_appid;
	std::string                m_appkey;
	std::string                m_token;*/
};


 
 