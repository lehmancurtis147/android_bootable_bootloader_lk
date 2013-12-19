/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <msm_panel.h>
#include <mipi_dsi.h>
#include <platform/iomap.h>
#include <sys/types.h>
#include <err.h>
#include <reg.h>
#include <mdp4.h>
#include <debug.h>

/* JDI wqxga split display panel commands */
static const unsigned char jdi_wqxga_mcap[4] = {
		0xB0, 0x00, DTYPE_GEN_WRITE2, 0x80,
};

static const unsigned char jdi_wqxga_intf_setting[12] = {
		0x06, 0x00, DTYPE_GEN_LWRITE, 0xC0,
			0xB3, 0x04, 0x08, 0x00,
				0x22, 0x00, 0xFF, 0xFF
};

static const unsigned char jdi_wqxga_intf_id_setting[8] = {
		0x02, 0x00, DTYPE_GEN_LWRITE, 0xC0,
			0xB4, 0x0C, 0xFF, 0xFF,
};

static const unsigned char jdi_wqxga_dsi_ctrl[8] = {
		0x03, 0x00, DTYPE_GEN_LWRITE, 0xC0,
			0xB6, 0x3A, 0xD3, 0xFF,
};

static const unsigned char jdi_wqxga_pixel_format[4] = {
		0x3A, 0x77, DTYPE_DCS_WRITE1, 0x80,
};

static const unsigned char jdi_wqxga_col_addr[12] = {
		0x05, 0x00, DTYPE_DCS_LWRITE, 0xC0,
			0x2A, 0x00, 0x00, 0x04,
				0xFF, 0xFF, 0xFF, 0xFF,
};

static const unsigned char jdi_wqxga_page_addr[12] = {
		0x05, 0x00, DTYPE_DCS_LWRITE, 0xC0,
			0x2B, 0x00, 0x00, 0x06,
				0x3F, 0xFF, 0xFF, 0xFF,
};

static const unsigned char jdi_wqxga_tear_on[4] = {
		0x35, 0x00, DTYPE_DCS_WRITE1, 0x80,
};

static const unsigned char jdi_wqxga_tear_scanline[8] = {
		0x03, 0x00, DTYPE_DCS_LWRITE, 0xC0,
			0x44, 0x00, 0x00, 0xFF,
};

static const unsigned char jdi_wqxga_write_brightness[4] = {
		0x51, 0xFF, DTYPE_DCS_WRITE1, 0x80,
};

static const unsigned char jdi_wqxga_ctrl_display[4] = {
		0x53, 0x24, DTYPE_DCS_WRITE1, 0x80,
};

static const unsigned char jdi_wqxga_intf_setting2[12] = {
		0x06, 0x00, DTYPE_GEN_LWRITE, 0xC0,
			0xB3, 0x14, 0x08, 0x00,
				0x22, 0x00, 0xFF, 0xFF
};

/* End of JDI wqxga split display commands */

static struct mipi_dsi_cmd jdi_wqxga_video_mode_cmds[] = {
	{sizeof(dsi_display_sw_reset), (char *)dsi_display_sw_reset, 120},
	{sizeof(jdi_wqxga_mcap), (char *)jdi_wqxga_mcap, 120},
	{sizeof(jdi_wqxga_intf_setting), (char *)jdi_wqxga_intf_setting, 120},
	{sizeof(jdi_wqxga_intf_id_setting), (char *)jdi_wqxga_intf_id_setting, 120},
	{sizeof(jdi_wqxga_dsi_ctrl), (char *)jdi_wqxga_dsi_ctrl, 120},
	{sizeof(jdi_wqxga_pixel_format), (char *)jdi_wqxga_pixel_format, 120},
	{sizeof(jdi_wqxga_col_addr), (char *)jdi_wqxga_col_addr, 120},
	{sizeof(jdi_wqxga_page_addr), (char *)jdi_wqxga_page_addr, 120},
	{sizeof(jdi_wqxga_tear_on), (char *)jdi_wqxga_tear_on, 120},
	{sizeof(jdi_wqxga_tear_scanline), (char *)jdi_wqxga_tear_scanline, 120},
	{sizeof(jdi_wqxga_write_brightness), (char *)jdi_wqxga_write_brightness, 120},
	{sizeof(jdi_wqxga_ctrl_display), (char *)jdi_wqxga_ctrl_display, 120},
	{sizeof(dsi_display_exit_sleep), (char *)dsi_display_exit_sleep, 120},
	{sizeof(jdi_wqxga_intf_setting2), (char *)jdi_wqxga_intf_setting2, 120},
	{sizeof(dsi_display_display_on), (char *)dsi_display_display_on, 120},
};

