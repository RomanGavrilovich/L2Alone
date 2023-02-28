#pragma once

#include <fstream>
#include <vector>

#include "logger.h"
#include "files_utils.h"

#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"

#define L2_FILE_CONFIG_KEY "PathToL2exe"
#define L2_WINDOW_NAME_KEY "L2WindowName"
#define LOGS_ENABLED_CONFIG_KEY "LogsEnabled"
#define CAPTURE_LOGS_ENABLED_CONFIG_KEY "CaptureLogsEnabled"

struct L2AloneConfig {
	string pathToL2;
	string l2WindowName = "Lineage";
	bool logsEnabled = false;
	bool captureLogsEnabled = false;
};

using namespace std;

bool splitParam(string s, string& k, string& v);
bool toBoolean(string s);

L2AloneConfig loadL2AloneConfig() {

	L2AloneConfig config;

	ifstream configFile;

	auto configFileAbsPath = getAbsoluteFilePath(L2_ALONE_CONFIG_FILE_NAME);

	configFile.open(configFileAbsPath);
	if (!configFile.is_open()) {
		string s;
		s.append("Can't open config file ").append(configFileAbsPath);

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
			else if (k == L2_WINDOW_NAME_KEY) {
				config.l2WindowName = v;
				cout << "L2 Window name: '" << config.l2WindowName << "'" << endl;
			}
		}
	}

	return config;
}

bool toBoolean(string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	std::istringstream is(s);
	bool b;
	is >> std::boolalpha >> b;
	return b;
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