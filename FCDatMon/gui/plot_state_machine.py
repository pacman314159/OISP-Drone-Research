"""
Plot State Machine Logic based on User Requirements.

State Variables:
bit 3: Signal chasing
bit 2: Data stream connected or unterminated
bit 1: Hardware connected (BLE/USB/LoRa)
bit 0: Fix Y-range button state
"""

def get_current_state(is_chasing: bool, is_data_stream_connected: bool, is_hw_connected: bool, is_y_fixed: bool) -> int:
    state = 0
    if is_chasing: state |= (1 << 3)
    if is_data_stream_connected: state |= (1 << 2)
    if is_hw_connected: state |= (1 << 1)
    if is_y_fixed: state |= (1 << 0)
    return state

def get_gesture_allowance(state: int):
    """
    Returns (vert_pan, horz_pan, vert_zoom, horz_zoom)
    """
    if state in [4, 5, 12, 13]:
        # Invalid states: data stream cannot happen without hardware connected
        return (False, False, False, False)
        
    allowances = {
        0: (True, True, True, True),
        1: (True, True, True, True),
        2: (True, True, True, True),
        3: (False, True, False, True),
        6: (True, True, True, True),
        7: (True, True, True, True),
        8: (True, True, True, True),
        9: (True, True, True, True),
        10: (True, True, True, True),
        11: (False, True, False, True),
        14: (True, False, True, True),
        15: (False, False, False, True)
    }
    
    return allowances.get(state, (True, True, True, True))
