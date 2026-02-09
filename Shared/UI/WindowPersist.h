#pragma once
#include <string>

// from https://github.com/Kosmokleaner/KosmoRay2 (personal code, no copyright issue)

class WindowPersist
{
public:
	
	bool fullscreen = false;
	bool maximized = false;
	// left, top, width, height
	int data[4] = { 100, 100, 1024, 768 };

    //
	void ApplyState(HWND hnd) const;
	// this changes the object internals
	// @param window 0 is silently ignored
	void SaveState(HWND hnd);
    //
    std::string getAsString() const;
    //
    void setFromString(const char* value);

	bool IsValid() const;
};