from enum import Enum, auto

# Constants
DEFAULT_SETUP_NAME = "FCDatMon Setup 01"
DEFAULT_PROTOCOL = "USB (Serial)"
DEFAULT_TARGET = "9DOF_IMU"
DEFAULT_LAYOUT = "1x2"

# Wiped default UUIDs to force manual entry
DEFAULT_BLE_SERVICE_UUID = ""
DEFAULT_BLE_CHAR_UUID = ""

COLOR_TITLE = (0, 255, 0)
COLOR_SECTION_HEADING = (255, 255, 100) # Bright yellow for all sections
COLOR_SESSION_HEADING = (255, 255, 100) 
COLOR_H1 = (220, 200, 100)
COLOR_AXIS_LBL = (130, 200, 130)
COLOR_LOG = (255, 255, 100)

THEME_RED_BTN = "theme_red_btn"
THEME_GREEN_BTN = "theme_green_btn"
THEME_YELLOW_BTN = "theme_yellow_btn"

# Global application states
CURRENT_READER = None
ABORT_PAIRING = False

LOG_FREQUENCY = False
FREQUENCY_WINDOW_SIZE = 500

WINDOW_WIDTH = 420
WINDOW_HEIGHT = 700

UI_EVENT_QUEUE = []

PLOT_CACHE = {}
PLOT_CHASE_ACTIVE = {}

PLOT_PALETTE = [
    # Top 20 (Most Used)
    "#E6194B", "#3CB44B", "#FFE119", "#4363D8", "#F58231",
    "#911EB4", "#46F0F0", "#F032E6", "#BCF60C", "#FABEBE",
    "#008080", "#E6BEFF", "#9A6324", "#FFFAC8", "#800000",
    "#AAFFC3", "#808000", "#FFD8B1", "#000075", "#808080",
    # 30 Additional Vibrant Colors
    "#FF5733", "#C70039", "#900C3F", "#581845", "#DAF7A6",
    "#2ECC71", "#27AE60", "#3498DB", "#2980B9", "#9B59B6",
    "#8E44AD", "#F1C40F", "#F39C12", "#E67E22", "#D35400",
    "#E74C3C", "#C0392B", "#1ABC9C", "#16A085", "#34495E",
    "#2C3E50", "#7F8C8D", "#BDC3C7", "#95A5A6", "#117864",
    "#7D3C98", "#D68910", "#BA4A00", "#7E5109", "#28B463"
]

class UIState(Enum):
    INIT = auto()
    TARGET_SELECTED = auto()
    PAIRING = auto()
    PAIRED = auto()
    STREAMING = auto()
    PAUSED = auto()

CURRENT_UI_STATE = UIState.INIT
