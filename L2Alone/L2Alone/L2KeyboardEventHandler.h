#pragma once

class L2KeyboardEventHandler {

public:

	virtual bool onKeyDown(KBDLLHOOKSTRUCT* kbdll) {
		return true;
	}

	virtual bool onKeyUp(KBDLLHOOKSTRUCT* kbdll) {
		return true;
	}
};