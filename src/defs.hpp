
#ifndef _DEFS_HPP_
#define _DEFS_HPP_

// To avoid freezing the UI
#define MAX_ADD_WATCHES  1000


#define IMPLE_NAME     "mytools.calc.CppWatchingWindow"
#define SERVICE_NAME   IMPLE_NAME
#define RESOURCE_NAME  "private:resource/toolpanel/mytools.calc/WatchingWindow"
#define CONFIG_NODE    "/mytools.calc.WatchingWindow/Settings"

#define CONFIG_OPTIONS "/org.openoffice.Office.OptionsDialog/Nodes/Calc/Leaves/org.openoffice.Office.OptionsDialog:Leaf['mytools.calc.CppWatchingWindow_01']"

#define PROP_WARN           "WarnNumberOfCells"
#define PROP_USE_INPUT_LINE "InputLine"
#define PROP_STORE_WATCHES  "StoreWatches"

#define EXT_ID     IMPLE_NAME
#define EXT_DIR    "vnd.sun.star.extension://" EXT_ID
#define IMG_DIR    EXT_DIR "/icons/"
#define IMGH_DIR    EXT_DIR "/iconsh/"
#define ABOUT_DIALOG EXT_DIR "/dialogs/About.xdl"
#define LICENSE_FILE EXT_DIR "/LICENSE"
#define RES_DIR    EXT_DIR "/resources"

#define RES_NAME   "strings"

#define IMG_ADD    "add_16.png"
#define IMG_DEL    "delete_16.png"
#define IMG_GOTO   "goto_16.png"
#define IMG_TUNE   "tune_16.png"
#define IMG_UPDATE "update_16.png"

#define CMD_ADD 65
#define CMD_REMOVE 66
#define CMD_UPDATE 67
#define CMD_GOTO 68
#define CMD_OPTION 71
#define CMD_CLEAR 72
#define CMD_INPUT_LINE 73
#define CMD_STORE_WATCHES 74
#define CMD_SETTINGS 75
#define CMD_ABOUT 76
#define CMD_UP   83
#define CMD_DOWN 84
#define CMD_HOME 85
#define CMD_END  86
#define CMD_SPACE 87
#define CMD_SELECT_CURRENT 90

#define CTL_GRID      "grid"
#define CTL_EDIT      "edit_input"
#define CTL_BTN_ADD   "btn_add"
#define CTL_BTN_DELETE "btn_delete"
#define CTL_BTN_GOTO   "btn_goto"
#define CTL_BTN_UPDATE "btn_update"
#define CTL_BTN_OPTION "btn_option"

#endif
