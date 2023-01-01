#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

condition_variable cv;
mutex m;

bool readyToAccept = true;
long long int event_content = 1;

void provide()
{
    while (1)
    {
        unique_lock<mutex> lk(m);
        if (!readyToAccept)
        {
            lk.unlock();
            continue;
        }

        event_content *= 2;

        cout << "[Provider] Initialized: " << event_content << endl;

        readyToAccept = false;

        cv.notify_all();
        lk.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void consume()
{
    while (1)
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []
                { return !readyToAccept; });

        cout << "[Consumer] Processing with: " << event_content << endl;

        readyToAccept = true;

        cv.notify_all();
        lk.unlock();
    }
}

int main()
{
    thread t(provide);

    consume();
}