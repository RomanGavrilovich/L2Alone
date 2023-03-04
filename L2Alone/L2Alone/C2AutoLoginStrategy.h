#pragma once

#include <sstream>
#include <future>
#include <condition_variable>
#include <shellapi.h>
#include <WinUser.h>
#include <dbt.h>
#include <psapi.h>

#include "AutologinStrategy.h"
#include "logger.h"

using namespace std;

class C2AutologinStrategy : public AutologinStrategy {

public:
	void doAutologin(HWND hWindow, string& login, string& password) override;
};

void C2AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password) {
	throw exception("Autologin for C2 is not implemented");
}