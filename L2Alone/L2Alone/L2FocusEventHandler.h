#pragma once


class L2FocusEventHandler {

public:
	virtual void onFocusLost() = 0;

	virtual void onFocusReceived() = 0;
};