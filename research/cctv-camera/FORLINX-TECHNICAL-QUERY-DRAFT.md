# Draft only — not sent

To: Forlinx technical sales (official product inquiry channel)
Subject: FET3588J-C shared Linux platform for outdoor fixed/PTZ 4K60 AI cameras

Hello Forlinx team,

We are evaluating FET3588J-C for fixed and PTZ cameras using a common Linux
platform. Target ambient is -20 to +50 C, with PoE, USB-C power/service,
Wi-Fi/Bluetooth, eventual LTE/5G and concurrent AI inference. Android is excluded.

Please provide:

1. Exact industrial module and matching evaluation-kit ordering codes,
   revision-specific STEP, mounted-height/keepout drawings, connector part
   numbers and mating tolerances. Initial evaluation preference is 8 GB RAM /
   64 GB eMMC, subject to workload validation. Confirm component temperature
   grades and required cooling at the industrial CPU/NPU clocks.
2. Clarify permitted input supply/tolerance, sequencing and startup current:
   product page says 12 V, brief 5–13 V, some manual rows 5–12 V. Provide
   measured idle and simultaneous video/AI power for the exact revision.
3. Identify a supported Linux BSP and demonstration of continuous 3840 x 2160
   60fps capture and H.265 encoding from (A) Sony IMX585 MIPI module with ISP
   and (B) Sony FCB-ER9500 HDMI-compatible output. Specify driver, ISP tuning,
   pixel format, bit depth, memory path and hardware-encoder limitations.
   Do you support FRAMOS FSM-IMX585 specifically, or another exact module?
4. Confirm simultaneous use of HDMI RX or MIPI, cellular over USB3, Wi-Fi over
   PCIe, Bluetooth over USB/UART, Ethernet and a separate USB OTG service port.
   Provide a reviewed pinmux proposal and controller/PHY-sharing restrictions.
5. Provide RKNN tooling/runtime support, supported model operators and a
   concurrent inference/4K60 benchmark if available. Peak TOPS alone is not
   enough to select the platform. Can clocks and thermal limits be monitored?
6. Provide Linux SDK/firmware redistribution terms, security update policy,
   recovery/secure-boot support and module lifecycle. Confirm carrier design
   review availability and documentation access requirements.
7. Quote one evaluation kit and two industrial modules, indicative 100/1,000
   module pricing, MOQ and lead times. These are quote quantities only.

We will design housing-specific carriers and need evidence before freezing
mechanical or PCB design. Do not start custom development or production.

Reference: https://www.forlinx.net/product/rk3588-som-134.html
No contact or order has been made.
