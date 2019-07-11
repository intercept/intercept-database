#include "threading.h"
#include "ittnotify.h"
#include "../ittnotify/ittnotify.h"
#include "logger.h"
#include "config.h"
using namespace std::chrono_literals;

#include <iomanip> // put_time
#ifdef _MSVC_LANG
#include <windows.h>
#endif

#define LOG_Thread(x) if (Logger::get().isThreadLogEnabled()) Logger::get().logThread((r_string)x)

void logMessageWithTime(std::string msg) {
    return;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::size_t fractional_seconds = ms.count() % 1000;



    std::stringstream ss;
    ss << msg << " " << std::put_time(std::localtime(&in_time_t), "%H:%M:%S.") << fractional_seconds << "\n";

    #ifdef _MSVC_LANG
    OutputDebugStringA(ss.str().c_str());
    #endif
}


__itt_domain* domain = __itt_domain_create("Threading");


__itt_string_handle* worker_run = __itt_string_handle_create("Worker::run");
__itt_string_handle* worker_runTask = __itt_string_handle_create("Worker::run::runTask");
__itt_string_handle* worker_processTaskTask = __itt_string_handle_create("Worker::run::processTask");
__itt_string_handle* worker_cleanup = __itt_string_handle_create("Worker::run::cleanup");

Worker::Worker() {
    LOG_Thread("Worker created "+std::to_string(reinterpret_cast<uintptr_t>(this)));
    __itt_sync_create(&taskLock, "std:::mutex", "Worker::taskLock", __itt_attr_mutex);
}

Worker::~Worker() {
    LOG_Thread("Worker destroying "+std::to_string(reinterpret_cast<uintptr_t>(this)));
    if (myThread) {
        std::unique_lock l(taskLock);
        parentConnection.reset();
        l.unlock();
        if (myThread->joinable()) myThread->join();
    }
    __itt_sync_destroy(&taskLock);
}

void Worker::run() {
    __itt_task_begin(domain, __itt_null, __itt_null, worker_run);
    bool rememberUpdateWorklist = false;

    __itt_counter counter = __itt_counter_create("tasksToDo", "worker");


    while (true) {
        __itt_sync_prepare(&taskLock);
        std::unique_lock l(taskLock);
        __itt_sync_acquired(&taskLock);
        auto parentCon = parentConnection.lock();

        if (!parentCon) {//If parent got deleted (went out of scope in SQF land) we want to exit too
            LOG_Thread("Worker::lostparent "+std::to_string(reinterpret_cast<uintptr_t>(this)));
            __itt_task_begin(domain, __itt_null, __itt_null, worker_cleanup);
            exiting = true; //#TODO going out of scope doesn't work
            while (!tasks.empty()) {
                std::shared_ptr<Task> task = tasks.front();
                tasks.pop();
                task->prom.set_value(false);
            }
            __itt_task_end(domain);
            __itt_task_end(domain); //end worker_run task
            __itt_sync_releasing(&taskLock);
            __itt_counter_destroy(counter);
            return;
        }

        if (tasks.empty()) {
            __itt_sync_releasing(&taskLock);
            hasTasksCondition.wait(l);
            __itt_sync_acquired(&taskLock);
        }
        __itt_task_begin(domain, __itt_null, __itt_null, worker_processTaskTask);
        uint64_t tasksLeft = tasks.size();
        __itt_counter_set_value(counter, &tasksLeft);
        if (tasksLeft) {
            std::shared_ptr<Task> task = tasks.front();
            tasks.pop();
            tasksLeft--;
            lastJob = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            __itt_sync_releasing(&taskLock);
            l.unlock();
            LOG_Thread("Worker::dotask W"+std::to_string(reinterpret_cast<uintptr_t>(this)) + " T"+std::to_string(reinterpret_cast<uintptr_t>(task.get())));
            __itt_task_begin(domain, __itt_null, __itt_null, worker_runTask);
            task->prom.set_value(task->job(workerConnection));
            __itt_task_end(domain);
            if (task->isInWorkList || rememberUpdateWorklist) {
                if (tasksLeft % 64 == 0) { //Don't trust the compiler telling you nonsense
                    Threading::get().updateAsyncWorkLists();
                    rememberUpdateWorklist = false;
                } else
                    rememberUpdateWorklist = true;
            }

            LOG_Thread("Worker::taskdone W"+std::to_string(reinterpret_cast<uintptr_t>(this)) + " T"+std::to_string(reinterpret_cast<uintptr_t>(task.get())));
        }

        if (rememberUpdateWorklist) {
            if (tasksLeft % 64 == 0) {
                Threading::get().updateAsyncWorkLists();
                rememberUpdateWorklist = false;
            }
                
        }


        __itt_task_end(domain);
        __itt_sync_releasing(&taskLock);
    }

}


__itt_string_handle* worker_pushTask = __itt_string_handle_create("Worker::pushTask");
std::future<bool> Worker::pushTask(std::function<bool(mariadb::connection_ref)> task, bool intoWorkList) {
    logMessageWithTime("Worker::pushTask");
    __itt_task_begin(domain, __itt_null, __itt_null, worker_pushTask);
    auto newTask = std::make_shared<Task>();
    newTask->worker = shared_from_this();
    newTask->job = std::move(task);
    newTask->isInWorkList = intoWorkList;
    auto fut = newTask->prom.get_future();

    LOG_Thread("Worker::pushTask W"+std::to_string(reinterpret_cast<uintptr_t>(this)) + " T"+std::to_string(reinterpret_cast<uintptr_t>(newTask.get())));

    __itt_sync_prepare(&taskLock);
    std::unique_lock l(taskLock);
    __itt_sync_acquired(&taskLock);
    if (exiting) {
        newTask->prom.set_value(false);
        return fut;
    }
    tasks.emplace(newTask);
    __itt_sync_releasing(&taskLock);
    l.unlock();

    hasTasksCondition.notify_all();
    logMessageWithTime("Worker::pushTask ret");
    __itt_task_end(domain);
    return fut;
}


