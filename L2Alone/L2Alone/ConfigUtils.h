#pragma once

#include <fstream>
#include <vector>
#include <algorithm>

#include "logger.h"
#include "Utils.h"
#include "WindowDefinition.h"

#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"

#define L2_FILE_CONFIG_KEY L"PathToL2exe"
#define L2_VERSION L"L2Version"
#define MOUSE_CLICK_DELAY L"MouseClickDelay"
#define MOUSE_EVENTS_DELAY L"MouseEventsDelay"
#define VISION_INIT_TIMEOUT L"VisionInitTimeoutMs"
#define INPUT_INITIAL_DELAY L"InputInitialDelay"
#define INPUT_FALLBACK_DELAY L"InputFallbackDelay"
#define ACCOUNT_IN_USE_DELAY L"AccountInUseDelay"

// Features
#define FAST_FLOW_ENABLED L"FastFlowEnabled"
#define CENTER_WINDOW_ENABLED L"CenterWindowEnabled"
#define ACCOUNT_KEY L"Account."
#define QUIT_ENABLED L"QuitEnabled"

// Debugging options
#define DEBUGGING_ENABLED L"DebugEnabled"
#define SAVE_REF_SCREEN L"SaveRefScreen"
#define SAVE_WC_FAILURES L"SaveWcFailures"
#define LOGGER_ENABLED L"LogEnabled"

// Recovery feature
#define CRASH_RECOVERY_ENABLED L"CrashRecoveryEnabled"
#define CRASH_RECOVERY_MIN_DELAY L"CrashRecoveryMinDelayMs"

// Layout management
#define LAYOUT_MANAGER L"LayoutManagerEnabled"
#define LAYOUT_CACHE_ENABLED L"LayoutManagerCacheEnabled"
 
enum L2Version {
	NONE,
	C2,
	C5,
};

struct L2AccountHotKey {
	int fKey;
	string login;
	string password;
	L2CharSlot slot;
};

struct WindowLayoutConfig {
	int monitor;
	bool fullScreen;
	int x;
	int y;
	int width;
	int height;
};

struct LayoutConfig {
	vector<WindowLayoutConfig> windowConfigs;
};

struct L2AloneConfig {
	L2Version version = L2Version::NONE;
	wstring pathToL2 = L"";
	vector<L2AccountHotKey> accountHotKeys;
	int mouseClickDelay = -1;
	int mouseEventsDelay = 0;
	int visionInitTimeout = 10000;
	bool fastFlowEnabled = true;
	bool layoutManagerEnabled = true;
	bool quitEnabled = true;
	bool centerWindow = true;
	bool layoutCacheEnabled = true;

	// Input
	int inputInitialDelay = 100;
	int inputFallbackDelay = 1000;
	int windowTransitionRetryCount = 3;
	int accountInUseDelay = 500;

	// Debugging
	bool debugSaveRefScreen = false;
	bool debugSaveWcFailures = false;
	bool debugLogEnabled = false;
	bool debugEnabled = false;
	string debugBmpPath = "";

	// Recovery
	bool crashRecoveryEnabled = true;
	int crashRecoveryMinDelayMs = 60000;
};

using namespace std;

bool splitParam(wstring s, wstring& k, wstring& v);
bool toBoolean(string s);
wstring trim(wstring s);
string trim(string s);

L2AccountHotKey getAccountHotKey(string& hotKeyConfigKey, string hotKeyConfigValue);
L2Version toL2Version(string value);

