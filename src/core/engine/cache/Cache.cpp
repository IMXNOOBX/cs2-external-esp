#include "Cache.hpp"


bool Cache::Init() {
    return GetInstance().InitImpl();
}

bool Cache::InitImpl() {


    LOGF(INFO, "Succesfully initialized engine...");
    return true;
}