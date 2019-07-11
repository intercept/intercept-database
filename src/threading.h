#pragma once
#include <intercept.hpp>
#include "../intercept/src/host/common/singleton.hpp"
#include <future>
#include <queue>
#include <mariadb++/connection.hpp>
#include "res.h"
using namespace intercept::types;



class Worker;
class Task {
public:
    std::function<bool(mariadb::connection_ref)> job;
    std::promise<bool> prom;
    std::shared_ptr<Worker> worker; //Need to keep connection alive while Tasks exists.
    bool isInWorkList;
};

class Worker : public std::enable_shared_from_this<Worker> {

public:
    Worker();
    ~Worker();
    void run();


    std::future<bool> pushTask(std::function<bool(mariadb::connection_ref)> task, bool intoWorkList = false);


    std::queue<std::shared_ptr<Task>> tasks;
    std::mutex taskLock;
    bool exiting = false;
    std::weak_ptr<mariadb::connection> parentConnection;
    mariadb::connection_ref workerConnection;
    std::atomic_ullong lastJob;
    std::condition_variable hasTasksCondition;
    std::shared_ptr<std::thread> myThread;
};




class Threading : public intercept::singleton<Threading> {

public:

    Threading() : lastCleanup(std::chrono::system_clock::now()) {}
    std::future<bool> pushTask(mariadb::connection_ref con, std::function<bool(mariadb::connection_ref)> task, bool intoWorkList = false);
    void doCleanup();
    bool isConnected(mariadb::account_ref acc);

    std::map<mariadb::account_ref, std::vector<std::shared_ptr<Worker>>> workers;
    std::chrono::system_clock::time_point lastCleanup;


    std::vector<ref<GameDataDBAsyncResult>> asyncWork;
    std::vector<ref<GameDataDBAsyncResult>> completedAsyncTasks;
    std::mutex asyncWorkMutex;
    std::atomic_bool hasCompletedAsyncWork = false;
    void pushAsyncWork(ref<GameDataDBAsyncResult> work);
    void updateAsyncWorkLists();
};