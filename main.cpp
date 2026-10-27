#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

std::mutex mtx;
int clientCount = 0;
const int MAX_CLIENTS = 10;
bool isFinished = false;       // сигнал для завершения операциониста
std::condition_variable cv;    // чтобы операционист не крутился вхолостую

// Поток 1: Клиенты
void clientThread() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            std::lock_guard<std::mutex> lock(mtx);
            ++clientCount;
            std::cout << "Клиент пришёл. В очереди: " << clientCount << std::endl;
        }
        cv.notify_one();  // разбудить операциониста
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        isFinished = true;
    }
    cv.notify_all();
}

// Поток 2: Операционист
void operatorThread() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return clientCount > 0 || isFinished; });

            if (clientCount == 0 && isFinished) {
                std::cout << "Очередь пуста. Операционист закончил работу." << std::endl;
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (clientCount > 0) {
                --clientCount;
                std::cout << "Операционист обслужил клиента. В очереди: " << clientCount << std::endl;
            }
        }
    }
}

int main() {
    std::cout << "Максимальное количество клиентов: " << MAX_CLIENTS << std::endl;

    std::thread t1(clientThread);
    std::thread t2(operatorThread);

    t1.join();
    t2.join();

    std::cout << "Programma Zavershina." << std::endl;
    return 0;
}