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

#include "GLFW/glfw3.h"

namespace unisync {
	enum Key : u16 {
		KeyUp           =  GLFW_KEY_UP,
        KeyDown         =  GLFW_KEY_DOWN,
        KeyOpen         =  GLFW_KEY_RIGHT,
        KeyClose        =  GLFW_KEY_LEFT,
        KeyEdit         =  GLFW_KEY_ENTER,
        KeyMainAction   =  GLFW_KEY_SPACE,
        KeyDelete       =  GLFW_KEY_BACKSPACE,
        KeyFreeze       =  GLFW_KEY_F,
        KeyMelt         =  GLFW_KEY_M,
    };

	// void CreateDefaultKeybindings();
}