L2AloneConfig loadL2AloneConfig() {

	L2AloneConfig config;

	wifstream configFile;

	configFile.open(L2_ALONE_CONFIG_FILE_NAME);
	if (!configFile.is_open()) {
		string s;
		s.append("Can't open config file ").append(L2_ALONE_CONFIG_FILE_NAME);

		throw std::exception(s.c_str());
	}

	std::vector<wstring> params;
	std::wstring line;
	while (std::getline(configFile, line)) {
		params.push_back(line);
	}
	configFile.close();

	for (wstring s : params) {

		if (trim(s).size() == 0) {
			continue;
		}

		wstring k, v;
		if (splitParam(s, k, v)) {
			if (startWith(k, L"//")) {
				continue;
			}

			if (k == L2_FILE_CONFIG_KEY) {
				config.pathToL2 = v;
				logger.log("L2 file set to: ", config.pathToL2);
			}
			else if (startWith(k, ACCOUNT_KEY)) {
				string accountHk(k.begin(), k.end());
				string accountHv(v.begin(), v.end());
				config.accountHotKeys.push_back(getAccountHotKey(accountHk, accountHv));
			}
			else if (k == L2_VERSION) {
				string l2VersionV(v.begin(), v.end());
				config.version = toL2Version(l2VersionV);
				logger.log("L2 version: ", config.version);
			}
			else if (k == MOUSE_CLICK_DELAY) {
				config.mouseClickDelay = stoi(trim(v));
				logger.log("Mouse input speed: ", config.mouseClickDelay);
			}
			else if (k == MOUSE_EVENTS_DELAY) {
				config.mouseEventsDelay = stoi(trim(v));
				logger.log("Mouse input delay: ", config.mouseEventsDelay);
			}
			else if (k == VISION_INIT_TIMEOUT) {
				config.visionInitTimeout = stoi(trim(v));
				logger.log("Vision init timeout: ", config.visionInitTimeout);
			}
			else if (k == SAVE_REF_SCREEN) {
				config.debugSaveRefScreen = (bool)stoi(trim(v));
				logger.log("Save ref screen: ", config.debugSaveRefScreen);
			}
			else if (k == SAVE_WC_FAILURES) {
				config.debugSaveWcFailures = (bool)stoi(trim(v));
				logger.log("Save WC failures: ", config.debugSaveWcFailures);
			}
			else if (k == LOGGER_ENABLED) {
				config.debugLogEnabled = (bool)stoi(trim(v));
				logger.log("Logger enabled: ", config.debugLogEnabled);
			}
			else if (k == FAST_FLOW_ENABLED) {
				config.fastFlowEnabled = (bool)stoi(trim(v));
				logger.log("Fast flow enabled: ", config.fastFlowEnabled);
			}
			else if (k == INPUT_FALLBACK_DELAY) {
				config.inputFallbackDelay = stoi(trim(v));
				logger.log("Input fallback delay: ", config.inputFallbackDelay);
			}
			else if(k == INPUT_INITIAL_DELAY) {
				config.inputInitialDelay = stoi(trim(v));
				logger.log("Input initial delay: ", config.inputInitialDelay);
			}
			else if (k == DEBUGGING_ENABLED) {
				config.debugEnabled = stoi(trim(v));
				logger.log("Debug enabled: ", config.debugEnabled);
			}
			else if (k == ACCOUNT_IN_USE_DELAY) {
				config.accountInUseDelay = stoi(trim(v));
				logger.log("Account in use delay: ", config.accountInUseDelay);
			}
			else if (k == LAYOUT_MANAGER) {
				config.layoutManagerEnabled = stoi(trim(v));
				logger.log("Layout manager: ", config.layoutManagerEnabled);
			}
			else if (k == QUIT_ENABLED) {
				config.quitEnabled = stoi(trim(v));
				logger.log("Quit enabled: ", config.quitEnabled);
			}
			else if (k == CENTER_WINDOW_ENABLED) {
				config.centerWindow = stoi(trim(v));
				logger.log("Center window: ", config.centerWindow);
			}
			else if (k == LAYOUT_CACHE_ENABLED) {
				config.layoutCacheEnabled = stoi(trim(v));
				logger.log("Layout cache enabled: ", config.layoutCacheEnabled);
			}
			else if (k == CRASH_RECOVERY_ENABLED) {
				config.crashRecoveryEnabled = stoi(trim(v));
				logger.log("Crash recovery: ", config.crashRecoveryEnabled);
			}
			else if (k == CRASH_RECOVERY_MIN_DELAY) {
				config.crashRecoveryMinDelayMs = stoi(trim(v));
				logger.log("Crash recovery min delay ms: ", config.crashRecoveryMinDelayMs);
			}
		}
	}

	if (config.version == L2Version::NONE) {
		stringstream ss;
		ss << "Specify '" << L2_VERSION << "' in " << L2_ALONE_CONFIG_FILE_NAME << ", e.g '" << L2_VERSION << "=C5'";
		throw exception(ss.str().c_str());
	}

	if (!config.debugEnabled) {
		config.debugLogEnabled = false;
		config.debugSaveRefScreen = false;
		config.debugSaveWcFailures = false;
	}

	return config;
}

