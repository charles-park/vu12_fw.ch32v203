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

HDMI Audio 테스트
* Linux Alsa-utils를 활용하여 스피커 테스트 : https://yongeekd01.tistory.com/110

HDMI EDID 생성 (https://wiki.odroid.com/common/hdmi_howto/custom_edid)
1. https://tomverbeure.github.io/video_timings_calculator 에서 해상도에 맞는 parameter를 구함.
2. 만약 지원하지 않는 pixel clock(ODROID-XU4)인 경우 가까운 clock중 높을 것을 사용함. (Refresh rate는 높아짐)
3. 해당 parameter를 PC_APP.make_edid폴더안의 mk_edid.c에 아래와 같이 적용함.
```

// xu4 support edid.bin
//
// pixel clock support list
// https://github.com/hardkernel/linux/blob/odroid-5.4.y/drivers/gpu/drm/exynos/exynos_hdmi.c#L396-L740
//
cconst video_timing  lcd_timing = {
    /*
    111750,  // pixel clock(Khz) -> refresh 60Hz pixel clock
    */
    115500,  // pixel clock(Khz) -> xu4 support pixel clock (refresh 61.9Hz)

    // lcd horizontal data
    96,     // hfp
    192,    // hs
    288,    // hbp
    1920,   // hact = (real view area, 960 * 2(LVDS ch) = 1920)
    2496,   // htotal = hact + hbp + hs + hfp

    // lcd vertical data
    3,      // vfp
    10,     // vs
    15,     // vbp
    720,    // vact = (real view area, 720)
    748     // vtotal = vact + vbp + vs + vfp
};

```
4. build (gcc -o mk_edid mk_edid.c) 및 실행
5. edid.bin파일 생성 및 화면에 edid data표시 (복사하여 firmware edid 배열 데이터로 사용함)