int mipi_jdi_video_wqxga_config(void *pdata)
{
	int ret = NO_ERROR;

	/* 3 Lanes -- Enables Data Lane0, 1, 2 */
	uint8_t lane_en = 0xf;
	uint64_t low_pwr_stop_mode = 0;

	/* Needed or else will have blank line at top of display */
	uint8_t eof_bllp_pwr = 0x9;

	uint8_t interleav = 0;
	struct lcdc_panel_info *lcdc = NULL;
	struct msm_panel_info *pinfo = (struct msm_panel_info *)pdata;

	if (pinfo == NULL)
		return ERR_INVALID_ARGS;

	lcdc =  &(pinfo->lcdc);
	if (lcdc == NULL)
		return ERR_INVALID_ARGS;

	ret = mdss_dsi_video_mode_config((pinfo->xres/2 + lcdc->xres_pad),
			(pinfo->yres + lcdc->yres_pad),
			(pinfo->xres/2),
			(pinfo->yres),
			(lcdc->h_front_porch),
			(lcdc->h_back_porch + lcdc->h_pulse_width),
			(lcdc->v_front_porch),
			(lcdc->v_back_porch + lcdc->v_pulse_width),
			(lcdc->h_pulse_width),
			(lcdc->v_pulse_width),
			pinfo->mipi.dst_format,
			pinfo->mipi.traffic_mode,
			lane_en,
			low_pwr_stop_mode,
			eof_bllp_pwr,
			interleav,
			MIPI_DSI0_BASE);

	ret = mdss_dsi_video_mode_config((pinfo->xres/2 + lcdc->xres_pad),
			(pinfo->yres + lcdc->yres_pad),
			(pinfo->xres/2),
			(pinfo->yres),
			(lcdc->h_front_porch),
			(lcdc->h_back_porch + lcdc->h_pulse_width),
			(lcdc->v_front_porch),
			(lcdc->v_back_porch + lcdc->v_pulse_width),
			(lcdc->h_pulse_width),
			(lcdc->v_pulse_width),
			pinfo->mipi.dst_format,
			pinfo->mipi.traffic_mode,
			lane_en,
			low_pwr_stop_mode,
			eof_bllp_pwr,
			interleav,
			MIPI_DSI1_BASE);

	return ret;
}

int mipi_jdi_video_wqxga_on()
{
	int ret = NO_ERROR;
	return ret;
}

int mipi_jdi_video_wqxga_off()
{
	int ret = NO_ERROR;
	return ret;
}

static struct mdss_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* 720x1280, RGB888, 4 Lane 60 fps video mode */
	/* regulator */
	{0x07, 0x09, 0x03, 0x00, 0x20, 0x00, 0x01},
	/* timing */
	{0xef, 0x38, 0x25, 0x00, 0x67, 0x70, 0x29, 0x3c,
		0x2c, 0x03, 0x04, 0x00},
#if 0
	{0x22, 0x42, 0x35, 0x00, 0x5c, 0x53, 0x37, 0x44,
		0x2a, 0x03, 0x04, 0x00},
#endif
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x06},
	/* bist control */
	{0x00, 0x00, 0xb1, 0xff, 0x00, 0x00},
	/* lanes config */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x97,
	 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x01, 0x97,
	 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x97,
	 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x01, 0x97,
	 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xbb},
};

void mipi_jdi_video_wqxga_init(struct msm_panel_info *pinfo)
{
	if (!pinfo)
		return;

	pinfo->xres = 2560;
	pinfo->yres = 1600;
	/*
	 *
	 * Panel's Horizontal input timing requirement is to
	 * include dummy(pad) data of 200 clk in addition to
	 * width and porch/sync width values
	 */

	pinfo->type = MIPI_VIDEO_PANEL;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 24;
	pinfo->lcdc.h_back_porch = 80;
	pinfo->lcdc.h_front_porch = 164;
	pinfo->lcdc.h_pulse_width = 12;
	pinfo->lcdc.v_back_porch = 4;
	pinfo->lcdc.v_front_porch = 12;
	pinfo->lcdc.v_pulse_width = 4;
	pinfo->lcdc.border_clr = 0;	/* blk */
	pinfo->lcdc.underflow_clr = 0xff;	/* blue */
	pinfo->lcdc.hsync_skew = 0;
	pinfo->clk_rate = 424000000;
	pinfo->lcdc.dual_pipe = TRUE;
	pinfo->lcdc.pipe_swap = TRUE;
	pinfo->lcdc.split_display = TRUE;

	pinfo->mipi.lane_swap = 0x6;
	pinfo->mipi.dual_dsi = TRUE;
	pinfo->mipi.broadcast = TRUE;
	pinfo->mipi.mode = DSI_VIDEO_MODE;
	pinfo->mipi.pulse_mode_hsa_he = FALSE;
	pinfo->mipi.hfp_power_stop = FALSE;
	pinfo->mipi.hbp_power_stop = FALSE;
	pinfo->mipi.hsa_power_stop = FALSE;
	pinfo->mipi.eof_bllp_power_stop = TRUE;
	pinfo->mipi.bllp_power_stop = TRUE;
	pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo->mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;

	pinfo->mipi.vc = 0;
	pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo->mipi.data_lane0 = TRUE;
	pinfo->mipi.data_lane1 = TRUE;
	pinfo->mipi.data_lane2 = TRUE;
	pinfo->mipi.data_lane3 = TRUE;
	pinfo->mipi.t_clk_post = 0x1e;
	pinfo->mipi.t_clk_pre = 0x38;
	pinfo->mipi.stream = 0; /* dma_p */
	pinfo->mipi.mdp_trigger = 0;
	pinfo->mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo->mipi.frame_rate = 60;
	pinfo->mipi.mdss_dsi_phy_db = &dsi_video_mode_phy_db;
	pinfo->mipi.tx_eot_append = TRUE;

	pinfo->mipi.num_of_lanes = 4;
	pinfo->mipi.panel_cmds = jdi_wqxga_video_mode_cmds;
	pinfo->mipi.num_of_panel_cmds =
				 ARRAY_SIZE(jdi_wqxga_video_mode_cmds);

	pinfo->on = mipi_jdi_video_wqxga_on;
	pinfo->off = mipi_jdi_video_wqxga_off;
	pinfo->config = mipi_jdi_video_wqxga_config;

	return;
}
