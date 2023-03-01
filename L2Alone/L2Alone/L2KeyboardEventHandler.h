#pragma once

class L2KeyboardEventHandler {

public:
	virtual void onKeyDown(KBDLLHOOKSTRUCT* kbdll) = 0;

	virtual void onKeyUp(KBDLLHOOKSTRUCT* kbdll) = 0;
};