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
#define LOGS_ENABLED_CONFIG_KEY "LogsEnabled"
#define CAPTURE_LOGS_ENABLED_CONFIG_KEY "CaptureLogsEnabled"
#define ACCOUNT_KEY "Account"

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

struct L2AloneConfig {
	L2Version version = L2Version::NONE;
	string pathToL2 = "";
	bool logsEnabled = false;
	bool captureLogsEnabled = false;
	vector<L2AccountHotKey> accountHotKeys;
};

using namespace std;

bool splitParam(string s, string& k, string& v);
bool toBoolean(string s);

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
		string k, v;
		if (splitParam(s, k, v)) {
			if (startWith(k, "//")) {
				continue;
			}

			logger.log("Parameter received '", k, "' '", v, "'");

			if (k == L2_FILE_CONFIG_KEY) {
				config.pathToL2 = v;
				cout << "L2 file set to: " << config.pathToL2 << endl;
			}
			else if (k == LOGS_ENABLED_CONFIG_KEY) {
				config.logsEnabled = toBoolean(v);
				cout << "Core logs enabled: " << config.logsEnabled << endl;
			}
			else if (k == CAPTURE_LOGS_ENABLED_CONFIG_KEY) {
				config.captureLogsEnabled = toBoolean(v);
				cout << "Capture logs enabled: " << config.captureLogsEnabled << endl;
			}
			else if (startWith(k, ACCOUNT_KEY)) {
				config.accountHotKeys.push_back(getAccountHotKey(k, v));
			}
			else if (k == L2_VERSION) {
				config.version = toL2Version(v);
			}
		}
	}

	if (config.version == L2Version::NONE) {
		stringstream ss;
		ss << "Specify '" << L2_VERSION << "' in " << L2_ALONE_CONFIG_FILE_NAME << ", e.g '" << L2_VERSION << "=C5'";
		throw exception(ss.str().c_str());
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

string trim(string s) {

	int trimStart = 0;
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] != ' ') {
			trimStart = i;
			break;
		}
	}

	int trimEnd = 0;
	for (int i = s.size() - 1; i >= 0; --i) {
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

		int secondIndex = s.find(",", index + 1);
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