L2Version toL2Version(string value) {
	if (value == "C2") {
		return L2Version::C2;
	}
	else if (value == "C5") {
		return L2Version::C5;
	}

	stringstream ss;
	ss << "Error in configuration. Invalid " << L2_VERSION << " '" << value << "'" << ". Expected 'C2' or 'C5' ";
	throw exception(ss.str().c_str());
}

bool toBoolean(string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	std::istringstream is(s);
	bool hasHorizontalBorder;
	is >> std::boolalpha >> hasHorizontalBorder;
	return hasHorizontalBorder;
}

wstring trim(wstring s) {

	if (s.size() == 0) {
		return s;
	}

	size_t trimStart = 0;
	for (auto i = 0; i < s.size(); ++i) {
		if (s[i] != ' ') {
			trimStart = i;
			break;
		}
	}

	size_t trimEnd = 0;
	for (auto i = s.size() - 1; i >= 0; --i) {
		if (s[i] != ' ') {
			trimEnd = i;
			break;
		}
	}

	return s.substr(trimStart, trimEnd - trimStart + 1);
}

string trim(string s) {

	if (s.size() == 0) {
		return s;
	}

	size_t trimStart = 0;
	for (auto i = 0; i < s.size(); ++i) {
		if (s[i] != ' ') {
			trimStart = i;
			break;
		}
	}

	size_t trimEnd = 0;
	for (auto i = s.size() - 1; i >= 0; --i) {
		if (s[i] != ' ') {
			trimEnd = i;
			break;
		}
	}

	return s.substr(trimStart, trimEnd - trimStart + 1);
}

bool splitParam(wstring s, wstring& k, wstring& v) {
	auto index = s.find(L"=");
	if (index >= 0) {
		k = trim(s.substr(0, index));
		v = trim(s.substr(index + 1, s.size()));
		return true;
	}

	return false;
}

bool parseAccountFKey(string s, int &fKey) {
	auto dotIndex = s.find(".");
	if (dotIndex > 0) {
		string indexString = s.substr(dotIndex + 1);
		indexString = trim(indexString);
		fKey = stoi(indexString);
		return true;
	}

	return false;
}

void splitHotKeyAccountValue(string s, string& login, string& password, L2CharSlot &slot) {
	auto index = s.find(",");
	if (index > 0) {

		login = trim(s.substr(0, index));

		auto secondIndex = s.find(",", index + 1);
		if (secondIndex != -1) {
			password = trim(s.substr(index + 1, secondIndex - index - 1));
			auto slotString = trim(s.substr(secondIndex + 1));
			int slotIndex;
			try {
				slotIndex = stoi(slotString);
			}
			catch (exception e) {
				stringstream ss;
				ss << "Can't parse character slot '" << slotString << "'. It must be number in range [1,7]";
				throw exception(ss.str().c_str());
			}

			if (0 < slotIndex && slotIndex < 8) {
				slot = (L2CharSlot)slotIndex;
			}
			else {
				stringstream ss;
				ss << "Character slot must be in range [1,7], but received " << slotIndex;
				throw exception(ss.str().c_str());
			}
		}
		else {
			password = trim(s.substr(index + 1));
			slot = L2CharSlot::ACTIVE;
		}
	}
}

L2AccountHotKey getAccountHotKey(string& hotKeyConfigKey, string hotKeyConfigValue) {

	int fKey;
	if (!parseAccountFKey(hotKeyConfigKey, fKey)) {

		stringstream ss;
		ss << "Can't parse configuration '" << hotKeyConfigKey << "'. Format should be Account.index. E.g Account.1";
		throw exception(ss.str().c_str());
	}

	if (fKey < 1 && fKey > 12) {
		stringstream ss;
		ss << "Invalid configuration. Account key must be [1,12]. Found " << fKey;
		throw exception(ss.str().c_str());
	}

	string login, password;
	L2CharSlot slot;
	splitHotKeyAccountValue(hotKeyConfigValue, login, password, slot);

	return L2AccountHotKey{ fKey, login, password, slot };
}

string getL2VersionName(L2Version v) {
	if (v == L2Version::C2) {
		return "C2";
	}
	else if (v == L2Version::C5) {
		return "C5";
	}

	throw exception("Unexpected L2 version");
}
