\# OISP Drone Research: CAD Data Management \& Git Workflow



This repository contains the Autodesk Inventor 3D CAD data for the OISP Quadcopter Drone. Because this is a multi-disciplinary engineering project, strict Product Data Management (PDM) rules must be followed to ensure information visibility to all members.



\## 1. Prefix Definitions (`\\\[SYS]` and `\\\[TYP]`)

Every file begins with a subsystem and file type code so team members can immediately identify the component's engineering discipline and CAD structure.



\*\*`\\\[SYS]` - Subsystem:\*\*

\* `SYS` = Top-level System (The complete assembled drone)

\* `MEC` = Mechanical (Frames, structural mounts, hardware)

\* `PWR` = Power Delivery (Batteries, power distribution boards, cables)

\* `PRP` = Propulsion (Motors, ESCs, propellers)

\* `AVI` = Avionics \& Electronics (Flight controllers, sensors, receivers)



\*\*`\\\[TYP]` - File Type:\*\*

\* `ASM` = Assembly (`.iam` files)

\* `PRT` = Part (`.ipt` files)



\## 2. File Naming Convention \& The "Zero Rule"

All files \*\*must\*\* follow this exact nomenclature:

\*\*Format:\*\* `\\\[SYS]-\\\[TYP]-\\\[ID]\\\_\\\[DESCRIPTION].\\\[EXT]`



\*\*THE ZERO RULE:\*\*

To prevent parts from being confused with master assemblies in the BOM:

\* \*\*Assemblies (`ASM` / `.iam`)\*\* must \*\*ALWAYS\*\* end with a zero (`0`).

\* \*\*Parts (`PRT` / `.ipt`)\*\* must \*\*NEVER\*\* end with a zero (`0`).



The hierarchy is defined mathematically by the 4-digit `\\\[ID]` block:

\* \*\*Level 0 (Top-Level Assembly):\*\* `0000`

&#x20; \* \*Example:\* `SYS-ASM-0000\\\_Main.iam`

\* \*\*Level 1 (1st Sub-Assembly Layer):\*\* Increments of `1000`

&#x20; \* \*Example:\* `MEC-ASM-1000\\\_F450\\\_Frame.iam` (Correct - ends in 0)

&#x20; \* \*Never:\* `MEC-ASM-1001\\\_F450\\\_Frame.iam` (Incorrect - assemblies cannot end in non-zero)

\* \*\*Level 2 (2nd Sub-Assembly Layer):\*\* Increments of `100` (Reserved for future modularity)

&#x20; \* \*Example:\* `MEC-ASM-1100\\\_Arm\\\_Module.iam`

\* \*\*Level 3 (Parts / Lowest Layer):\*\* Increments of `1`

&#x20; \* \*Example:\* `MEC-PRT-1001\\\_Base\\\_Top.ipt` (Correct - ends in non-zero)

&#x20; \* \*Never:\* `MEC-PRT-1000\\\_Base\\\_Top.ipt` (Incorrect - parts cannot end in zero)



\*Note: Do not use spaces in the `\\\[DESCRIPTION]`. Use underscores `\\\_` to prevent Git path-encoding errors.\*



\## 3. Mandatory Git Commit Workflow (The STEP Rule)

Because Inventor files are heavily version-dependent and parametrically linked, \*\*you must export a neutral geometry file at the end of every working session.\*\*



Before you run `git add` and `git commit`, you must:

1\. Open the top-level assembly: `SYS-ASM-0000\\\_Main.iam`.

2\. Export the entire model as a STEP file: `SYS-ASM-0000\\\_Main.stp`.

3\. Save all files and close Inventor.

4\. Commit both the native Inventor files and the new `.stp` file to Git.



\---



\# OISP Drone Research: Quản lý Dữ liệu CAD \& Quy trình Git



Kho lưu trữ này chứa dữ liệu 3D CAD (Autodesk Inventor) cho dự án OISP Quadcopter Drone. Vì đây là một dự án kỹ thuật đa ngành, nên phải tuân thủ nghiêm ngặt các quy tắc Quản lý Dữ liệu Sản phẩm (PDM) để đảm bảo tính minh bạch thông tin cho tất cả các thành viên.



\## 1. Định nghĩa Tiền tố (`\\\[SYS]` và `\\\[TYP]`)

