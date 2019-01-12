#include "threading.h"
using namespace std::chrono_literals;

#include <iomanip> // put_time
#include <windows.h>
void logMessageWithTime(std::string msg) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::size_t fractional_seconds = ms.count() % 1000;



    std::stringstream ss;
    ss << msg << " " << std::put_time(std::localtime(&in_time_t), "%H:%M:%S.") << fractional_seconds << "\n";

    OutputDebugStringA(ss.str().c_str());
}



void Worker::run() {
    while (true) {
        std::unique_lock l(taskLock);
        auto parentCon = parentConnection.lock();

        if (!parentCon) {//If parent got deleted (went out of scope in SQF land) we want to exit too
            exiting = true; //#TODO going out of scope doesn't work
            while (!tasks.empty()) {
                std::shared_ptr<Task> task = tasks.front();
                tasks.pop();
                task->prom.set_value(false);
            }
            return; 
        }

        if (tasks.empty()) {
            hasTasksCondition.wait(l);
        }

        if (!tasks.empty()) {
            std::shared_ptr<Task> task = tasks.front();
            tasks.pop();
            lastJob = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            l.unlock();
            logMessageWithTime("do task");
            task->prom.set_value(task->job(workerConnection));

            if (task->isInWorkList) {
                Threading::get().updateAsyncWorkLists();
            }


            logMessageWithTime("task done");
        }
    }

}

std::future<bool> Worker::pushTask(std::function<bool(mariadb::connection_ref)> task, bool intoWorkList) {
    logMessageWithTime("Worker::pushTask");
    auto newTask = std::make_shared<Task>();
    newTask->worker = shared_from_this();
    newTask->job = std::move(task);
    newTask->isInWorkList = intoWorkList;
    auto fut = newTask->prom.get_future();


    std::unique_lock l(taskLock);
    if (exiting) {
        newTask->prom.set_value(false);
        return fut;
    }
    tasks.emplace(newTask);
    l.unlock();

    hasTasksCondition.notify_all();
    logMessageWithTime("Worker::pushTask ret");

    return fut;
}

std::future<bool> Threading::pushTask(mariadb::connection_ref con, std::function<bool(mariadb::connection_ref)> task, bool intoWorkList) {
    logMessageWithTime("Threading::pushTask");
    auto found = workers.find(con->account());
    if (found != workers.end()) {
        //Already have a worker on that account.
        return found->second->pushTask(std::move(task), intoWorkList);
    }
    logMessageWithTime("create worker");
    auto newWorker = std::make_shared<Worker>();
    newWorker->parentConnection = con;
    newWorker->workerConnection = mariadb::connection::create(con->account());
    newWorker->myThread = std::make_shared<std::thread>([newWorker]() {newWorker->run(); });
    workers[con->account()] = newWorker;

    return newWorker->pushTask(std::move(task), intoWorkList);
}


void Threading::doCleanup() {
    if (lastCleanup + 30s > std::chrono::system_clock::now()) return;
    logMessageWithTime("Threading::doCleanup");


    //Will only be called from mainthread so noone can insert stuff now.
    for (auto& [acc,worker] : workers) {
        std::unique_lock l(worker->taskLock);
        if (!worker->tasks.empty()) continue; //Is still working on tasks
        logMessageWithTime("worker cleanup check");
        auto lastTask = std::chrono::system_clock::from_time_t(worker->lastJob);
        if (lastTask + 60s < std::chrono::system_clock::now()) { //no tasks for 60s, kill worker
            logMessageWithTime("worker do cleanup");
            worker->parentConnection.reset(); //no parent will mean it exits next iteration
            worker->hasTasksCondition.notify_all(); //force iteration
            if (worker->myThread->joinable()) worker->myThread->join(); //wait for thread to exit
            logMessageWithTime("worker cleaned up");
        }
        workers.erase(acc);
        return;
    }
    lastCleanup = std::chrono::system_clock::now();
}

void Threading::pushAsyncWork(ref<GameDataDBAsyncResult> work) {
    std::unique_lock l(asyncWorkMutex);
    asyncWork.emplace_back(work);

}

void Threading::updateAsyncWorkLists() {
    std::unique_lock l(asyncWorkMutex);



    auto p = std::stable_partition(asyncWork.begin(), asyncWork.end(),
        [&](const auto& x) { 
            auto res = x->data->fut.wait_for(std::chrono::nanoseconds(0));
            
            return res != std::future_status::ready; });
    // range insert with move
    completedAsyncTasks.insert(completedAsyncTasks.end(), std::make_move_iterator(p),
        std::make_move_iterator(asyncWork.end()));
    // erase the moved-from elements.
    asyncWork.erase(p, asyncWork.end());
    //#TODO we can just delete completed tasks that don't have callback or callbackargs
    //better do it here than in mainthread
    hasCompletedAsyncWork = !completedAsyncTasks.empty();

}
