#pragma once

#include <fstream>
#include <vector>
#include <algorithm>

#include "logger.h"
#include "Utils.h"
#include "WindowDefinition.h"

#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"

#define L2_FILE_CONFIG_KEY "PathToL2exe"
#define L2_VERSION "L2Version"
#define MOUSE_CLICK_DELAY "MouseClickDelay"
#define MOUSE_EVENTS_DELAY "MouseEventsDelay"
#define VISION_INIT_TIMEOUT "VisionInitTimeoutMs"
#define INPUT_INITIAL_DELAY "InputInitialDelay"
#define INPUT_FALLBACK_DELAY "InputFallbackDelay"
#define ACCOUNT_IN_USE_DELAY "AccountInUseDelay"

// Features
#define LOGIN_PASSWORD_VALIDATION_ENABLED "LoginPasswordValidationEnabled"
#define FAST_FLOW_ENABLED "FastFlowEnabled"
#define CENTER_WINDOW_ENABLED "CenterWindowEnabled"
#define ACCOUNT_KEY "Account."
#define QUIT_ENABLED "QuitEnabled"
#define PVP_MODE_ENABLED "PvpModeEnabled"

// Debugging options
#define DEBUGGING_ENABLED "DebugEnabled"
#define SAVE_REF_SCREEN "SaveRefScreen"
#define SAVE_WC_FAILURES "SaveWcFailures"
#define LOGGER_ENABLED "LogEnabled"
#define ERROR_WINDOW_ENABLED "ErrorWindowEnabled"

// Recovery feature
#define CRASH_RECOVERY_ENABLED "CrashRecoveryEnabled"
#define CRASH_RECOVERY_MIN_DELAY "CrashRecoveryMinDelayMs"

// Layout management
#define LAYOUT_MANAGER "LayoutManagerEnabled"
#define LAYOUT_CACHE_ENABLED "LayoutManagerCacheEnabled"
 
enum L2Version { 
	NONE,
	C2,
	C3,
	C4,
	C5,
	IL,
	GRACIA_EPILOGUE,
	ESSENSE
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
	string pathToL2 = "";
	vector<L2AccountHotKey> accountHotKeys;
	int mouseClickDelay = -1;
	int mouseEventsDelay = 25;
	int visionInitTimeout = 10000;
	bool fastFlowEnabled = true;
	bool layoutManagerEnabled = true;
	bool quitEnabled = true;
	bool pvpModeEnabled = true;
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
	bool errorWindowEnabled = true;

	// Recovery
	bool crashRecoveryEnabled = true;
	int crashRecoveryMinDelayMs = 60000;

	// Autologin link
	bool loginPasswordValidatinEnabled = true;
};

using namespace std;

bool splitParam(string s, string& k, string& v);
bool toBoolean(string s);
string trim(string s);

L2AccountHotKey getAccountHotKey(string& hotKeyConfigKey, string hotKeyConfigValue);
L2Version toL2Version(string value);

L2AloneConfig loadL2AloneConfig() {

	L2AloneConfig config;

	ifstream configFile;

	configFile.open(L2_ALONE_CONFIG_FILE_NAME);
	if (!configFile.is_open()) {
		string s;
		s.append("Can't open config file ").append(L2_ALONE_CONFIG_FILE_NAME);

		throw std::exception(s.c_str());
	}

	std::vector<string> params;
	std::string line;
	while (std::getline(configFile, line)) {
		params.push_back(line);
	}
	configFile.close();

	for (string s : params) {

		if (trim(s).size() == 0) {
			continue;
		}

		string k, v;
		if (splitParam(s, k, v)) {
			if (startWith(k, "//")) {
				continue;
			}

			if (k == L2_FILE_CONFIG_KEY) {
				config.pathToL2 = v;
				cout << "L2 file set to: " << config.pathToL2 << endl;
			}
			else if (startWith(k, ACCOUNT_KEY)) {
				config.accountHotKeys.push_back(getAccountHotKey(k, v));
			}
			else if (k == L2_VERSION) {
				config.version = toL2Version(v);
				logger.log("L2 version: ", config.version);
			}
			else if (k == MOUSE_CLICK_DELAY) {
				config.mouseClickDelay = stoi(trim(v));
				logger.log("Mouse click delay: ", config.mouseClickDelay);
			}
			else if (k == MOUSE_EVENTS_DELAY) {
				config.mouseEventsDelay = stoi(trim(v));
				logger.log("Mouse events delay: ", config.mouseEventsDelay);
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
			else if (k == PVP_MODE_ENABLED) {
				config.pvpModeEnabled = stoi(trim(v));
				logger.log("Pvp mode enabled: ", config.pvpModeEnabled);
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
			else if (k == LOGIN_PASSWORD_VALIDATION_ENABLED) {
				config.loginPasswordValidatinEnabled = stoi(trim(v));
				logger.log("Login and password validation enabled: ", config.loginPasswordValidatinEnabled);
			}
			else if (k == ERROR_WINDOW_ENABLED) {
				config.errorWindowEnabled = stoi(trim(v));
				logger.log("Error window enabled: ", config.errorWindowEnabled);
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
	else if (value == "C3") {
		return L2Version::C3;
	}
	else if (value == "C4") {
		return L2Version::C4;
	}
	else if (value == "C5") {
		return L2Version::C5;
	}
	else if (value == "IL") {
		return L2Version::IL;
	}
	else if (value == "ESSENSE") {
		return L2Version::ESSENSE;
	}
	else if (value == "GRACIA_EPILOGUE") {
		return L2Version::GRACIA_EPILOGUE;
	}

	stringstream ss;
	ss << "Error in configuration. Invalid " << L2_VERSION << " '" << value << "'" << ". Allowed are 'C2', 'C3', 'C4', 'C5', 'IL', 'ESSENSE', 'GRACIA_EPILOGUE'";
	throw exception(ss.str().c_str());
}

bool toBoolean(string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	std::istringstream is(s);
	bool hasHorizontalBorder;
	is >> std::boolalpha >> hasHorizontalBorder;
	return hasHorizontalBorder;
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

bool splitParam(string s, string& k, string& v) {
	auto index = s.find("=");
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

	if (v == L2Version::C3) {
		return "C3";
	}

	if (v == L2Version::C4) {
		return "C4";
	}

	if (v == L2Version::C5) {
		return "C5";
	}

	if (v == L2Version::IL) {
		return "IL";
	}

	if (v == L2Version::ESSENSE) {
		return "ESSENSE";
	}

	throw exception("Unexpected L2 version");
}