__itt_string_handle* threading_pushTask = __itt_string_handle_create("Threading::pushTask");
__itt_string_handle* threading_pushTask_spinup = __itt_string_handle_create("Threading::pushTask::spinup");
std::future<bool> Threading::pushTask(mariadb::connection_ref con, std::function<bool(mariadb::connection_ref)> task, bool intoWorkList) {
    logMessageWithTime("Threading::pushTask");
    __itt_task_begin(domain, __itt_null, __itt_null, threading_pushTask);
    auto found = workers.find(con->account());
    if (found != workers.end()) {

        auto activeWorkerCount = std::count_if(found->second.begin(), found->second.end(), [](const std::shared_ptr<Worker>& w) {
            return !w->exiting;
        });

        if (activeWorkerCount >= Config::get().getWorkerCount()) {
            const auto it = std::min_element(found->second.begin(), found->second.end(), [](const std::shared_ptr<Worker>& l, const std::shared_ptr<Worker>& r) {
                if (l->exiting) return false;
                if (r->exiting) return true;
                return l->tasks.size() < r->tasks.size();
            });

            if (!(*it)->exiting) {
                //Already have a worker on that account.
                auto ret = (*it)->pushTask(std::move(task), intoWorkList);
                __itt_task_end(domain);
                return ret;
            }
        }
    }


    logMessageWithTime("create worker");
    __itt_task_begin(domain, __itt_null, __itt_null, threading_pushTask_spinup);
    auto newWorker = std::make_shared<Worker>();
    newWorker->parentConnection = con;
    newWorker->workerConnection = mariadb::connection::create(con->account());
    newWorker->myThread = std::make_shared<std::thread>([newWorker]() {newWorker->run(); });
    workers[con->account()].emplace_back(newWorker);
    __itt_task_end(domain);

    auto ret = newWorker->pushTask(std::move(task), intoWorkList);
    __itt_task_end(domain);
    return ret;
}

__itt_string_handle* threading_doCleanup = __itt_string_handle_create("Threading::doCleanup");
__itt_string_handle* threading_doCleanup_workerDoCleanup = __itt_string_handle_create("Threading::doCleanup::workerDoCleanup");
void Threading::doCleanup() {
    if (lastCleanup + 30s > std::chrono::system_clock::now()) return;
    logMessageWithTime("Threading::doCleanup");
    __itt_task_begin(domain, __itt_null, __itt_null, threading_doCleanup);

    //Will only be called from mainthread so noone can insert stuff now.
    for (auto& [acc,workerlist] : workers) {

        for (auto& worker : workerlist) {
            __itt_sync_prepare(&worker->taskLock);
            std::unique_lock l(worker->taskLock);
            __itt_sync_acquired(&worker->taskLock);
            if (!worker->tasks.empty()) continue; //Is still working on tasks
            logMessageWithTime("worker cleanup check");
            auto lastTask = std::chrono::system_clock::from_time_t(worker->lastJob);
            LOG_Thread("Worker::cleanupCheck W"+std::to_string(reinterpret_cast<uintptr_t>(worker.get())) + " LastTask "+ 
            std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastTask).count())+"s"
            );
            if (lastTask + 60s < std::chrono::system_clock::now()) { //no tasks for 60s, kill worker
                LOG_Thread("Worker::doCleanup W"+std::to_string(reinterpret_cast<uintptr_t>(worker.get())));
                //logMessageWithTime("worker do cleanup");
                __itt_task_begin(domain, __itt_null, __itt_null, threading_doCleanup_workerDoCleanup);
                worker->parentConnection.reset(); //no parent will mean it exits next iteration
                __itt_sync_releasing(&worker->taskLock);
                l.unlock();
                worker->hasTasksCondition.notify_all(); //force iteration
                if (worker->myThread->joinable()) worker->myThread->join(); //wait for thread to exit
                workerlist.erase(std::remove(workerlist.begin(), workerlist.end(), worker), workerlist.end());
                //workers.erase(acc);
                __itt_task_end(domain);
                LOG_Thread("Worker::cleanupDone W"+std::to_string(reinterpret_cast<uintptr_t>(worker.get())));

                __itt_task_end(domain);
                return;
            }
        }
        if (workerlist.empty())
            workers.erase(acc);
    }
    lastCleanup = std::chrono::system_clock::now();
    __itt_task_end(domain);
}

bool Threading::isConnected(mariadb::account_ref acc) {
    auto found = workers.find(acc);
    if (found == workers.end()) return false;
    //#TODO potentially not threadsafe

    for (auto& it : found->second) {
       if (it->workerConnection->connected()) return true;
    }

    return false;
}

void Threading::pushAsyncWork(ref<GameDataDBAsyncResult> work) {
    std::unique_lock l(asyncWorkMutex);
    asyncWork.emplace_back(work);
}
__itt_string_handle* threading_updateAsyncWorkLists = __itt_string_handle_create("Threading::updateAsyncWorkLists");

__itt_counter counter = __itt_counter_create("completedTasks", "threading");

void Threading::updateAsyncWorkLists() {
    __itt_task_begin(domain, __itt_null, __itt_null, threading_updateAsyncWorkLists);
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
    uint64_t sz = completedAsyncTasks.size();

    LOG_Thread("Threading::updatedAsyncWorklist waitingResults "+std::to_string(sz));
    __itt_counter_set_value(counter, &sz);

    __itt_task_end(domain);
    if (asyncWork.empty())
        __itt_pause();
}
