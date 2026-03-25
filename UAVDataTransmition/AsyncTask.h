

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

class AsyncTask
{
public:
    AsyncTask()
		:terminated(false), isRunning(false)
    {
         
    }

    virtual ~AsyncTask()
    {
       
    }

public:
    bool start() 
    {
        thread = std::thread(&AsyncTask::loop, this);

        return true;
    }

	void pause()
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			isRunning = false;
		}
	}

	void resume()
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			isRunning = true;
		}

		cv.notify_one();
	}


	void stop()
    {
		{
			std::lock_guard<std::mutex> lock(mutex);
			terminated = true;
		}

		cv.notify_one();

        thread.join();
    }

  


protected:
	virtual bool execute() = 0;

private:
    void loop() 
    {
		 while (true)
		 {
			 std::unique_lock<std::mutex> lock(mutex);

			 cv.wait(lock, [this] {
				 return isRunning || terminated; }
			 );

			 if (terminated)
			 {
				 break;
			 }

			 lock.unlock();

			 if (!execute())
			 {
				 break;
			 }
           
		 }  
    }

private:
    std::thread                thread;

	std::mutex                 mutex;
	std::condition_variable    cv;
	bool                       terminated;
	bool                       isRunning;
};


 
 