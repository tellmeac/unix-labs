#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

condition_variable cv;
mutex m;

bool is_provided = false;
long long int event_content = 1;

void provide()
{
    while (1)
    {
        unique_lock<mutex> lk(m);
        event_content *= 2;

        cout << "[Provider] Initialized: " << event_content << endl;

        is_provided = true;

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
                { return is_provided; });

        cout << "[Consumer] Processing with: " << event_content << endl;

        is_provided = false;

        lk.unlock();
    }
}

int main()
{
    thread t(provide);

    consume();
}