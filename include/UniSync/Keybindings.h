#pragma once

/*
#define KEYMAP_RENAME VK_RETURN
#define KEYMAP_DELETE VK_DELETE
#define KEYMAP_UP VK_UP
#define KEYMAP_DOWN VK_DOWN
#define KEYMAP_OPEN VK_RIGHT
#define KEYMAP_CLOSE VK_LEFT

#define KEYMAP_LINKED 'L'
#define KEYMAP_FAILED 'F'

#define KEYMAP_PENDING 'P'
#define KEYMAP_STATIC 'S'
#define KEYMAP_IGNORE 'I'

#define KEYMAP_SYNCUNITS 'U'
#define KEYMAP_CONNECTIONS 'C'
#define KEYMAP_SYNCHRONIZE 'Y'
#define KEYMAP_SAVE 'T'
*/
namespace unisync {
	enum Key : uint16_t {
		KeyUp, KeyDown, KeyOpen, KeyClose, KeyEdit, KeyMainAction, KeyDelete, KeyFreeze, KeyMelt
	};

	void CreateDefaultKeybindings();
}