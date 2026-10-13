<!-- converted from program_state_and_evts.xlsx -->

## Sheet: Sheet1
| No | States |  |  |  | Gestures Allowance |  |  |  | Note | Events |  |  |  |  |  |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|  | signal chasing | data stream connected or unterminated | ble connected | Fix Y-range button state | vertical pan | horizontal pan | vertical zoom | horizontal zoom |  | Fix Y-range ticked or unticked | Effect | Next state | Signal chasing enabled or unabled | Effect | Next state |
| 0 | False | False | False | False | True | True | True | True |  | ticked | snap back to determined range | 1 | ticked |  |  |
| 1 | False | False | False | True | True | True | True | True |  | unticked |  | 0 | ticked |  |  |
| 2 | False | False | True | False | True | True | True | True |  | ticked | snap back to determined range | 3 | ticked |  |  |
| 3 | False | False | True | True | False | True | False | True |  | unticked |  | 2 | ticked |  |  |
| 4 | False | True | False | False | don’t exist | don’t exist | don’t exist | don’t exist | data stream cant happen without ble connected | ticked |  |  | ticked |  |  |
| 5 | False | True | False | True | don’t exist | don’t exist | don’t exist | don’t exist | data stream cant happen without ble connected | unticked |  |  | ticked |  |  |
| 6 | False | True | True | False | True | True | True | True |  | ticked | snap back to determined range | 7 | ticked | start chasing | 14 |
| 7 | False | True | True | True | False | True | False | True |  | unticked |  | 6 | ticked | start chasing | 15 |
| 8 | True | False | False | False | True | True | True | True |  | ticked | snap back to determined range | 9 | unticked |  |  |
| 9 | True | False | False | True | True | True | True | True | no data yet so fix y range has no effect | unticked |  | 8 | unticked |  |  |
| 10 | True | False | True | False | True | True | True | True |  | ticked | snap back to determined range | 11 | unticked |  |  |
| 11 | True | False | True | True | False | True | False | True |  | unticked |  | 10 | unticked |  |  |
| 12 | True | True | False | False | don’t exist | don’t exist | don’t exist | don’t exist | data stream cant happen without ble connected | ticked |  |  | unticked |  |  |
| 13 | True | True | False | True | don’t exist | don’t exist | don’t exist | don’t exist | data stream cant happen without ble connected | unticked |  |  | unticked |  |  |
| 14 | True | True | True | False | True | False | True | True |  | ticked | snap back to determined range | 15 | unticked | stop chasing | 6 |
| 15 | True | True | True | True | False | False | False | True |  | unticked |  | 14 | unticked | stop chasing | 7 |