Mỗi tệp bắt đầu bằng mã phân hệ và loại tệp để các thành viên trong nhóm có thể nhận diện ngay lập tức chuyên ngành kỹ thuật và cấu trúc CAD của chi tiết đó.



\*\*`\\\[SYS]` - Phân hệ (Subsystem):\*\*

\* `SYS` = Hệ thống tổng thể (Toàn bộ drone đã lắp ráp)

\* `MEC` = Cơ khí (Khung, ngàm kết cấu, ốc vít)

\* `PWR` = Năng lượng (Pin, mạch chia nguồn PDB, dây cáp)

\* `PRP` = Động lực (Động cơ, ESC, cánh quạt)

\* `AVI` = Điện tử hàng không (Mạch điều khiển bay, cảm biến, bộ thu phát sóng)



\*\*`\\\[TYP]` - Loại tệp (File Type):\*\*

\* `ASM` = Cụm lắp ráp (Tệp `.iam`)

\* `PRT` = Chi tiết đơn (Tệp `.ipt`)



\## 2. Quy tắc Đặt tên \& Quy tắc "Số 0"

Tất cả các tệp \*\*bắt buộc\*\* phải tuân theo cú pháp sau:

\*\*Định dạng:\*\* `\\\[SYS]-\\\[TYP]-\\\[ID]\\\_\\\[DESCRIPTION].\\\[EXT]`



\*\*QUY TẮC SỐ 0:\*\*

Để tránh việc nhầm lẫn giữa chi tiết đơn và cụm lắp ráp trong BOM:

\* \*\*Cụm lắp ráp (`ASM` / `.iam`)\*\* \*\*LUÔN LUÔN\*\* phải kết thúc bằng số không (`0`).

\* \*\*Chi tiết đơn (`PRT` / `.ipt`)\*\* \*\*KHÔNG BAO GIỜ\*\* được kết thúc bằng số không (`0`).



Cấp bậc của cấu trúc được xác định bằng toán học thông qua khối 4 chữ số `\\\[ID]`:

\* \*\*Level 0 (Cụm lắp ráp tổng/Top-Level):\*\* `0000`

&#x20; \* \*Ví dụ:\* `SYS-ASM-0000\\\_Main.iam`

\* \*\*Level 1 (Cụm lắp ráp phụ cấp 1):\*\* Bước nhảy `1000`

&#x20; \* \*Ví dụ:\* `MEC-ASM-1000\\\_F450\\\_Frame.iam` (Đúng - kết thúc bằng 0)

&#x20; \* \*Không bao giờ:\* `MEC-ASM-1001\\\_F450\\\_Frame.iam` (Sai - cụm lắp ráp không được kết thúc bằng số khác 0)

\* \*\*Level 2 (Cụm lắp ráp phụ cấp 2):\*\* Bước nhảy `100` (Dự phòng cho thiết kế module trong tương lai)

&#x20; \* \*Ví dụ:\* `MEC-ASM-1100\\\_Arm\\\_Module.iam`

\* \*\*Level 3 (Chi tiết đơn/Part):\*\* Bước nhảy `1`

&#x20; \* \*Ví dụ:\* `MEC-PRT-1001\\\_Base\\\_Top.ipt` (Đúng - kết thúc bằng số khác 0)

&#x20; \* \*Không bao giờ:\* `MEC-PRT-1000\\\_Base\\\_Top.ipt` (Sai - chi tiết đơn không được kết thúc bằng số 0)



\*Lưu ý: Không sử dụng dấu cách trong phần `\\\[DESCRIPTION]`. Hãy dùng dấu gạch dưới `\\\_` để tránh các lỗi mã hóa đường dẫn của Git.\*



\## 3. Quy tắc Bắt buộc trước khi Commit (Quy tắc STEP)

Vì các tệp Inventor phụ thuộc rất nhiều vào phiên bản phần mềm và có tính liên kết tham số (parametric), \*\*bạn phải xuất một tệp hình học trung lập vào cuối mỗi phiên làm việc.\*\*



Trước khi chạy lệnh `git add` và `git commit`, bạn bắt buộc phải:

1\. Mở cụm lắp ráp tổng: `SYS-ASM-0000\\\_Main.iam`.

2\. Xuất (Export) toàn bộ mô hình dưới dạng tệp STEP: `SYS-ASM-0000\\\_Main.stp`.

3\. Lưu tất cả các tệp và đóng Inventor.

4\. Commit cả tệp Inventor gốc và tệp `.stp` mới lên Git.

