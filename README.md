# vu12_fw.ch32v203
저가형 RISC-V(CH32V203)을 사용한 Vu12(HDMI2LVDS:1920x720) Project

ch32v203 참조사이트
* WCH Product site : https://www.wch-ic.com/products/CH32V203.html
* WCH ISP Tool(wchisp) : https://github.com/ch32-rs/wchisp
* RISC-V Toolchain : http://www.mounriver.com/download (Linux 2024/04/03: MRS_Toolchain_Linux_x64_V1.91.tar.xz)
* Demo Board : https://github.com/WeActStudio/WeActStudio.BluePill-Plus-CH32
* Project Doc(Protocol & Port define) : https://docs.google.com/spreadsheets/d/1HiW1aLE0fOjp_hWNbl3xYpfYre3RCjelAIYnHm_vM1g/edit?pli=1#gid=1131227336
* 기존 Project(CH552) : https://github.com/charles-park/vu12_fw.ch552?tab=readme-ov-file

ch32v203 write error
* isp download시 verify error 발생 이유는 flash protect가 되어 있어서 발생됨.
* wchisp tool을 사용하여 flash unprotect후 사용하여야 함. (wchisp config unprotect)

ch32v203 모델별 메모리 설정
* Ld 폴더안의 Link.ld내용을 수정

ch32V203 Debug serial port 설정
* Debug 폴더안의 debug.h 파일 내용중 DEBUG_UARTx 설정

